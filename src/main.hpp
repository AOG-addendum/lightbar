// MIT License
//
// Copyright (c) 2020 Christian Riggenbach
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <WiFi.h>
#include <WiFiMulti.h>
#include "SPIFFS.h"

#include <HTTPClient.h>

#include <AsyncUDP.h>

#include <ESPUI.h>

#include <Wire.h>

#include "neopixel.h"

extern uint16_t labelLoad;
extern uint16_t labelSupplyVoltage;
extern uint16_t labelSwitchStates;
extern uint16_t labelXTE;
extern uint16_t labelBrightness;
extern uint16_t widgetCmPerLightbarPixel;
extern uint16_t widgetCmPerDistInc;

extern uint16_t labelStatusOutput;
#define ONBOARDPIXEL 1
extern tNeopixel pixel[ONBOARDPIXEL];
extern tNeopixelContext onBoardNeopixel;

///////////////////////////////////////////////////////////////////////////
// Configuration
///////////////////////////////////////////////////////////////////////////


struct LightbarConfig {

  char ssid[24] = "AOG hub";
  char password[24] = "password";
  char hostname[24] = "ESP32-Lightbar";
  uint8_t apModePin = 13;

  uint32_t baudrate = 115200;

  bool enableOTA = false;

  uint8_t gpioSteerswitch = 6;
  uint8_t gpioApMode = 9;
  uint8_t gpioCDS = 8;
  uint8_t gpioPotentiometer = 5;
  bool steerSwitchActiveLow = false;
  bool steerSwitchIsMomentary = false;

  uint8_t numberOfPixels = 89;         // Odd number, dont use 0
  uint8_t cmPerLightbarPixel = 16;     // Must be a multiple of cmPerDistInt
  uint8_t cmPerDistInc = 2;            // The number of centimeters represented by a change in 1 of the AOG cross track error byte

  uint16_t aogPortSendFrom = 5577;
  uint16_t aogPortListenTo = 8888;
  uint16_t aogPortSendTo = 9999;

};
extern LightbarConfig lightbarConfig, lightbarConfigDefaults;

struct Brightness {

  uint16_t potentiometer = 0;
  uint16_t cdsCell = 0;
  uint16_t ledOutput = 0;

};
extern Brightness brightness;

struct Initialisation {

  uint16_t portSendFrom = 5577;
  uint16_t portListenTo = 8888;
  uint16_t portSendTo = 9999;

};
extern Initialisation initialisation;

///////////////////////////////////////////////////////////////////////////
// Global Data
///////////////////////////////////////////////////////////////////////////

struct SteerSetpoints {
  int16_t crossTrackError = 0;
  double requestedSteerAngle = 0;
  bool enabled = false;

  time_t lastPacketReceived = 0;
};
extern SteerSetpoints steerSetpoints;

struct Machine {
  bool steeringEnabled = false; // ESP32 internal steering state > sent to AOG
};
extern Machine machine;

///////////////////////////////////////////////////////////////////////////
// external Libraries
///////////////////////////////////////////////////////////////////////////

extern AsyncUDP udpSendFrom;

///////////////////////////////////////////////////////////////////////////
// Helper Functions
///////////////////////////////////////////////////////////////////////////
extern void setResetButtonToRed();

extern void initESPUI();
extern void initIdleStats();
extern void initWiFi();
extern void initLightbar();
extern void initDiagnostics();
