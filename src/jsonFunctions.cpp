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

#include <memory>

#include <FS.h>
#include <SPIFFS.h>

#include "main.hpp"
#include "jsonFunctions.hpp"

void loadSavedConfig() {
  {
    auto j = loadJsonFromFile( "/lightbar.json" );
    parseJsonToLightbarConfig( j, lightbarConfig );
  }
}

void saveConfig() {
  {
    const auto j = parseLightbarConfigToJson( lightbarConfig );
    saveJsonToFile( j, "/lightbar.json" );
  }
}

json loadJsonFromFile( const char* fileName ) {
  json j;

  if( SPIFFS.exists( fileName ) ) {
    File file = SPIFFS.open( fileName, "r" );

    if( file ) {
      std::vector<uint8_t> data;
      data.resize( file.size() );

      file.read( data.data(), file.size() );

      try {
        j = json::parse( data/*, nullptr, false*/ );
      } catch( json::exception& e ) {
        // output exception information
        Serial.print( "message: " );
        Serial.println( e.what() );
        Serial.print( "exception id: " );
        Serial.println( e.id );
      }
    } else {
      Serial.print( "Could not open file for reading: " );
      Serial.println( fileName );
      Serial.flush();
    }

    file.close();
  }

  return j;
}

void saveJsonToFile( const json& json, const char* fileName ) {
  // pretty print with 2 spaces indentation
  auto data = json.dump( 2 );

  File file = SPIFFS.open( fileName, "w" );

  if( file && !file.isDirectory() ) {
    file.write( ( uint8_t* )data.c_str(), data.size() );
  } else {
    Serial.print( "Could not open file for writing: " );
    Serial.println( fileName );
    Serial.flush();
  }

  file.close();
}

json parseLightbarConfigToJson( const LightbarConfig& config ) {
  json j;

  j["wifi"]["ssid"] = config.ssid;
  j["wifi"]["password"] = config.password;
  j["wifi"]["hostname"] = config.hostname;
  
  j["workswitch"]["steerSwitchActiveLow"] = config.steerSwitchActiveLow;
  j["workswitch"]["steerSwitchIsMomentary"] = config.steerSwitchIsMomentary;
  
  j["connection"]["baudrate"] = config.baudrate;
  j["connection"]["enableOTA"] = config.enableOTA;

  j["lightbar"]["numberOfPixels"] = config.numberOfPixels;
  j["lightbar"]["cmPerPixel"] = config.cmPerLightbarPixel;
  j["lightbar"]["cmPerDistanceIncrement"] = config.cmPerDistInc;

  j["connection"]["aog"]["sendFrom"] = config.aogPortSendFrom;
  j["connection"]["aog"]["listenTo"] = config.aogPortListenTo;
  j["connection"]["aog"]["sendTo"] = config.aogPortSendTo;

  return j;
}

void parseJsonToLightbarConfig( json& j, LightbarConfig& config ) {
  if( j.is_object() ) {
    try {
      {
        std::string str = j.value( "/wifi/ssid"_json_pointer, lightbarConfigDefaults.ssid );
        memset( config.ssid, 0, sizeof( config.ssid ) );
        memcpy( config.ssid, str.c_str(), str.size() );
      }
      {
        std::string str = j.value( "/wifi/password"_json_pointer, lightbarConfigDefaults.password );
        memset( config.password, 0, sizeof( config.password ) );
        memcpy( config.password, str.c_str(), str.size() );
      }
      {
        std::string str = j.value( "/wifi/hostname"_json_pointer, lightbarConfigDefaults.hostname );
        memset( config.hostname, 0, sizeof( config.hostname ) );
        memcpy( config.hostname, str.c_str(), str.size() );
      }
      
      config.steerSwitchActiveLow = j.value( "/workswitch/steerSwitchActiveLow"_json_pointer, lightbarConfigDefaults.steerSwitchActiveLow );
      config.steerSwitchIsMomentary = j.value( "/workswitch/steerSwitchIsMomentary"_json_pointer, lightbarConfigDefaults.steerSwitchIsMomentary );

      config.baudrate = j.value( "/connection/baudrate"_json_pointer, lightbarConfigDefaults.baudrate );
      config.enableOTA = j.value( "/connection/enableOTA"_json_pointer, lightbarConfigDefaults.enableOTA );

      config.numberOfPixels = j.value( "/lightbar/numberOfPixels"_json_pointer, lightbarConfigDefaults.numberOfPixels );
      config.cmPerLightbarPixel = j.value( "/lightbar/cmPerPixel"_json_pointer, lightbarConfigDefaults.cmPerLightbarPixel );
      config.cmPerDistInc = j.value( "/lightbar/cmPerDistanceIncrement"_json_pointer, lightbarConfigDefaults.cmPerDistInc );

      config.aogPortSendFrom = j.value( "/connection/aog/sendFrom"_json_pointer, lightbarConfigDefaults.aogPortSendFrom );
      config.aogPortListenTo = j.value( "/connection/aog/listenTo"_json_pointer, lightbarConfigDefaults.aogPortListenTo );
      config.aogPortSendTo = j.value( "/connection/aog/sendTo"_json_pointer, lightbarConfigDefaults.aogPortSendTo );

    } catch( json::exception& e ) {
      // output exception information
      Serial.print( "message: " );
      Serial.println( e.what() );
      Serial.print( "exception id: " );
      Serial.println( e.id );
      Serial.flush();
    }
  }
}
