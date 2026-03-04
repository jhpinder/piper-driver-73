// config.cpp — MIDI configuration tables (loaded from LittleFS)
#include "config.h"

#include <LittleFS.h>
#include <VFS.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

Config::Config() {
  memset(valveToChannel, 0, sizeof(valveToChannel));
  memset(valveToNote, 0, sizeof(valveToNote));
  memset(channelNoteToValve_, 0, sizeof(channelNoteToValve_));
}

int Config::loadFromFlash() {
  Serial.println("Loading MIDI configuration from flash...");
  if (!LittleFS.begin())
    return 1;

  VFS.root(LittleFS);

  FILE* fp = fopen(CONFIG_PATH, "r");
  if (fp == nullptr)
    return 2;

  char buffer[64];
  while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
    char* token;
    int valve, channel, note;

    token = strtok(buffer, ",");
    if (token == nullptr) {
      fclose(fp);
      return 3;
    }
    valve = atoi(token);

    token = strtok(nullptr, ",");
    if (token == nullptr) {
      fclose(fp);
      return 4;
    }
    channel = atoi(token);

    token = strtok(nullptr, ",");
    if (token == nullptr) {
      fclose(fp);
      return 5;
    }
    note = atoi(token);

    if (valve < 1) {
      fclose(fp);
      return 6;
    }
    if (valve > MAX_VALVES) {
      fclose(fp);
      return 7;
    }
    if (channel < 0) {
      fclose(fp);
      return 8;
    }
    if (channel > 15) {
      fclose(fp);
      return 9;
    }
    if (note < 0) {
      fclose(fp);
      return 10;
    }
    if (note > 127) {
      fclose(fp);
      return 11;
    }

    // Forward mapping (last definition wins for duplicates)
    valveToChannel[valve] = static_cast<uint8_t>(channel);
    valveToNote[valve] = static_cast<uint8_t>(note);

    // Reverse mapping
    channelNoteToValve_[channel][note] = static_cast<uint8_t>(valve);

    // Channel 0 means "all channels"
    if (channel == 0) {
      for (int i = 0; i < 16; i++) {
        channelNoteToValve_[i][note] = static_cast<uint8_t>(valve);
      }
    }
  }
  fclose(fp);
  return 0;
}

int Config::getValve(int channel, int note) const { return channelNoteToValve_[channel][note]; }
