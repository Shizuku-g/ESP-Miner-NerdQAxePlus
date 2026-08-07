#pragma once

#include "asic.h"
#include "bm1370.h"
#include "board.h"
#include "drivers/rev7/TPS546.h"
#include "nerdqaxeplus.h"

class NerdQaxePlus2 : public NerdQaxePlus {
  private:
    bool m_hasRev7TPS546 = false;

    void applyRev7Profile();
    void selectRev7BuckConverter();
    bool probeRev7Buck();

  protected:
    uint16_t m_rev7VoltageDomains = 2;
    virtual Rev7TPS546::TPS546_CONFIG createRev7Tps546Config();

  public:
    NerdQaxePlus2();
    bool initBoard() override;
    bool initAsics() override;
    bool setVoltage(float core_voltage) override;
    float getTemperature(int index);
    void requestChipTemps() override;
};
