// midi_engine.h — MIDI message reception and valve dispatch
#pragma once

#include "piper-midi.h"
#include <Arduino.h>

class Config;
class Hardware;

/**
 * Reads 2-byte MIDI-like messages from the RS-485 UART (Serial1) and
 * translates them into valve on/off actions via the Hardware layer.
 */
class MidiEngine {
public:
  /**
   * @param config  Reference to the MIDI configuration tables.
   * @param hw      Reference to the hardware driver.
   */
  MidiEngine(const Config& config, Hardware& hw);

  /** Initialise UART0 (Serial1) for RS-485 reception at 1 Mbaud. */
  void begin();

  /**
   * Call from loop().  If a complete 2-byte message is available,
   * parse it and drive the corresponding valve.
   */
  void process();

private:
  const Config& config_;
  Hardware& hw_;
};
