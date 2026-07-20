#pragma once

#include "nerdqaxeplus2.h"

// BV001：Based on NerdQAxe++, featuring a single BM1373 chip, with all other hardware unchanged.
class Bv001 : public NerdQaxePlus2 {
  private:
    void applyBv001AsicProfile();

  public:
    Bv001();
    bool initBoard() override;
};
