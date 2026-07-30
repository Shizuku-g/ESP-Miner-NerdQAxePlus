#include "bv002.h"

#include "bm1373.h"
#include "drivers/rev7/TPS546.h"
#include "esp_log.h"

static const char *TAG = "bv002";

Bv002::Bv002() : NerdQaxePlus2()
{
    m_deviceModel = "BV002";
    m_miningAgent = m_deviceModel;
    m_asicModel = "BM1373";
    m_asicCount = 2;

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

bool Bv002::initBoard()
{
    if (!NerdQaxePlus2::initBoard()) {
        return false;
    }

    applyBv002AsicProfile();
    loadSettings();

    ESP_LOGI(TAG, "BV002 init done (version=%d, ethernet=W5500)", m_version);

    return true;
}
