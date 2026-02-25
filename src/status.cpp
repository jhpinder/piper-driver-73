// status.cpp — Current sensing, overcurrent protection, and diagnostic test modes
#include "status.h"
#include "hardware.h"
#include "pin_defs.h"

using namespace PinDefs;

Status::Status(Hardware &hw)
    : hw_(hw)
{
}

int Status::readCurrentMa() const
{
    int raw = analogRead(current_sense);
    /*  12-bit ADC: 4095 = 3.3 V = 33 A (33 000 mA).
     *  Each LSB ≈ 8.057 mA.
     *  Approximation: raw*8 + raw/16 - raw/256 - raw/512  */
    return (raw * 8) + (raw >> 4) - (raw >> 8) - (raw >> 9);
}

bool Status::checkOvercurrent()
{
    if (digitalRead(not_overcurrent_alert) == 0 ||
        readCurrentMa() > MAX_CURRENT_MA) {
        hw_.initValves();
        return true;
    }
    return false;
}

// ── Test / demo modes ──────────────────────────────────────────────

void Status::cycleLeds()
{
    constexpr int bot = 1;
    constexpr int top = 73;

    for (int i = bot; i <= top; i++) {
        hw_.setValveState(i, 1);
        hw_.writeValves();
        delay(200);
        hw_.setValveState(i, 0);
        hw_.writeValves();
        delay(50);
    }

    for (int i = bot; i <= top; i++) {
        hw_.setValveState(i, 1);
        hw_.writeValves();
        delay(41);
        hw_.setValveState(i, 0);
        hw_.writeValves();
    }
}

void Status::showDipSwitches()
{
    int r = hw_.readDipSwitches();

    hw_.setValveState(73, (r >> 2) & 1);
    hw_.setValveState(72, (r >> 3) & 1);
    hw_.setValveState(71, ~digitalRead(unused_input_0));
    hw_.setValveState(6,  ~digitalRead(unused_input_1));
    hw_.setValveState(8,  ~digitalRead(unused_input_3));
    hw_.setValveState(9,  ~digitalRead(unused_input_5));
    hw_.setValveState(10, ~digitalRead(unused_input_6));
    hw_.writeValves();

    delay(5);
}
