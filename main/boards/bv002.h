#pragma once

#include "nerdqaxeplus2.h"

// BV002：2xBM1373 + W5500 Ethernet + 3-phase TPS546
class Bv002 : public NerdQaxePlus2 {
  private:
    void applyBv002AsicProfile();

  protected:
    Rev7TPS546::TPS546_CONFIG createRev7Tps546Config() override;

  public:
    Bv002();
    bool initBoard() override;
    bool hasEthernet() override { return true; }
};
