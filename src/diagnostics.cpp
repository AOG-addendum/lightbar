
#include <stdio.h>
#include <string.h>

#include "main.hpp"
#include "jsonFunctions.hpp"

void diagnosticWorker1Hz( void* z ) {
  vTaskDelay( 2000 );
  constexpr TickType_t xFrequency = 1000;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for( ;; ) {

    {
      String str;
      str.reserve( 30 );
      str = "Requested angle : ";
      str += ( int16_t ) steerSetpoints.requestedSteerAngle;
      str += "\nenabled : ";
      str += steerSetpoints.enabled ? "Yes" : "No";
      str += "\n";
      time_t elapsed = millis() - steerSetpoints.lastPacketReceived;
      if( elapsed < 1000 ){
        str += ( time_t )elapsed;
        str += " millis ago";
      } else {
        str += ( time_t )elapsed / 1000;
        str += " seconds ago";
      }
      ESPUI.updateLabel( labelXTE, str );
    }

    {
      String str;
      str.reserve( 30 );
      str = "Potentiometer : ";
      str += ( uint16_t ) brightness.potentiometer;
      str += "\nCDS Cell : ";
      str += ( uint16_t ) brightness.cdsCell;
      str += "\nLed Output : ";
      str += ( uint16_t ) brightness.ledOutput;
      ESPUI.updateLabel( labelBrightness, str );
    }
    {
      String str;
      str.reserve( 30 );
      if( lightbarConfig.steerSwitchIsMomentary == true ){
        str = "Momentary steer switch: ";
      } else {
        str = "Maintained steer switch: ";
      }
      str += ( bool )( digitalRead( lightbarConfig.gpioSteerswitch ) != lightbarConfig.steerSwitchActiveLow ) ? "On " : "Off " ;
      time_t elapsed = millis() - machine.lastAutosteerMillis;
      if( elapsed < 1000 ){
        str += ( time_t )elapsed;
        str += " millis ago";
      } else {
        str += ( time_t )elapsed / 1000;
        str += " seconds ago";
      }
      ESPUI.updateLabel( labelSwitchStates, str );
    }
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}


void initDiagnostics() {
  xTaskCreate( diagnosticWorker1Hz, "diagnosticWorker", 3096, NULL, 3, NULL );
}
