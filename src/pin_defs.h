// pin_defs.h — Pin assignments for the 73-driver board
#pragma once

#include <Arduino.h>

namespace PinDefs {

// DIP switches (active-low with pull-up)
extern const int dip_switches[4];  // DIP1, DIP2, DIP3, DIP4

// Shift register control
extern const int shift_reg_clock;
extern const int to_shift_reg;
extern const int not_shift_reg_oe;
extern const int shift_reg_load;
extern const int not_shift_reg_clr;
extern const int loopback_from_shift_reg;

// Analog / alert
extern const int current_sense;
extern const int not_overcurrent_alert;

// UART / RS-485
extern const int RxD;

// NeoPixel
extern const int NeoOut;
extern const int NeoOut2;

// MicroSD (SPI1)
extern const int SD_detect;
extern const int SD_rx_miso;
extern const int SD_csn;
extern const int SD_sck;
extern const int SD_tx_mosi;

// Direct valve driver for output 73
extern const int Drive73;

// Spare inputs (active-low, used in test modes)
extern const int unused_input_0;
extern const int unused_input_1;
extern const int unused_input_3;
extern const int unused_input_5;
extern const int unused_input_6;

/// Configure every pin's direction and initial state.
void initPins();

}  // namespace PinDefs
