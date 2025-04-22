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

AsyncUDP udpLocalPort;

#define ONBOARDPIXEL 1
tNeopixel onBoardPixel[ONBOARDPIXEL] = { };
tNeopixelContext onBoardPixelContext = neopixel_Init( ONBOARDPIXEL, 39 );

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
  onBoardPixel[0] = { 0, NP_RGB( 128, 0, 0 )};
  neopixel_SetPixel( onBoardPixelContext, onBoardPixel, ONBOARDPIXEL );

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

  if( udpLocalPort.listen( initialisation.portListenTo ) ) {
    udpLocalPort.onPacket( []( AsyncUDPPacket packet ) {
      uint8_t* data = packet.data();
      if( data[1] + ( data[0] << 8 ) != 0x8081 ){
          return;
      }
      uint16_t pgn = data[3] + ( data[2] << 8 );
      // see pgn.xlsx in https://github.com/farmerbriantee/AgOpenGPS/tree/master/AgOpenGPS_Dev
      switch( pgn ) {
        case 0x7FFE: {
          steerSetpoints.enabled = data[7];
          steerSetpoints.crossTrackError = data[10] - 127;
          steerSetpoints.requestedSteerAngle = (( double ) ((( int16_t ) data[8]) | (( int8_t ) data[9] << 8 ))) * 0.01; //horrible code to make negative doubles work

          steerSetpoints.lastPacketReceived = millis();

          if ( machine.steeringEnabled == steerSetpoints.enabled ) {
            // clear mismatch flag when machine control and AOG agree again
            machine.AogEngagedMismatch = false;
          } else if( machine.AogEngagedMismatch == false ) {
            // user pressed AOG autosteer button in software, we need to ACK so AOG listens to disengage
            machine.steeringEnabled = steerSetpoints.enabled; // reflect AOG state, will ACKNOWLEDGE when returned to AOG
          }
        }
        break;

        default:
          break;
      }
    } );
  }

  initSwitches();
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
