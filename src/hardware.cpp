// hardware.cpp — Valve I/O, shift-register control, SD card, DIP switches
#include "hardware.h"
#include "pin_defs.h"

using namespace PinDefs;

// ── Valve-number → byte-offset / bit-mask lookup tables ────────────

/*
 *  Valve state layout (transmitted via SPI to 9 shift registers):
 *  valve_state[0] =  72 71 70 69 68 67 66 65
 *  valve_state[1] =  64 63 62 61 60 59 58 57
 *  valve_state[2] =  56 55 54 53 52 51 50 49
 *  valve_state[3] =  48 47 46 45 44 43 42 41
 *  valve_state[4] =  40 39 38 37 36 35 34 33
 *  valve_state[5] =  32 31 30 29 28 27 26 25
 *  valve_state[6] =  24 23 22 21 20 19 18 17
 *  valve_state[7] =  16 15 14 13 12 11 10  9
 *  valve_state[8] =   8  7  6  5  4  3  2  1
 *  valve_state[9] =   x  x  x  x  x  x  x 73
 */
const int Hardware::kValveByte[74] = {
    0, // dummy so valve numbers 1-73 can index directly
    8, 8, 8, 8, 8, 8, 8, 8,
    7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    9 // valve 73 — driven directly, not via shift registers
};

const int Hardware::kValveMask[74] = {
    0, // dummy
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
    0x01};

// ── Construction ───────────────────────────────────────────────────

Hardware::Hardware()
{
    memset(valveState_, 0, sizeof(valveState_));
}

// ── Valve control ──────────────────────────────────────────────────

void Hardware::initValves()
{
    digitalWrite(to_shift_reg, 0);
    digitalWrite(shift_reg_clock, 0);

    // Clear shift registers
    digitalWrite(not_shift_reg_clr, 1);
    digitalWrite(not_shift_reg_clr, 0);
    digitalWrite(not_shift_reg_clr, 1);

    // Latch to outputs
    digitalWrite(shift_reg_load, 0);
    digitalWrite(shift_reg_load, 1);
    digitalWrite(shift_reg_load, 0);

    // Enable output drivers
    digitalWrite(not_shift_reg_oe, 0);

    // Turn off Valve 73
    digitalWrite(Drive73, 0);

    clearValveState();
}

void Hardware::setValveState(int valveNumber, int value)
{
    if (value)
    {
        valveState_[kValveByte[valveNumber]] |= kValveMask[valveNumber];
    }
    else
    {
        valveState_[kValveByte[valveNumber]] &= ~kValveMask[valveNumber];
    }
}

void Hardware::writeValves()
{
    writeValvesSpi(valveState_);
}

void Hardware::writeValves(uint8_t *valveState)
{
    writeValvesSpi(valveState);
}

void Hardware::clearValveState()
{
    memset(valveState_, 0, sizeof(valveState_));
}

uint8_t *Hardware::getValveState()
{
    return valveState_;
}

// ── Low-level valve write methods ──────────────────────────────────

void Hardware::writeValvesBitBang(uint8_t *valveState)
{
    for (int i = 0; i < 9; i++)
    {
        uint8_t a = valveState[i];
        for (int j = 0; j < 8; j++)
        {
            digitalWrite(to_shift_reg, a & 0x01);
            a >>= 1;
            digitalWrite(shift_reg_clock, 1);
            digitalWrite(shift_reg_clock, 0);
        }
    }

    // Latch shift register state to outputs
    digitalWrite(shift_reg_load, 1);
    digitalWrite(shift_reg_load, 0);

    // Write the 73rd output directly
    digitalWrite(Drive73, valveState[9] & 0x01);
}

void Hardware::writeValvesSpi(uint8_t *valveState)
{
    uint8_t rxData[VALVE_STATE_SIZE];
    SPI.beginTransaction(SPISettings(4000000, LSBFIRST, SPI_MODE0));
    SPI.transfer(valveState, rxData, VALVE_STATE_SIZE - 1);
    SPI.endTransaction();

    // Latch shift register state to outputs
    digitalWrite(shift_reg_load, 1);
    digitalWrite(shift_reg_load, 0);

    // Write the 73rd output directly
    digitalWrite(Drive73, valveState[9] & 0x01);
}

// ── Shift register self-test ───────────────────────────────────────

int Hardware::selfTest()
{
    int i;

    static const bool testPattern[72] = {
        0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1,
        1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1};

    // Shift out test pattern
    for (i = 0; i < 72; i++)
    {
        digitalWrite(to_shift_reg, testPattern[i]);
        digitalWrite(shift_reg_clock, 1);
        digitalWrite(shift_reg_clock, 0);
    }
    // Read back
    for (i = 0; i < 72; i++)
    {
        if (digitalRead(loopback_from_shift_reg) != testPattern[i])
            break;
        digitalWrite(shift_reg_clock, 1);
        digitalWrite(shift_reg_clock, 0);
    }
    if (i != 72)
        return (i + 1);

    // Inverse pattern
    for (i = 0; i < 72; i++)
    {
        digitalWrite(to_shift_reg, !testPattern[i]);
        digitalWrite(shift_reg_clock, 1);
        digitalWrite(shift_reg_clock, 0);
    }
    for (i = 0; i < 72; i++)
    {
        if (digitalRead(loopback_from_shift_reg) != !testPattern[i])
            break;
        digitalWrite(shift_reg_clock, 1);
        digitalWrite(shift_reg_clock, 0);
    }
    if (i != 72)
        return (i + 100);

    // Clear and verify
    digitalWrite(not_shift_reg_clr, 0);
    digitalWrite(not_shift_reg_clr, 1);
    for (i = 0; i < 72; i++)
    {
        if (digitalRead(loopback_from_shift_reg) != 0)
            break;
        digitalWrite(shift_reg_clock, 1);
        digitalWrite(shift_reg_clock, 0);
    }
    if (i != 72)
        return (i + 200);

    return 0;
}

// ── SPI setup ──────────────────────────────────────────────────────

void Hardware::initSpi()
{
    SPI.setTX(to_shift_reg);
    SPI.setRX(loopback_from_shift_reg);
    SPI.setSCK(shift_reg_clock);
    SPI.begin(false); // don't use hardware chip select
}

// ── MicroSD ────────────────────────────────────────────────────────

bool Hardware::sdPresent() const
{
    return !digitalRead(SD_detect);
}

void Hardware::sdInit()
{
    SPI1.setRX(SD_rx_miso);
    SPI1.setTX(SD_tx_mosi);
    SPI1.setSCK(SD_sck);
    SPI1.setCS(SD_csn);
    SD.begin(SD_csn, SPI_HALF_SPEED, SPI1);
}

void Hardware::sdEnd()
{
    SD.end();
}

void Hardware::printSdConfigFile()
{
    File fp = SD.open("/config.txt");
    if (fp)
    {
        char buffer[64];
        while (fp.available())
        {
            int n = fp.readBytesUntil('\n', buffer, sizeof(buffer));
            buffer[n] = 0;
            Serial.print("Read from file: ");
            Serial.println(buffer);
        }
        fp.close();
    }
    else
    {
        Serial.println("Error opening file for reading");
    }
}

// ── DIP switches ───────────────────────────────────────────────────

int Hardware::readDipSwitches() const
{
    int a = 0;
    if (digitalRead(dip_switches[3]))
        a |= 0x08;
    if (digitalRead(dip_switches[2]))
        a |= 0x04;
    if (digitalRead(dip_switches[1]))
        a |= 0x02;
    if (digitalRead(dip_switches[0]))
        a |= 0x01;
    return a;
}
