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

#include <stdio.h>
#include <string.h>

#include "jsonFunctions.hpp"
#include "main.hpp"

#include <ESPmDNS.h>
#include <WiFi.h>

#include <DNSServer.h>
#include <ESPUI.h>

#include <AsyncElegantOTA.h>
#include "driver/gpio.h"

///////////////////////////////////////////////////////////////////////////
// global data
///////////////////////////////////////////////////////////////////////////

LightbarConfig lightbarConfig, lightbarConfigDefaults;
Brightness brightness;
Initialisation initialisation;
SteerSetpoints steerSetpoints;
Machine machine;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
SemaphoreHandle_t i2cMutex;

#define ONBOARDPIXEL 1
tNeopixel pixel[ONBOARDPIXEL] = { };
tNeopixelContext onBoardNeopixel = neopixel_Init( ONBOARDPIXEL, 39 );

const byte DNS_PORT = 53;
IPAddress apIP( 192, 168, 1, 1 );

///////////////////////////////////////////////////////////////////////////
// external Libraries
///////////////////////////////////////////////////////////////////////////
DNSServer dnsServer;

///////////////////////////////////////////////////////////////////////////
// Application
///////////////////////////////////////////////////////////////////////////
void setup( void ) {
  Serial.begin( 115200 );

  WiFi.disconnect( true );

  if( !LittleFS.begin( true ) ) {
    Serial.println( "LittleFS Mount Failed" );
    return;
  }

  loadSavedConfig();

  Serial.println( "Welcome to esp32-lightbar.\nTo configure, please open the webui." );

  pinMode( 38, OUTPUT );
  digitalWrite( 38, HIGH );
  pixel[0] = { 0, NP_RGB( 128, 0, 0 )};
  neopixel_SetPixel( onBoardNeopixel, pixel, ONBOARDPIXEL );

  initWiFi();
  apIP = WiFi.localIP();

  dnsServer.start( DNS_PORT, "*", apIP );
  
  Serial.print( "\n\nWiFi parameters:" );
  Serial.print( "Mode: " );
  Serial.println( WiFi.getMode() == WIFI_AP ? "Station" : "Client" );
  Serial.print( "IP address: " );
  Serial.println( WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP() );


  /*
  * .begin loads and serves all files from PROGMEM directly.
  * If you want to serve the files from LittleFS use ESPUI.beginSPIFFS
  * (.prepareFileSystem has to be run in an empty sketch before)
  */

  /*
  * Optionally you can use HTTP BasicAuth. Keep in mind that this is NOT a
  * SECURE way of limiting access.
  * Anyone who is able to sniff traffic will be able to intercept your password
  * since it is transmitted in cleartext. Just add a username and password,
  * for example begin("ESPUI Control", "username", "password")
  */

  initESPUI();
  initLightbar();

  if( lightbarConfig.enableOTA ) {
    AsyncElegantOTA.begin( ESPUI.WebServer() );
  }
  initIdleStats();
  initDiagnostics();

  if( !MDNS.begin( "lightbar" )){
    Serial.println( "Error starting mDNS" );
  }
}

void loop( void ) {
  dnsServer.processNextRequest();
  vTaskDelay( 100 );
}
