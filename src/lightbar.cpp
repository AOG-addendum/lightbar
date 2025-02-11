// MIT License
//
// Copyright (c) 2024 Reuben Rissler based on work of 
// https://github.com/chriskinal/AOG_Lightbar_UDP_WiFi_ESP32/blob/main/src/main.cpp
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
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "neopixel.h"

#include "main.hpp"
#include "jsonFunctions.hpp"

AsyncUDP udpLocalPort;

#define NUMPIXELS 89    // Odd number, dont use 0
#define GPIO_LED 35

void getBrightnessLevels(){
  brightness.cdsCell = analogRead( lightbarConfig.gpioCDS ) / 16;
  int16_t maxBrightness = 255 - brightness.cdsCell;
  int16_t minBrightness = -brightness.cdsCell;
  brightness.potentiometer = analogRead( lightbarConfig.gpioPotentiometer ) / 16;
  brightness.ledOutput = map( brightness.potentiometer, 0, 255, minBrightness, maxBrightness );
  brightness.ledOutput += brightness.cdsCell;
  brightness.ledOutput = constrain( brightness.ledOutput, 0, 255 );
}

void lightbarWorker1Hz( void* z ) {
  constexpr TickType_t xFrequency = 200;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  tNeopixelContext neopixel = neopixel_Init( NUMPIXELS, GPIO_LED );
  for( ;; ) {

    getBrightnessLevels();

    uint8_t centerpixel = ( NUMPIXELS-1 ) / 2;
    uint8_t cmPerLBPixel = lightbarConfig.cmPerLightbarPixel / lightbarConfig.cmPerDistInc;
    int8_t level = constrain( (int8_t)( steerSetpoints.crossTrackError / cmPerLBPixel ), -centerpixel, centerpixel);
    int8_t n = level + centerpixel;
    tNeopixel pixel[NUMPIXELS] = { };
    if( steerSetpoints.enabled == false ){
      for ( uint8_t i = 0; i < NUMPIXELS; i++ ){
        if ( i == centerpixel ){ //Center
          pixel[i] = { i, NP_RGB( 0, 0, brightness.ledOutput )}; // blue
        } else if ( i < centerpixel && i > ( centerpixel - 4 )){ //Right Bar
          pixel[i] = { i, NP_RGB( 0, brightness.ledOutput, 0 )}; // green
        } else if ( i > centerpixel && i < ( centerpixel + 4 )){ //Left Bar
          pixel[i] = { i, NP_RGB( brightness.ledOutput, 0, 0 )}; // red
        } else {
          pixel[i] = { i, NP_RGB( 0, 0, 0 )};
        }
      }
    } else {
      for ( uint8_t i = 0; i < NUMPIXELS; i++ ){
        if ( i == centerpixel && i == n ){//Center
          pixel[i] = { i, NP_RGB( 0, 0, brightness.ledOutput )}; // blue
        } else if ( level < 0 && i >= n && i < centerpixel ){ //Right Bar
          pixel[i] = { i, NP_RGB( 0, brightness.ledOutput, 0 )}; // green
        } else if ( level > 0 && i <= n && i > centerpixel ){ //Left Bar
          pixel[i] = { i, NP_RGB( brightness.ledOutput, 0, 0 )}; // red
        } else {
          pixel[i] = { i, NP_RGB( 0, 0, 0 )};
        }
      }
    }
    neopixel_SetPixel( neopixel, pixel, NUMPIXELS );
    vTaskDelay( pdMS_TO_TICKS( 50 ));
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void initLightbar() {

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
        }
        break;

        default:
          break;
      }
    } );
  }

  pinMode(( uint8_t )lightbarConfig.gpioCDS, INPUT );
  pinMode(( uint8_t ) lightbarConfig.gpioPotentiometer, INPUT );
  getBrightnessLevels();
  pinMode( GPIO_LED, OUTPUT );
  tNeopixelContext neopixel = neopixel_Init( NUMPIXELS, GPIO_LED );
  tNeopixel pixel[NUMPIXELS] = { };

  for ( uint8_t i = 0; i < NUMPIXELS; i++ ){
    pixel[i] = { i, NP_RGB( brightness.ledOutput, 0, 0 )}; //red
  }
  neopixel_SetPixel( neopixel, pixel, NUMPIXELS );
  Serial.println("red test");
  vTaskDelay( 1000 );

  for ( uint8_t i = 0; i < NUMPIXELS; i++ ){
    pixel[i] = { i, NP_RGB( 0, brightness.ledOutput, 0 )}; // green
  }
  neopixel_SetPixel( neopixel, pixel, NUMPIXELS );
  Serial.println("green test");
  vTaskDelay( 1000 );

  for ( uint8_t i = 0; i < NUMPIXELS; i++ ){
    pixel[i] = { i, NP_RGB( 0, 0, brightness.ledOutput )}; //blue
  }
  neopixel_SetPixel( neopixel, pixel, NUMPIXELS );
  Serial.println("blue test");
  vTaskDelay( 1000 );

  for ( uint8_t i = 0; i < NUMPIXELS; i++ ){
    pixel[i] = { i, NP_RGB( 0, 0, 0 )};
  }
  neopixel_SetPixel( neopixel, pixel, NUMPIXELS );
  Serial.println("Led off test");
  neopixel_Deinit(neopixel);
  vTaskDelay( 1000 );

  xTaskCreate( lightbarWorker1Hz, "lightbarWorker", 3096, NULL, 3, NULL );
}