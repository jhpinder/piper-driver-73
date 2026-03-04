// midi_engine.cpp — MIDI message reception and valve dispatch
#include "midi_engine.h"
#include "config.h"
#include "hardware.h"
#include "pin_defs.h"

MidiEngine::MidiEngine(const Config& config, Hardware& hw) : config_(config), hw_(hw) {}

void MidiEngine::begin() {
  Serial1.setRX(PinDefs::RxD);
  Serial1.begin(1000000);
}

void MidiEngine::process() {
  while (Serial1.available() >= 2) {
    Serial.write(Serial1.peek());   // Echo the raw MIDI byte for debugging
    uint8_t byte1 = Serial1.peek(); // Peek first to check validity
    uint8_t status_nibble = (byte1 >> 4) & 0x0F;

    // Check if this could be a valid MIDI status byte (8=Note Off, 9=Note On)
    if (status_nibble != 8 && status_nibble != 9) {
      // Invalid status byte - discard and try to resync
      Serial1.read(); // consume the bad byte
      continue;
    }

    // We have a potentially valid status byte, need the note byte too
    if (Serial1.available() < 2) {
      // Not enough data yet, wait for the next byte
      break;
    }

    int cmd = Serial1.read();
    Serial.write(Serial1.peek()); // Echo the note byte for debugging
    int note = Serial1.read() & 0x7F;
    int channel = cmd & 0x0F;
    cmd = (cmd >> 4) & 0x0F;

    int valve = config_.getValve(channel, note);

    // Note Off
    if (valve != 0 && cmd == 8) {
      hw_.setValveState(valve, 0);
      hw_.writeValves();
    }
    // Note On
    else if (valve != 0 && cmd == 9) {
      hw_.setValveState(valve, 1);
      hw_.writeValves();
    }
    // All Notes Off (CC 123) — currently disabled, kept for reference
    // else if (cmd == 11 && note == 123) { hw_.initValves(); }
  }
}
