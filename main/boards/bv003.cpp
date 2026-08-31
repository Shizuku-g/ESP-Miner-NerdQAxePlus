#include "bv003.h"

#include <stdio.h>

#include "bm1373.h"
#include "drivers/rev7/TPS546.h"
#include "drivers/TMP1075.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "global_state.h"
#include "serial.h"

static const char *TAG = "bv003";

static constexpr int BV003_TMP1075_COUNT = 2;

static constexpr float CHIP_TEMP_OFFSET = 10.0f;

static float readChipTemp(int device_index)
{
    const float temp = TMP1075_read_temperature(device_index);
    if (temp == 0.0f) {
        return 0.0f;
    }
    return temp + CHIP_TEMP_OFFSET;
}

Bv003::Bv003() : NerdQaxePlus2()
{
    m_deviceModel = "BV003";
    m_miningAgent = m_deviceModel;
    m_asicModel = "BM1373";
    m_asicCount = 4;
    m_rev7VoltageDomains = 4;
    m_numPhases = 4;
    m_imax = 180;
    m_ifault = 200.0f;

    m_asics = new BM1373();
    m_vrFrequency = m_defaultVrFrequency = m_asics->getDefaultVrFrequency();

    m_fanLabels[0] = "M2 (Use a Y-splitter cable for multiple ASIC fans.)";
    m_fanLabels[1] = "M1";
    m_swarmColorName = "#11d51e";

    applyBv003AsicProfile();

#ifdef BV003
    m_theme = new ThemeBv003();
#endif
}

void Bv003::applyBv003AsicProfile()
{
    m_asicJobIntervalMs = 500;
    m_version = 505;
    m_asicFrequencies = {550, 600, 650, 700, 750, 800};
    m_asicVoltages = {1000, 1050, 1100, 1150, 1200};
    m_defaultAsicFrequency = m_asicFrequency = 600;
    m_defaultAsicVoltageMillis = m_asicVoltageMillis = 1150;
    m_absMaxAsicFrequency = 1000;
    m_absMinAsicVoltageMillis = 900;
    m_absMaxAsicVoltageMillis = 1400;
    m_initVoltageMillis = 1050;

    m_maxPin = 300.0;
    m_minPin = 80.0;
    m_maxVin = 13.0;
    m_minVin = 11.0;
    m_minCurrentA = 0.0f;
    m_maxCurrentA = 32.0f;

    m_asicMaxDifficulty = 4096;
    m_asicMinDifficulty = 1024;
    m_asicMinDifficultyDualPool = 512;
}

Rev7TPS546::TPS546_CONFIG Bv003::createRev7Tps546Config()
{
    return Rev7TPS546::TPS546_create_quad_config();
}

#ifdef BV003_CHIP_PROBE

void Bv003::shutdownAsicPower()
{
    setAsicReset(false);
    LDO_disable();
    VREG_disable();
    setVoltage(0.0f);
}

void Bv003::probeChipsZeroCore()
{
    if (!m_hasRev7TPS546) {
        ESP_LOGW(TAG, "chip probe: TPS546 not detected");
        return;
    }

    shutdownAsicPower();
    vTaskDelay(pdMS_TO_TICKS(250));

    LDO_enable();
    vTaskDelay(pdMS_TO_TICKS(100));

    setAsicReset(true);
    vTaskDelay(pdMS_TO_TICKS(500));

    SERIAL_clear_buffer();
    const int detected = m_asics ? m_asics->probeChipCount() : 0;
    m_chipsDetected = detected;

    ESP_LOGI(TAG, "chip probe: detected %d / %d (core 0V)", detected, m_asicCount);

    shutdownAsicPower();
}

bool Bv003::initAsics()
{
    ESP_LOGI(TAG, "chip probe firmware, skipping mining init");
    shutdownAsicPower();
    return true;
}

void Bv003::finalizeProbeReport()
{
    m_probeReport.tmp1075Ok = m_numTempSensors >= BV003_TMP1075_COUNT;
    m_probeReport.tmp1075C0 = m_probeTmpRawC[0];
    m_probeReport.tmp1075C1 = m_probeTmpRawC[1];
    m_probeReport.w5500Ok = NETWORK.probeEthHardware();
    m_probeReport.tps546Ok = m_hasRev7TPS546;
    m_probeReport.chipsDetected = m_chipsDetected;
    m_probeReport.chipsExpected = m_asicCount;
    updateProbeEthStatus();
}

void Bv003::updateProbeEthStatus()
{
    m_probeReport.w5500Ok = NETWORK.isEthHardwarePresent();
    m_probeReport.ethHasIp = NETWORK.hasEthIp();
}

void Bv003::formatProbeScreenText(char *buf, size_t len) const
{
    if (!buf || len == 0) {
        return;
    }

    const bool asicOk = m_probeReport.chipsDetected == m_probeReport.chipsExpected
                        && m_probeReport.chipsExpected > 0;

    snprintf(buf, len,
             "BV003 CHIP PROBE\n\n"
             "TMP1075:  %s (%.1f/%.1f C)\n"
             "W5500:    %s\n"
             "ETH IP:   %s\n"
             "TPS546:   %s\n"
             "ASIC:     %s (%d/%d)",
             m_probeReport.tmp1075Ok ? "OK" : "FAIL",
             m_probeReport.tmp1075C0,
             m_probeReport.tmp1075C1,
             m_probeReport.w5500Ok ? "OK" : "FAIL",
             m_probeReport.ethHasIp ? "OK" : "WAIT",
             m_probeReport.tps546Ok ? "OK" : "FAIL",
             asicOk ? "OK" : "FAIL",
             m_probeReport.chipsDetected,
             m_probeReport.chipsExpected);
}

#endif

void Bv003::detectChipTempSensors()
{
    int found = 0;
    for (int i = 0; i < BV003_TMP1075_COUNT; i++) {
        float temp = TMP1075_read_temperature(i);
        if (temp == 0.0f) {
            break;
        }
        ESP_LOGI(TAG, "found chip TMP1075 %d (%.2f C, addr 0x%02x)",
                 i, temp, TMP1075_I2CADDR_DEFAULT + i);
#ifdef BV003_CHIP_PROBE
        m_probeTmpRawC[i] = temp;
#endif
        found++;
    }
    m_numTempSensors = found;
    ESP_LOGI(TAG, "found %d chip TMP1075 sensors", m_numTempSensors);
}

bool Bv003::initBoard()
{
    if (!NerdQaxePlus2::initBoard()) {
        return false;
    }

    applyBv003AsicProfile();
    loadSettings();

    detectChipTempSensors();

#ifdef BV003_CHIP_PROBE
    probeChipsZeroCore();
    finalizeProbeReport();
#endif

    ESP_LOGI(TAG, "BV003 init done (version=%d, ethernet=W5500, vr_domains=%d)",
             m_version, m_rev7VoltageDomains);

    return true;
}

float Bv003::getTemperature(int index)
{
    if (index < 0 || index >= m_numTempSensors) {
        return 0.0f;
    }
    return readChipTemp(index);
}

void Bv003::requestChipTemps()
{
    if (m_shutdown) {
        for (int i = 0; i < m_asicCount; i++) {
            setChipTemp(i, 0.0f);
        }
        return;
    }

    const float temp0 = readChipTemp(0);
    const float temp1 = readChipTemp(1);

    if (temp0) {
        setChipTemp(0, temp0);
        setChipTemp(1, temp0);
    }
    if (temp1) {
        setChipTemp(2, temp1);
        setChipTemp(3, temp1);
    }
}
