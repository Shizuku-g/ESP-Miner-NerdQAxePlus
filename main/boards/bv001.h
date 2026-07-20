#pragma once

#include "nerdqaxeplus2.h"

// BV001：基于 NerdQAxe++，单颗 BM1373，其余硬件不变
class Bv001 : public NerdQaxePlus2 {
  private:
    void applyBv001AsicProfile();

  public:
    Bv001();
    bool initBoard() override;
};
