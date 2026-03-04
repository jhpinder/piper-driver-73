// hardware.h — Valve I/O, shift-register control, SD card, DIP switches
#pragma once

#include "config.h"

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

/**
 * Manages all low-level board hardware:
 *   - 73 valve drivers (72 via shift registers + 1 direct GPIO)
 *   - SPI interface to shift register chain
 *   - MicroSD card detection and initialisation
 *   - DIP switch reading
 */
class Hardware {
public:
  /// Number of bytes in the valve state array (72 SR bits + 1 direct = 10 bytes)
  static constexpr int VALVE_STATE_SIZE = 10;

  Hardware();

  // ── Valve control ──────────────────────────────────────────────

  /** Initialise shift registers and output drivers; all valves off. */
  void initValves();

  /** Set a single valve (1-73) to on (1) or off (0) in the state buffer. */
  void setValveState(int valveNumber, int value);

  /** Push the current valve state buffer to hardware (SPI + direct GPIO). */
  void writeValves();

  /** Push an external valve state buffer to hardware. */
  void writeValves(uint8_t* valveState);

  /** Clear the valve state buffer (all off). */
  void clearValveState();

  /** Direct access to the valve state buffer. */
  uint8_t* getValveState();

  // ── Shift register self-test ───────────────────────────────────

  /** Bit-bang a test pattern through the shift registers and verify.
   *  @return 0 on success, non-zero indicates failure position. */
  int selfTest();

  // ── SPI setup ──────────────────────────────────────────────────

  /** Configure SPI0 for shift register communication. Call AFTER selfTest(). */
  void initSpi();

  // ── MicroSD ────────────────────────────────────────────────────

  /** @return true if the MicroSD card-detect pin is active (card present). */
  bool sdPresent() const;

  /** Initialise SD card on SPI1 (call after card insertion). */
  void sdInit();

  /** End SD session (call on card removal). */
  void sdEnd();

  /** Print the contents of /config.txt on the SD card to Serial. */
  void printSdConfigFile();

  // ── DIP switches ───────────────────────────────────────────────

  /** Read the 4-bit DIP switch value (0-15).
   *  DIP4 = bit 3 (MSB), DIP1 = bit 0 (LSB).
   *  "On" = 0. */
  int readDipSwitches() const;

private:
  uint8_t valveState_[VALVE_STATE_SIZE];

  // Valve-number-to-byte/mask mapping tables (index 0 unused; 1-73 valid)
  static const int kValveByte[74];
  static const int kValveMask[74];

  /** Low-level bit-bang write (used by selfTest path, not normal operation). */
  void writeValvesBitBang(uint8_t* valveState);

  /** Low-level SPI write (normal operation). */
  void writeValvesSpi(uint8_t* valveState);
};
