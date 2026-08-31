#include "bv002.h"

#include <stdio.h>

#include "bm1373.h"
#include "drivers/rev7/TPS546.h"
#include "drivers/TMP1075.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "global_state.h"
#include "serial.h"

static const char *TAG = "bv002";

static constexpr int BV002_TMP1075_COUNT = 1;

static constexpr float CHIP_TEMP_OFFSET = 10.0f;

static float readChipTemp(int device_index)
{
    const float temp = TMP1075_read_temperature(device_index);
    if (temp == 0.0f) {
        return 0.0f;
    }
    return temp + CHIP_TEMP_OFFSET;
}

Bv002::Bv002() : NerdQaxePlus2()
{
    m_deviceModel = "BV002";
    m_miningAgent = m_deviceModel;
    m_asicModel = "BM1373";
    m_asicCount = 2;
    m_rev7VoltageDomains = 2;
    m_numPhases = 3;
    m_imax = 135;
    m_ifault = 150.0f;

    m_asics = new BM1373();
    m_vrFrequency = m_defaultVrFrequency = m_asics->getDefaultVrFrequency();

    applyBv002AsicProfile();

#ifdef BV002
    m_theme = new ThemeBv002();
#endif
}

void Bv002::applyBv002AsicProfile()
{
    m_asicJobIntervalMs = 500;
    m_version = 504;
    m_asicFrequencies = {550, 600, 650, 700, 750, 800};
    m_asicVoltages = {1000, 1050, 1100, 1150, 1200};
    m_defaultAsicFrequency = m_asicFrequency = 600;
    m_defaultAsicVoltageMillis = m_asicVoltageMillis = 1150;
    m_absMaxAsicFrequency = 1000;
    m_absMinAsicVoltageMillis = 900;
    m_absMaxAsicVoltageMillis = 1400;
    m_initVoltageMillis = 1050;

    m_maxPin = 180.0;
    m_minPin = 52.0;
    m_maxVin = 13.0;
    m_minVin = 11.0;
    m_minCurrentA = 0.0f;
    m_maxCurrentA = 16.0f;

    m_asicMaxDifficulty = 4096;
    m_asicMinDifficulty = 1024;
    m_asicMinDifficultyDualPool = 512;
}

Rev7TPS546::TPS546_CONFIG Bv002::createRev7Tps546Config()
{
    return Rev7TPS546::TPS546_create_triple_config();
}

#ifdef BV002_CHIP_PROBE

void Bv002::shutdownAsicPower()
{
    setAsicReset(false);
    LDO_disable();
    VREG_disable();
    setVoltage(0.0f);
}

void Bv002::probeChipsZeroCore()
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

bool Bv002::initAsics()
{
    ESP_LOGI(TAG, "chip probe firmware, skipping mining init");
    shutdownAsicPower();
    return true;
}

void Bv002::finalizeProbeReport()
{
    m_probeReport.tmp1075Ok = m_numTempSensors > 0;
    m_probeReport.tmp1075C = m_probeTmpRawC;
    m_probeReport.w5500Ok = NETWORK.probeEthHardware();
    m_probeReport.tps546Ok = m_hasRev7TPS546;
    m_probeReport.chipsDetected = m_chipsDetected;
    m_probeReport.chipsExpected = m_asicCount;
    updateProbeEthStatus();
}

void Bv002::updateProbeEthStatus()
{
    m_probeReport.w5500Ok = NETWORK.isEthHardwarePresent();
    m_probeReport.ethHasIp = NETWORK.hasEthIp();
}

void Bv002::formatProbeScreenText(char *buf, size_t len) const
{
    if (!buf || len == 0) {
        return;
    }

    const bool asicOk = m_probeReport.chipsDetected == m_probeReport.chipsExpected
                        && m_probeReport.chipsExpected > 0;

    snprintf(buf, len,
             "BV002 CHIP PROBE\n\n"
             "TMP1075:  %s (%.1f C)\n"
             "W5500:    %s\n"
             "ETH IP:   %s\n"
             "TPS546:   %s\n"
             "ASIC:     %s (%d/%d)",
             m_probeReport.tmp1075Ok ? "OK" : "FAIL",
             m_probeReport.tmp1075C,
             m_probeReport.w5500Ok ? "OK" : "FAIL",
             m_probeReport.ethHasIp ? "OK" : "WAIT",
             m_probeReport.tps546Ok ? "OK" : "FAIL",
             asicOk ? "OK" : "FAIL",
             m_probeReport.chipsDetected,
             m_probeReport.chipsExpected);
}

#endif

void Bv002::detectChipTempSensors()
{
    int found = 0;
    for (int i = 0; i < BV002_TMP1075_COUNT; i++) {
        const float temp = TMP1075_read_temperature(i);
        if (temp == 0.0f) {
            break;
        }
        ESP_LOGI(TAG, "found chip TMP1075 %d (%.2f C, addr 0x%02x)",
                 i, temp, TMP1075_I2CADDR_DEFAULT + i);
#ifdef BV002_CHIP_PROBE
        m_probeTmpRawC = temp;
#endif
        found++;
    }
    m_numTempSensors = found;
    if (found == 0) {
        ESP_LOGW(TAG, "chip TMP1075 not found (addr 0x%02x)", TMP1075_I2CADDR_DEFAULT);
    }
}

bool Bv002::initBoard()
{
    if (!NerdQaxePlus2::initBoard()) {
        return false;
    }

    applyBv002AsicProfile();
    loadSettings();

    detectChipTempSensors();

#ifdef BV002_CHIP_PROBE
    probeChipsZeroCore();
    finalizeProbeReport();
#endif

    ESP_LOGI(TAG, "BV002 init done (version=%d, ethernet=W5500, temp_sensors=%d)",
             m_version, m_numTempSensors);

    return true;
}

float Bv002::getTemperature(int index)
{
    if (index < 0 || index >= m_numTempSensors) {
        return 0.0f;
    }
    return readChipTemp(index);
}

void Bv002::requestChipTemps()
{
    if (m_shutdown) {
        for (int i = 0; i < m_asicCount; i++) {
            setChipTemp(i, 0.0f);
        }
        return;
    }

    const float temp = readChipTemp(0);
    if (temp) {
        for (int i = 0; i < m_asicCount; i++) {
            setChipTemp(i, temp);
        }
    }
}
