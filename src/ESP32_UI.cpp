
#include <stdio.h>

#include <ESPUI.h>

#include "main.hpp"
#include "jsonFunctions.hpp"

uint16_t labelLoad;
uint16_t buttonReset;

uint16_t labelXTE;
uint16_t labelBrightness;
uint16_t labelBuildDate;
uint16_t labelSwitchStates;
uint16_t widgetCmPerLightbarPixel;
uint16_t widgetCmPerDistInc;
char downloadFilename[50];

void setResetButtonToRed() {
  ESPUI.getControl( buttonReset )->color = ControlColor::Alizarin;
  ESPUI.updateControl( buttonReset );
}

void checkLightbarMultiplier(){
  uint8_t multiplier = lightbarConfig.cmPerLightbarPixel / lightbarConfig.cmPerDistInc;
  if( multiplier < 1 ){
    multiplier = 1;
  }
  lightbarConfig.cmPerLightbarPixel = lightbarConfig.cmPerDistInc * multiplier;
  ESPUI.updateNumber( widgetCmPerLightbarPixel, lightbarConfig.cmPerLightbarPixel );
}
  
void initESPUI ( void ) {

  labelLoad = ESPUI.addControl( ControlType::Label, "Load:", "", ControlColor::Turquoise );
  labelXTE = ESPUI.addControl( ControlType::Label, "AOG Message", "0", ControlColor::Peterriver );

  buttonReset = ESPUI.addControl( ControlType::Button, "Store the Settings", "Apply", ControlColor::Emerald, Control::noParent,
  []( Control * control, int id ) {
    if( id == B_UP ) {
      saveConfig();
    }
  } );

  buttonReset = ESPUI.addControl( ControlType::Button, "If this turns red, you have to", "Apply & Reboot", ControlColor::Emerald, Control::noParent,
  []( Control * control, int id ) {
    if( id == B_UP ) {
      saveConfig();
      SPIFFS.end();
      ESP.restart();
    }
  } );

  uint16_t tabConfigurations;
 
  // Status Tab
  {
    uint16_t tab = ESPUI.addControl( ControlType::Tab, "Status", "Status" );

    String buildDate = String(__DATE__);
    buildDate += String(" ");
    buildDate += String(__TIME__);
    labelBuildDate = ESPUI.addControl( ControlType::Label, "Build date", buildDate, ControlColor::Turquoise, tab );

    labelSwitchStates = ESPUI.addControl( ControlType::Label, "Switch states", "Not loaded", ControlColor::Emerald, tab );
    labelBrightness = ESPUI.addControl( ControlType::Label, "Brightness", "0", ControlColor::Turquoise, tab );
  }

  // Network Tab
  {
    uint16_t tab = ESPUI.addControl( ControlType::Tab, "Network", "Network" );

    {
      uint16_t baudrate = ESPUI.addControl( ControlType::Select, "Baudrate Serial", String( lightbarConfig.baudrate ), ControlColor::Peterriver, tab,
      []( Control * control, int id ) {
        uint32_t baudrate = control->value.toInt();
        lightbarConfig.baudrate = baudrate;
      } );
      ESPUI.addControl( ControlType::Option, "4800", "4800", ControlColor::Alizarin, baudrate );
      ESPUI.addControl( ControlType::Option, "9600", "9600", ControlColor::Alizarin, baudrate );
      ESPUI.addControl( ControlType::Option, "19200", "19200", ControlColor::Alizarin, baudrate );
      ESPUI.addControl( ControlType::Option, "38400", "38400", ControlColor::Alizarin, baudrate );
      ESPUI.addControl( ControlType::Option, "57600", "57600", ControlColor::Alizarin, baudrate );
      ESPUI.addControl( ControlType::Option, "115200", "115200", ControlColor::Alizarin, baudrate );
      ESPUI.addControl( ControlType::Option, "230400", "230400", ControlColor::Alizarin, baudrate );
      ESPUI.addControl( ControlType::Option, "460800", "460800", ControlColor::Alizarin, baudrate );
      ESPUI.addControl( ControlType::Option, "921600", "921600", ControlColor::Alizarin, baudrate );
    }

    ESPUI.addControl( ControlType::Text, "SSID*", String( lightbarConfig.ssid ), ControlColor::Wetasphalt, tab,
    []( Control * control, int id ) {
      control->value.toCharArray( lightbarConfig.ssid, sizeof( lightbarConfig.ssid ) );
      setResetButtonToRed();
    } );
    ESPUI.addControl( ControlType::Text, "Password*", String( lightbarConfig.password ), ControlColor::Wetasphalt, tab,
    []( Control * control, int id ) {
      control->value.toCharArray( lightbarConfig.password, sizeof( lightbarConfig.password ) );
      setResetButtonToRed();
    } );
    ESPUI.addControl( ControlType::Text, "Hostname*", String( lightbarConfig.hostname ), ControlColor::Wetasphalt, tab,
    []( Control * control, int id ) {
      control->value.toCharArray( lightbarConfig.hostname, sizeof( lightbarConfig.hostname ) );
      setResetButtonToRed();
    } );

    ESPUI.addControl( ControlType::Switcher, "OTA Enabled*", lightbarConfig.enableOTA ? "1" : "0", ControlColor::Wetasphalt, tab,
    []( Control * control, int id ) {
      lightbarConfig.enableOTA = control->value.toInt() == 1;
      setResetButtonToRed();
    } );

    ESPUI.addControl( ControlType::Number, "Port to send from*", String( lightbarConfig.aogPortSendFrom ), ControlColor::Wetasphalt, tab,
    []( Control * control, int id ) {
      lightbarConfig.aogPortSendFrom = control->value.toInt();
      setResetButtonToRed();
    } );

    ESPUI.addControl( ControlType::Number, "Port to send to*", String( lightbarConfig.aogPortSendTo ), ControlColor::Wetasphalt, tab,
    []( Control * control, int id ) {
      lightbarConfig.aogPortSendTo = control->value.toInt();
      setResetButtonToRed();
    } );
    ESPUI.addControl( ControlType::Number, "Port to listen to*", String( lightbarConfig.aogPortListenTo ), ControlColor::Wetasphalt, tab,
    []( Control * control, int id ) {
      lightbarConfig.aogPortListenTo = control->value.toInt();
      setResetButtonToRed();
    } );
  }

  // Switches/Buttons Tab
  {
    uint16_t tab = ESPUI.addControl( ControlType::Tab, "Steerswitch", "Steerswitch" );

    {
      ESPUI.addControl( ControlType::Switcher, "Autosteer Switch Active Low", lightbarConfig.steerSwitchActiveLow ? "1" : "0", ControlColor::Peterriver, tab,
      []( Control * control, int id ) {
        lightbarConfig.steerSwitchActiveLow = control->value.toInt() == 1;
      } );
    }

    {
      ESPUI.addControl( ControlType::Switcher, "Autosteer Switch is Momentary*", lightbarConfig.steerSwitchIsMomentary ? "1" : "0", ControlColor::Wetasphalt, tab,
      []( Control * control, int id ) {
        lightbarConfig.steerSwitchIsMomentary = control->value.toInt() == 1;
        setResetButtonToRed();
      } );
    }
  }

  // Lightbar Tab
  {
    uint16_t tab = ESPUI.addControl( ControlType::Tab, "Lightbar", "Lightbar" );

    {
      widgetCmPerLightbarPixel = ESPUI.addControl( ControlType::Number, "Cm Per Lightbar Pixel", String( lightbarConfig.cmPerLightbarPixel ), ControlColor::Peterriver, tab,
      []( Control * control, int id ) {
        lightbarConfig.cmPerLightbarPixel = control->value.toInt();
        if( lightbarConfig.cmPerLightbarPixel < lightbarConfig.cmPerDistInc ){
          lightbarConfig.cmPerLightbarPixel = lightbarConfig.cmPerDistInc;
        }
        checkLightbarMultiplier();
      } );
    }
    {
      widgetCmPerDistInc = ESPUI.addControl( ControlType::Number, "Cm Per XTE Increment", String( lightbarConfig.cmPerDistInc ), ControlColor::Peterriver, tab,
      []( Control * control, int id ) {
        lightbarConfig.cmPerDistInc = control->value.toInt();
        if( lightbarConfig.cmPerDistInc < 1 ){
          lightbarConfig.cmPerDistInc = 1;
          ESPUI.updateNumber( widgetCmPerDistInc, lightbarConfig.cmPerDistInc );
        }
        checkLightbarMultiplier();
      } );
    }
  }

  char lightbarDownloadHTML [100];
  sprintf( downloadFilename, "/%s.json", lightbarConfig.hostname );
  sprintf( lightbarDownloadHTML, "<a href='%s'>Configuration</a>", downloadFilename );
  // Default Configurations Tab
  {
    uint16_t tab = ESPUI.addControl( ControlType::Tab, "Configurations", "Configurations" );

    ESPUI.addControl( ControlType::Label, "OTA Update:", "<a href='/update'>Update</a>", ControlColor::Carrot, tab );

    ESPUI.addControl( ControlType::Label, "Download the config:", lightbarDownloadHTML, ControlColor::Carrot, tab );

    ESPUI.addControl( ControlType::Label, "Upload the config:", "<form method='POST' action='/upload-config' enctype='multipart/form-data'><input name='f' type='file'><input type='submit'>ESP32 will restart after submitting</form>", ControlColor::Carrot, tab );
    
    tabConfigurations = tab;

  }
  
  static String title;

  title = "AOG Lightbar :: ";

  title += lightbarConfig.hostname;
  ESPUI.begin( title.c_str() );

  ESPUI.WebServer()->on( downloadFilename, HTTP_GET, []( AsyncWebServerRequest * request ) {

    Serial.print( "Preparing " );
    Serial.print( downloadFilename );
    Serial.println( " for download" );

    char ibuffer[64];
    File f1 = SPIFFS.open( "/lightbar.json", "r" );    //open source file to read
    if ( !f1 ){
      Serial.println( "/lightbar.json not available for copying" );
      return;
    }

    File f2 = SPIFFS.open( downloadFilename, "w" );    //open destination file to write
    if ( !f2 ){
      Serial.print( downloadFilename );
      Serial.println( " could not be created" );
      return;
    }

    uint8_t blocks = 0;
    Serial.print( "Copied " );
    while ( f1.available() > 0 ){
      byte i = f1.readBytes( ibuffer, 64 ); // i = number of bytes placed in buffer from file f1
      f2.write(( uint8_t* )ibuffer, i );    // write i bytes from buffer to file f2
      blocks += 1;
      Serial.print( blocks );
      Serial.print( " " );
    }
    Serial.println( "blocks" );
    
    f2.close();
    f1.close();
    Serial.println( "File creation successful, downloading..." );     //debug
    delay( 5 );
    request->send( SPIFFS, downloadFilename, "application/json", true );
    delay( 5 );
    SPIFFS.remove( downloadFilename );

  } );
  
  // upload a file to /upload-config
  ESPUI.WebServer()->on( "/upload-config", HTTP_POST, []( AsyncWebServerRequest * request ) {
    request->send( 200 );
  }, [tabConfigurations]( AsyncWebServerRequest * request, String filename, size_t index, uint8_t* data, size_t len, bool final ) {
    if( !index ) {
      request->_tempFile = SPIFFS.open( "/lightbar.json", "w" );
    }

    if( request->_tempFile ) {
      if( len ) {
        request->_tempFile.write( data, len );
      }

      if( final ) {
        request->_tempFile.close();
        delay(10);
        ESP.restart();
      }
    }
  } );
}