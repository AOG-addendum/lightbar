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

#include "main.hpp"
#include "jsonFunctions.hpp"

void lightTest10Hz( void* z );

void getBrightnessLevels(){
  brightness.cdsCell = analogRead( lightbarConfig.gpioCDS ) / 16;
  brightness.potentiometer = analogRead( lightbarConfig.gpioPotentiometer ) / 16;
  brightness.ledOutput = brightness.potentiometer + ( brightness.cdsCell - 128 );
  brightness.ledOutput = constrain( brightness.ledOutput, 0, 255 );
}

void lightbarWorker1Hz( void* z ) {
  constexpr TickType_t xFrequency = 200;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  bool ledState = true;

  for( ;; ) {

    getBrightnessLevels();

    uint8_t centerpixel = ( lightbarConfig.numberOfPixels - 1 ) / 2;
    uint8_t cmPerLBPixel = lightbarConfig.cmPerLightbarPixel / lightbarConfig.cmPerDistInc;
    int8_t level = constrain( (int8_t)( steerSetpoints.requestedSteerAngle / cmPerLBPixel ), -centerpixel, centerpixel);
    int8_t n = level + centerpixel;
    if( lightbarConfig.ledTest == true ){
      tNeopixel pixel[lightbarConfig.numberOfPixels * 2] = { };
      ledState = !ledState;
      for ( uint8_t i = 0; i < ( lightbarConfig.numberOfPixels * 2 ); i++ ){
        if( i == ( lightbarConfig.numberOfPixels - 1 ) ){
          if( ledState ){
            pixel[i] = { i, NP_RGB( 255, 255, 255 )};
          } else pixel[i] = { i, NP_RGB( 0, 0, 0 )};
        } else {
          pixel[i] = { i, NP_RGB( 0, 0, 0 )};
        }
      }
      neopixel_SetPixel( lightbarPixels, pixel, lightbarConfig.numberOfPixels * 2 );
    } else if( steerSetpoints.enabled == false ){
      tNeopixel pixel[lightbarConfig.numberOfPixels] = { };
      for ( uint8_t i = 0; i < lightbarConfig.numberOfPixels; i++ ){
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
      neopixel_SetPixel( lightbarPixels, pixel, lightbarConfig.numberOfPixels );
    } else {
      tNeopixel pixel[lightbarConfig.numberOfPixels] = { };
      for ( uint8_t i = 0; i < lightbarConfig.numberOfPixels; i++ ){
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
      neopixel_SetPixel( lightbarPixels, pixel, lightbarConfig.numberOfPixels );
    }
    vTaskDelay( pdMS_TO_TICKS( 50 ));
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void lightbarRGBTest( void* z ) {
  getBrightnessLevels();
  tNeopixel pixel[lightbarConfig.numberOfPixels * 2] = { };

  for ( uint8_t i = 0; i < lightbarConfig.numberOfPixels; i++ ){
    pixel[i] = { i, NP_RGB( brightness.ledOutput, 0, 0 )}; //red
  }
  neopixel_SetPixel( lightbarPixels, pixel, lightbarConfig.numberOfPixels );
  Serial.println("red test");
  vTaskDelay( 1000 );

  for ( uint8_t i = 0; i < lightbarConfig.numberOfPixels; i++ ){
    pixel[i] = { i, NP_RGB( 0, brightness.ledOutput, 0 )}; // green
  }
  neopixel_SetPixel( lightbarPixels, pixel, lightbarConfig.numberOfPixels );
  Serial.println("green test");
  vTaskDelay( 1000 );

  for ( uint8_t i = 0; i < lightbarConfig.numberOfPixels; i++ ){
    pixel[i] = { i, NP_RGB( 0, 0, brightness.ledOutput )}; //blue
  }
  neopixel_SetPixel( lightbarPixels, pixel, lightbarConfig.numberOfPixels );
  Serial.println("blue test");
  vTaskDelay( 1000 );

  for ( uint8_t i = 0; i < lightbarConfig.numberOfPixels; i++ ){
    pixel[i] = { i, NP_RGB( 0, 0, 0 )};
  }
  neopixel_SetPixel( lightbarPixels, pixel, lightbarConfig.numberOfPixels );
  Serial.println("LED test off");
  vTaskDelay( 1000 );

  xTaskCreate( lightbarWorker1Hz, "lightbarWorker", 3096, NULL, 3, NULL );
  vTaskDelete( NULL );
}

void initLightbar() {

  pinMode(( uint8_t )lightbarConfig.gpioCDS, INPUT );
  pinMode(( uint8_t ) lightbarConfig.gpioPotentiometer, INPUT );
  
  xTaskCreate( lightbarRGBTest, "lightbarRGBTest", 3096, NULL, 3, NULL );
}