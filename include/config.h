#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

class Config {
public:
  // WiFi Settings
  char wifiSSID[64] = "";
  char wifiPassword[64] = "";
  
  // Station Settings
  char stationCode[4] = "PAD";
  
  // Display Settings
  bool useCallingAt = false;
  bool showStationName = true;  // Show/hide station name to make room for extra service
  int extraServices = 1;  // Number of extra services on bottom line (1-3)
  int refreshInterval = 60;  // Seconds between API calls
  int scrollSpeed = 50;  // Milliseconds for scrolling text
  int rotationSpeed = 15;  // Seconds between service rotations
  
  // Display Positions
  int yPos1st = 26;
  int yPos2nd = 38;
  int yPosAlt = 50;
  
  // Vertical Spacing
  int lineSpacing = 14;  // Default spacing between lines
  
  // Device Info
  String deviceId = "";
  String firmwareVersion = "2.0.0";
  
  // Methods
  bool load() {
    if (!SPIFFS.exists("/config.json")) {
      Serial.println("⚠️ No config file, using defaults");
      return false;
    }
    
    File file = SPIFFS.open("/config.json", "r");
    if (!file) {
      Serial.println("❌ Failed to open config");
      return false;
    }
    
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
      Serial.println("❌ Failed to parse config: " + String(error.c_str()));
      return false;
    }
    
    // Load WiFi
    if (doc.containsKey("wifiSSID")) {
      String ssid = doc["wifiSSID"].as<String>();
      ssid.trim();
      strncpy(wifiSSID, ssid.c_str(), sizeof(wifiSSID) - 1);
      wifiSSID[sizeof(wifiSSID) - 1] = '\0';
    } else {
      wifiSSID[0] = '\0';
    }
    if (doc.containsKey("wifiPassword")) {
      String password = doc["wifiPassword"].as<String>();
      strncpy(wifiPassword, password.c_str(), sizeof(wifiPassword) - 1);
      wifiPassword[sizeof(wifiPassword) - 1] = '\0';
    } else {
      wifiPassword[0] = '\0';
    }

    // Load Station
    if (doc.containsKey("stationCode")) {
      String station = doc["stationCode"].as<String>();
      station.trim();
      strncpy(stationCode, station.c_str(), sizeof(stationCode) - 1);
      stationCode[sizeof(stationCode) - 1] = '\0';
    } else {
      strncpy(stationCode, "PAD", sizeof(stationCode) - 1);
      stationCode[sizeof(stationCode) - 1] = '\0';
    }
    
    // Load Display Settings
    useCallingAt = doc["useCallingAt"] | false;
    showStationName = doc["showStationName"] | true;
    extraServices = doc["extraServices"] | 1;
    refreshInterval = doc["refreshInterval"] | 60;
    scrollSpeed = doc["scrollSpeed"] | 50;
    rotationSpeed = doc["rotationSpeed"] | 15;
    
    // Load Positions
    yPos1st = doc["yPos1st"] | 26;
    yPos2nd = doc["yPos2nd"] | 38;
    yPosAlt = doc["yPosAlt"] | 50;
    
    // Load Vertical Spacing
    lineSpacing = doc["lineSpacing"] | 14;
    
    // Load Device Info
    if (doc.containsKey("deviceId")) {
      deviceId = doc["deviceId"].as<String>();
    }
    if (doc.containsKey("firmwareVersion")) {
      firmwareVersion = doc["firmwareVersion"].as<String>();
    }
    
    Serial.println("✅ Config loaded");
    return true;
  }
  
  bool save() {
    StaticJsonDocument<1024> doc;
    
    // Save WiFi
    doc["wifiSSID"] = wifiSSID;
    doc["wifiPassword"] = wifiPassword;
    
    // Save Station
    doc["stationCode"] = stationCode;
    
    // Save Display Settings
    doc["useCallingAt"] = useCallingAt;
    doc["showStationName"] = showStationName;
    doc["extraServices"] = extraServices;
    doc["refreshInterval"] = refreshInterval;
    doc["scrollSpeed"] = scrollSpeed;
    doc["rotationSpeed"] = rotationSpeed;
    
    // Save Positions
    doc["yPos1st"] = yPos1st;
    doc["yPos2nd"] = yPos2nd;
    doc["yPosAlt"] = yPosAlt;
    
    // Save Vertical Spacing
    doc["lineSpacing"] = lineSpacing;
    
    // Save Device Info
    doc["deviceId"] = deviceId;
    doc["firmwareVersion"] = firmwareVersion;
    
    File file = SPIFFS.open("/config.json", "w");
    if (!file) {
      Serial.println("❌ Failed to open config for writing");
      return false;
    }
    
    if (serializeJson(doc, file) == 0) {
      Serial.println("❌ Failed to write config");
      file.close();
      return false;
    }
    
    file.close();
    Serial.println("✅ Config saved");
    return true;
  }
};

#endif