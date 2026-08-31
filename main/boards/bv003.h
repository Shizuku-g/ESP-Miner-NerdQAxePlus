#pragma once

#include "nerdqaxeplus2.h"

// BV003：4xBM1373 + W5500 Ethernet + 4-phase TPS546
class Bv003 : public NerdQaxePlus2 {
  private:
    void applyBv003AsicProfile();
    void detectChipTempSensors();

  protected:
    Rev7TPS546::TPS546_CONFIG createRev7Tps546Config() override;

#ifdef BV003_CHIP_PROBE
    void probeChipsZeroCore();
    void shutdownAsicPower();
    void finalizeProbeReport();

    struct Bv003ProbeReport {
        bool tmp1075Ok = false;
        float tmp1075C0 = 0.0f;
        float tmp1075C1 = 0.0f;
        bool w5500Ok = false;
        bool ethHasIp = false;
        bool tps546Ok = false;
        int chipsDetected = 0;
        int chipsExpected = 0;
    };

    Bv003ProbeReport m_probeReport;
    float m_probeTmpRawC[2] = {0.0f, 0.0f};
#endif

  public:
    Bv003();
    bool initBoard() override;
    void LDO_enable() override {}
    void LDO_disable() override {}
#ifdef BV003_CHIP_PROBE
    bool initAsics() override;
    void formatProbeScreenText(char *buf, size_t len) const;
    void updateProbeEthStatus();
#endif
    bool hasEthernet() override { return true; }
    float getTemperature(int index) override;
    void requestChipTemps() override;
};
