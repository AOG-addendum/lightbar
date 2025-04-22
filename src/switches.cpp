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

#include "main.hpp"
#include "jsonFunctions.hpp"

#include <string>       // std::string
#include <sstream>      // std::stringstream


AsyncUDP udpSendFrom;

void autosteerSwitchesWorker100Hz( void* z ) {
  constexpr TickType_t xFrequency = 10;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  bool previousState;
  bool switchState;

  switchState = digitalRead( ( uint8_t )lightbarConfig.gpioSteerswitch ); // initialize switch state on startup

  for( ;; ) {
    bool state = digitalRead( ( uint8_t )lightbarConfig.gpioSteerswitch);
    if( state != previousState ){
      machine.lastAutosteerMillis = millis();
      previousState = state;
    }
    if( millis() - machine.lastAutosteerMillis > 50 and switchState != state ){
      switchState = state;
      machine.AogEngagedMismatch = true;
      if( lightbarConfig.steerSwitchIsMomentary ){
        if( switchState == lightbarConfig.steerSwitchActiveLow ){
          machine.steeringEnabled = !machine.steeringEnabled;
        }
      } else {
        if( switchState == lightbarConfig.steerSwitchActiveLow ){
          machine.steeringEnabled = false;
        } else {
          machine.steeringEnabled = true;
        }
      }
    }
    
    uint8_t data[14] = {0};

    data[0] = 0x80; // AOG specific
    data[1] = 0x81; // AOG specific
    data[2] = 0x7F; // autosteer module to AOG
    data[3] = 0xFD; // autosteer module to AOG
    data[4] = 8;    // length of data

    {
      int16_t steerAngle = 0 ;
      data[5] = ( uint16_t )steerAngle;
      data[6] = ( uint16_t )steerAngle >> 8;
    }

    // read inputs
    data[11] |= machine.steeringEnabled ? 0 : 2;

    //data[12] = 0; // PWM ?
    //add the checksum
    int CRCtoAOG = 0;
    for (byte i = 2; i < sizeof(data) - 1; i++)
    {
      CRCtoAOG = (CRCtoAOG + data[i]);
    }
    data[sizeof(data) - 1] = CRCtoAOG;

    udpSendFrom.broadcastTo( data, sizeof( data ), initialisation.portSendTo );

    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void initSwitches() {

  if( lightbarConfig.aogPortSendFrom != 0 ) {
    initialisation.portSendFrom = lightbarConfig.aogPortSendFrom;
  }

  if( lightbarConfig.aogPortSendTo != 0 ) {
    initialisation.portSendTo = lightbarConfig.aogPortSendTo;
  }

  udpSendFrom.listen( initialisation.portSendFrom );

  pinMode( lightbarConfig.gpioSteerswitch, INPUT_PULLUP );

  xTaskCreate( autosteerSwitchesWorker100Hz, "autosteerSwitchesWorker", 3096, NULL, 3, NULL );

}
