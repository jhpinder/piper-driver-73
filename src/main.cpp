// main.cpp — Application entry point for the 73-driver board
//
// Written by HGP
//
// Uses Earle Philhower core for Raspberry Pi Pico (Arduino framework).
// Flash must include LittleFS storage for the config file.
// Upload config data with "Upload LittleFS to Pico" (Cmd-Shift-P).

#include <Arduino.h>

#include "config.h"
#include "hardware.h"
#include "midi_engine.h"
#include "pin_defs.h"
#include "status.h"

// ── Global instances ───────────────────────────────────────────────

static Config config;
static Hardware hardware;
static MidiEngine midi(config, hardware);
static Status status(hardware);

static bool lastSdPresent;
static int lastDipState;

static int configLoadErrors = 100;
static bool lastConfigLoadAttemptWasSd = true;
static bool shouldprintConfigCheck = true;

// ── Arduino entry points ───────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("\n\nPiper Output Driver 73 starting up...");

  PinDefs::initPins();

  // Raspberry Pi Pico has native 12-bit ADC
  analogReadResolution(12);

  // Wait for SD card to be inserted, time out after SD_INIT_TIMEOUT_MS
  int numTries = 0;
  int delayMs = 500;
  while (numTries < SD_INIT_TIMEOUT_MS / delayMs) {
    if (hardware.sdPresent()) {
      Serial.println("SD is present.");
      hardware.sdInit();
      hardware.printSdConfigFile();
      configLoadErrors = config.loadFromSdCard(hardware);
      if (configLoadErrors != 0) {
        Serial.print("Error populating config file from SD card:");
        Serial.println(configLoadErrors);
      } else {
        Serial.println("Midi tables have been configured.");
      }
      lastSdPresent = true;
      lastConfigLoadAttemptWasSd = true;
      break;
    } else {
      Serial.println("SD is not present, retrying... " + String(++numTries));
      lastSdPresent = false;
    }
    delay(delayMs);
  }

  // Load MIDI mapping tables from LittleFS
  if (configLoadErrors) {
    configLoadErrors = config.loadFromFlash();
    lastConfigLoadAttemptWasSd = false;
    if (configLoadErrors != 0) {
      Serial.print("Error populating config file from flash:");
      Serial.println(configLoadErrors);
    } else {
      Serial.println("Midi tables have been configured.");
    }
  }

  // Initialise UART for RS-485 MIDI reception
  midi.begin();

  // Initialise valve driver outputs (all off)
  hardware.initValves();

  // DIP switches
  {
    int d = hardware.readDipSwitches();
    Serial.print("DIP Switches state: ");
    Serial.println(d);
    lastDipState = d;
  }

  // Shift register self-test (must happen BEFORE SPI init)
  int u = hardware.selfTest();
  if (u) {
    Serial.print("Shift register test failed, ");
    Serial.println(u);
  } else {
    Serial.println("Shift register test passed.");
  }

  // Now initialise SPI for normal shift-register writes
  hardware.initSpi();
}

void loop() {

  // Check for DIP switch changes
  int r = hardware.readDipSwitches();
  if (r != lastDipState) {
    Serial.print("DIP Switches state: ");
    Serial.println(r);
    Serial.flush();
    lastDipState = r;
  }

  // Run the mode selected by DIP switches 1 & 2
  switch (r & 0x03) {
  case 0:
    status.cycleLeds();
    break;
  case 1:
    status.showDipSwitches();
    break;
  case 2:
    if (configLoadErrors && shouldprintConfigCheck) {
      Serial.print("Error populating config file from ");
      Serial.print(lastConfigLoadAttemptWasSd ? "SD card: " : "flash: ");
      Serial.println(configLoadErrors);
      shouldprintConfigCheck = false;
      return;
    }
    midi.process();
    break;
  case 3:
    Serial.print("Current measurement: ");
    Serial.println(status.readCurrentMa());
    Serial.flush();
    delay(1000);
    break;
  default:
    break;
  }

  // Hot-plug SD card detection
  bool sdNow = hardware.sdPresent();
  if (sdNow && !lastSdPresent) {
    Serial.println("Card inserted");
    hardware.sdInit();
    configLoadErrors = config.loadFromSdCard(hardware);
    lastConfigLoadAttemptWasSd = true;
    shouldprintConfigCheck = true;
  } else if (!sdNow && lastSdPresent) {
    hardware.sdEnd();
    Serial.println("Card removed");
    configLoadErrors = config.loadFromFlash();
    lastConfigLoadAttemptWasSd = false;
    shouldprintConfigCheck = true;
  }
  lastSdPresent = sdNow;
}
