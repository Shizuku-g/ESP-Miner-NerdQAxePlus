#pragma once

#include "nerdqaxeplus2.h"

// BV002：2 颗 BM1373 + W5500 以太网
class Bv002 : public NerdQaxePlus2 {
  private:
    void applyBv002AsicProfile();

  public:
    Bv002();
    bool initBoard() override;
    bool hasEthernet() override { return true; }
};
