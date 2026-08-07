#include "bv003.h"

#include "bm1373.h"
#include "drivers/rev7/TPS546.h"
#include "drivers/TMP1075.h"
#include "esp_log.h"

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
