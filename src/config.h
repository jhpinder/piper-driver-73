// config.h — MIDI configuration tables (loaded from LittleFS)
#pragma once

#include <Arduino.h>

/// Maximum number of valve drivers on the board.
static constexpr int MAX_VALVES = 73;

/**
 * Manages the mapping between MIDI channel/note pairs and physical valve
 * numbers.  Tables are populated once at startup from a config file stored
 * in LittleFS (on-board flash).
 */
class Config {
public:
    Config();

    /**
     * Read "midi_config.txt" from LittleFS and populate the lookup tables.
     * @return 0 on success, non-zero error code on failure.
     */
    int loadFromFlash();

    /**
     * Look up the valve number for a given MIDI channel and note.
     * @return valve number (1-73), or 0 if no mapping exists.
     */
    int getValve(int channel, int note) const;

    /// Forward lookup: valve → channel  (index 1–73 valid)
    uint8_t valveToChannel[MAX_VALVES + 1];
    /// Forward lookup: valve → note     (index 1–73 valid)
    uint8_t valveToNote[MAX_VALVES + 1];

private:
    /// Reverse lookup: [channel][note] → valve (0 = unmapped)
    uint8_t channelNoteToValve_[16][128];
};
