#pragma once

#include "nerdqaxeplus2.h"

// BV003：4xBM1373 + W5500 Ethernet + 4-phase TPS546
class Bv003 : public NerdQaxePlus2 {
  private:
    void applyBv003AsicProfile();
    void detectChipTempSensors();

  protected:
    Rev7TPS546::TPS546_CONFIG createRev7Tps546Config() override;

  public:
    Bv003();
    bool initBoard() override;
    bool hasEthernet() override { return true; }
    float getTemperature(int index) override;
    void requestChipTemps() override;
};
