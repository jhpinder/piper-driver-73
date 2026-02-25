// pin_defs.cpp — Pin assignments and initialization for the 73-driver board
#include "pin_defs.h"

namespace PinDefs {

const int dip_switches[4] = {16, 17, 20, 28};  // DIP1, DIP2, DIP3, DIP4

const int shift_reg_clock        = 18;
const int to_shift_reg           = 19;
const int not_shift_reg_oe       = 21;
const int shift_reg_load         = 22;
const int not_shift_reg_clr      = 27;
const int loopback_from_shift_reg = 4;

const int current_sense          = 26;
const int not_overcurrent_alert  = 14;

const int RxD                    = 13;

const int NeoOut                 = 12;
const int NeoOut2                = 15;

const int SD_detect              = 7;
const int SD_rx_miso             = 8;
const int SD_csn                 = 9;
const int SD_sck                 = 10;
const int SD_tx_mosi             = 11;

const int Drive73                = 2;

const int unused_input_0         = 0;
const int unused_input_1         = 1;
const int unused_input_3         = 3;
const int unused_input_5         = 5;
const int unused_input_6         = 6;

void initPins()
{
    // DIP switches – active-low with internal pull-up
    for (int i = 0; i < 4; i++) {
        pinMode(dip_switches[i], INPUT_PULLUP);
    }

    // Shift register control
    pinMode(to_shift_reg, OUTPUT);
    digitalWrite(to_shift_reg, 0);
    pinMode(shift_reg_clock, OUTPUT);
    digitalWrite(shift_reg_clock, 0);
    pinMode(not_shift_reg_oe, OUTPUT);
    digitalWrite(not_shift_reg_oe, 1);
    pinMode(shift_reg_load, OUTPUT);
    digitalWrite(shift_reg_load, 0);
    pinMode(not_shift_reg_clr, OUTPUT);
    digitalWrite(not_shift_reg_clr, 1);

    // Analog / alert
    pinMode(current_sense, INPUT);
    pinMode(not_overcurrent_alert, INPUT);

    // UART RX
    pinMode(RxD, INPUT);

    // NeoPixel
    pinMode(NeoOut, OUTPUT);
    digitalWrite(NeoOut, 0);
    pinMode(NeoOut2, INPUT);

    // MicroSD card detect
    pinMode(SD_detect, INPUT_PULLUP);

    // Shift register loopback (for self-test)
    pinMode(loopback_from_shift_reg, INPUT);

    // Direct valve driver 73
    pinMode(Drive73, OUTPUT);
    digitalWrite(Drive73, 0);

    // Spare inputs used for test modes
    pinMode(unused_input_0, INPUT_PULLUP);
    pinMode(unused_input_1, INPUT_PULLUP);
    pinMode(unused_input_3, INPUT_PULLUP);
    pinMode(unused_input_5, INPUT_PULLUP);
    pinMode(unused_input_6, INPUT_PULLUP);

    // Built-in LED
    pinMode(LED_BUILTIN, OUTPUT);
}

}  // namespace PinDefs
