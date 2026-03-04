// main.cpp — Application entry point for the 73-driver board
//
// Written by HGP
//
// Uses Earle Philhower core for Raspberry Pi Pico (Arduino framework).
// Flash must include LittleFS storage for the MIDI config file.
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

// ── Arduino entry points ───────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("\n\nPiper Driver 73 starting up...");

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
      lastSdPresent = true;
      break;
    } else {
      Serial.println("SD is not present.");
      lastSdPresent = false;
    }
    delay(delayMs);
    numTries++;
  }

  // Load MIDI mapping tables from LittleFS
  int err = config.loadFromFlash();
  if (err != 0) {
    Serial.print("Error populating midi config file:");
    Serial.println(err);
  } else {
    Serial.println("Midi tables have been configured.");
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
    hardware.printSdConfigFile();
  } else if (!sdNow && lastSdPresent) {
    hardware.sdEnd();
    Serial.println("Card removed");
  }
  lastSdPresent = sdNow;
}
