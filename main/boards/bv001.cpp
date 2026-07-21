#include "bv001.h"

#include "bm1373.h"
#include "esp_log.h"

static const char *TAG = "bv001";

Bv001::Bv001() : NerdQaxePlus2()
{
    m_deviceModel = "BV001";
    m_miningAgent = m_deviceModel;
    m_asicModel = "BM1373";
    m_asicCount = 1;

    m_asics = new BM1373();
    m_vrFrequency = m_defaultVrFrequency = m_asics->getDefaultVrFrequency();

    applyBv001AsicProfile();

#ifdef BV001
    m_theme = new ThemeBv001();
#endif
}

void Bv001::applyBv001AsicProfile()
{
    m_asicJobIntervalMs = 500;
    m_version = 503;
    m_asicFrequencies = {550, 600, 650, 700, 750, 800};
    m_asicVoltages = {1000, 1050, 1100, 1150, 1200};
    m_defaultAsicFrequency = m_asicFrequency = 600;
    m_defaultAsicVoltageMillis = m_asicVoltageMillis = 1000;
    m_absMaxAsicFrequency = 1000;
    m_absMinAsicVoltageMillis = 900;
    m_absMaxAsicVoltageMillis = 1250;
    m_initVoltageMillis = 1050;

    m_asicMaxDifficulty = 4096;
    m_asicMinDifficulty = 1024;
    m_asicMinDifficultyDualPool = 512;
}

bool Bv001::initBoard()
{
    if (!NerdQaxePlus2::initBoard()) {
        return false;
    }

    applyBv001AsicProfile();
    loadSettings();

    ESP_LOGI(TAG, "BV001 init done (version=%d)", m_version);

    return true;
}
