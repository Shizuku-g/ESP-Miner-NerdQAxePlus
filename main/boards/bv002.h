#pragma once

#include "nerdqaxeplus2.h"

// BV002：2xBM1373 + W5500 Ethernet + 3-phase TPS546
class Bv002 : public NerdQaxePlus2 {
  private:
    void applyBv002AsicProfile();
    void detectChipTempSensors();

  protected:
    Rev7TPS546::TPS546_CONFIG createRev7Tps546Config() override;

#ifdef BV002_CHIP_PROBE
    void probeChipsZeroCore();
    void shutdownAsicPower();
    void finalizeProbeReport();

    struct Bv002ProbeReport {
        bool tmp1075Ok = false;
        float tmp1075C = 0.0f;
        bool w5500Ok = false;
        bool ethHasIp = false;
        bool tps546Ok = false;
        int chipsDetected = 0;
        int chipsExpected = 0;
    };

    Bv002ProbeReport m_probeReport;
    float m_probeTmpRawC = 0.0f;
#endif

  public:
    Bv002();
    bool initBoard() override;
#ifdef BV002_CHIP_PROBE
    bool initAsics() override;
    void formatProbeScreenText(char *buf, size_t len) const;
    void updateProbeEthStatus();
#endif
    bool hasEthernet() override { return true; }
    float getTemperature(int index) override;
    void requestChipTemps() override;
};
