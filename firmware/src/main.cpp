#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>
#include <time.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>         // ← FIXED: Added missing include
#include <WebSocketsServer.h>
#include <WebSocketsClient.h>
#include <HTTPUpdate.h>
#include "config.h"
#include "web_pages.h"

// Code Quality Improvements - Phase 2
// Define guards before including headers to prevent duplicate function definitions
// Note: We use the header definitions for ServiceData, FetchState, and formatETD
// since other header code (display_functions.h) depends on them
#define DECODE_HTML_ENTITIES_DEFINED
#define EXTRACT_TAG_VALUE_DEFINED
#define GENERATE_DEVICE_ID_DEFINED
// Note: formatETD is NOT guarded - display_functions.h needs it from helpers.h

#include "constants.h"         // Named constants for all magic numbers
#include "types.h"             // Data structures to organize globals
#include "helpers.h"           // Validation and utility functions
#include "display_functions.h" // Display component functions

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

// ---------------- Display ----------------
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R0, 5, 16, 17);

// ---------------- Web Server ----------------
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// ---------------- Configuration ----------------
Config config;

// ---------------- API Settings ----------------
const char* apiHost = "lite.realtime.nationalrail.co.uk";
const char* apiPath = "/OpenLDBWS/ldb9.asmx";
const char* apiToken = "73ee3834-af35-4f22-9b8b-480b70571c39";

// ---------------- Data ----------------
// Note: ServiceData is now defined in types.h
// struct ServiceData {
//   char std[6];
//   char etd[10];
//   char destination[30];
//   char callingPoints[500];
// };

// Phase 2 Step 4: Migrate to DisplayState struct
DisplayState displayState;

// Migrated to displayState:
// - services[] → displayState.services[]
// - serviceCount → displayState.serviceCount
// - stationName → displayState.stationName
// - currentAlternatingService → displayState.currentAlternatingService
// - isAnimating → displayState.isAnimating
// - animationOffset → displayState.animationOffset
// - callingAtScrollOffset → displayState.callingAtScrollOffset
// - lastCallingAtScroll → displayState.lastCallingAtScroll
// - fetchingNewStation → displayState.fetchingNewStation
// - lastRotation → displayState.lastRotation
// - lastDisplaySnapshot → displayState.lastSnapshot

unsigned long lastDataUpdate = 0;  // Not in DisplayState - timing related

// Phase 2 Step 4: Migrate to SystemFlags struct
SystemFlags systemFlags;

// Migrated to systemFlags:
// - apMode → systemFlags.apMode
// - systemError → systemFlags.systemError
// - firstBoot → systemFlags.firstBoot

// Note: FetchState enum is now defined in types.h
// enum FetchState {
//   FETCH_IDLE,
//   FETCH_START,
//   FETCH_WAITING,
//   FETCH_READING,
//   FETCH_DONE,
//   FETCH_FAIL
// };

// Phase 2 Step 4: Migrate to FetchStateData struct
FetchStateData fetchStateData;
WiFiClientSecure fetchClient;  // Keep separate - not in FetchStateData

// Migrated to fetchStateData:
// - fetchState → fetchStateData.state
// - fetchBuffer → fetchStateData.buffer
// - fetchStartTime → fetchStateData.startTime
// - lastFetchAttempt → fetchStateData.lastAttempt
// - lastSuccessfulFetch → fetchStateData.lastSuccess

// Phase 2 Step 4: Migrate to WebSocketClients struct
WebSocketClients wsClients;

// Migrated to wsClients:
// - connectedClients[] → wsClients.clients[]
// - clientCount → wsClients.count
// - lastMetricsBroadcast → wsClients.lastMetricsBroadcast

// Phase 2 Step 4: Migrate to MonitoringState struct
MonitoringState monitoringState;
WebSocketsClient monitorClient;  // Keep separate - not in MonitoringState

// Migrated to monitoringState:
// - monitorServerHost → monitoringState.serverHost
// - monitorServerPort → monitoringState.serverPort
// - monitoringEnabled → monitoringState.enabled
// - monitorConnected → monitoringState.connected
// - lastMonitorHeartbeat → monitoringState.lastHeartbeat
// - lastMonitorDisconnect → monitoringState.lastDisconnect
// - MONITOR_HEARTBEAT_INTERVAL → Timing::MONITOR_HEARTBEAT_INTERVAL (from constants.h)

// ---------------- Logo Bitmap ----------------
const unsigned char logo_bitmap [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xfc, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x07, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 
	0xff, 0xff, 0xff, 0x0f, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x3f, 0x00, 0x00, 
	0xc0, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0x8f, 0x01, 0x0c, 0x00, 0x02, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 
	0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x07, 0x80, 0x01, 
	0x0c, 0x00, 0x03, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x04, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x01, 0x00, 0x03, 0x1f, 0x9f, 0x8f, 0xf0, 0x90, 0x63, 0x18, 
	0x1e, 0x3c, 0x10, 0x3f, 0x1e, 0xe0, 0xe1, 0x01, 0x42, 0x44, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 
	0x00, 0x0f, 0x0c, 0x10, 0x87, 0x98, 0x71, 0xe6, 0x0e, 0x33, 0x60, 0x9e, 0x33, 0x03, 0x30, 0x30, 
	0x03, 0x42, 0x64, 0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x00, 0x3c, 0x0c, 0x30, 0x82, 0x08, 0x33, 
	0xe6, 0x8f, 0x61, 0x60, 0x86, 0x31, 0x03, 0x18, 0x18, 0x06, 0x42, 0x34, 0x00, 0x00, 0x00, 0xf0, 
	0x03, 0x00, 0x00, 0x60, 0x0c, 0x3e, 0x82, 0x0c, 0x33, 0x66, 0x98, 0x61, 0x7c, 0x86, 0x30, 0x0e, 
	0x18, 0x18, 0x06, 0x42, 0x1c, 0x00, 0x00, 0x00, 0xfc, 0x03, 0x00, 0x00, 0x60, 0x8c, 0x31, 0x82, 
	0x0c, 0x33, 0x66, 0x90, 0x61, 0x66, 0x86, 0x30, 0x38, 0x18, 0x18, 0x06, 0x42, 0x3c, 0x00, 0x00, 
	0xff, 0xff, 0xff, 0xff, 0x0f, 0x60, 0x8c, 0x31, 0x82, 0x08, 0x33, 0x66, 0x98, 0x61, 0x42, 0x86, 
	0x31, 0x30, 0x18, 0x18, 0x06, 0x42, 0x34, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x8f, 0x73, 0x8c, 
	0x39, 0x86, 0x98, 0x31, 0xe6, 0x1f, 0x33, 0x66, 0x86, 0x3b, 0xb0, 0x31, 0x30, 0x23, 0x66, 0x64, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x3f, 0x3c, 0x1f, 0x8e, 0xf0, 0x30, 0xe6, 0x0f, 0x1e, 
	0x7e, 0x06, 0x3f, 0x9f, 0xe1, 0xe1, 0x31, 0x7e, 0xc4, 0x00, 0x00, 0x00, 0xfc, 0x01, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x1f, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xfc, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x07, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// ============ Forward Declarations ============
void displaySplashScreen();
void displayProgress(const char* step, int currentStep, int totalSteps, int progress);
void displayWelcomeScreen();
void displayAPScreen();
void displayReadyScreen(const IPAddress& ip);
void displayMessage(const char* line1, const char* line2 = "");
void displayStatus(const char* status);
// Note: extractTagValue, decodeHTMLEntities, formatETD, generateDeviceId
// are defined later in this file - no forward declaration needed
void handleFetchStateMachine();
bool parseAndDisplayResponse(String response);
bool asyncFetchStart();
void setupWebServer();
bool initializeWiFi();
void startAccessPoint();
void initializeTimeSync();
void handleAlternatingService(unsigned long currentTime);
void handleSystemError();
void updateDisplay();
bool checkFirstBoot();
// String generateDeviceId(); // Defined later, no forward declaration needed
void setupOTA();
// String formatETD(String etd); // Defined later, no forward declaration needed
// String fitTextToWidth(...); // Template version in helpers.h handles this
void monitorWebSocketEvent(WStype_t type, uint8_t * payload, size_t length);
void sendMonitorHeartbeat();

// ============ NEW: WebSocket Functions ============

// ============ MONITORING: Remote Monitoring Functions ============

void monitorWebSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("❌ Monitoring server disconnected");
      monitoringState.connected = false;
      monitoringState.lastDisconnect = millis();
      break;
      
    case WStype_CONNECTED:
      {
        Serial.println("✅ Connected to monitoring server");
        monitoringState.connected = true;
        
        // Register device
        String registerMsg = "{";
        registerMsg += "\"type\":\"register\",";
        registerMsg += "\"deviceId\":\"" + config.deviceId + "\",";
        registerMsg += "\"name\":\"Board-" + config.deviceId.substring(0, 8) + "\",";
        registerMsg += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
        registerMsg += "\"firmwareVersion\":\"" + config.firmwareVersion + "\",";
        registerMsg += "\"stationCode\":\"" + String(config.stationCode) + "\",";
        registerMsg += "\"displayState.stationName\":\"" + String(displayState.stationName) + "\",";
        registerMsg += "\"rssi\":" + String(WiFi.RSSI()) + ",";
        registerMsg += "\"uptime\":" + String(millis() / 1000) + ",";
        registerMsg += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
        registerMsg += "\"services\":" + String(displayState.serviceCount);
        registerMsg += "}";
        
        monitorClient.sendTXT(registerMsg);
      }
      break;
      
    case WStype_TEXT:
      {
        Serial.printf("📨 Monitor message: %s\n", payload);
        
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
          const char* msgType = doc["type"];
          
          // Handle server messages
          if (strcmp(msgType, "registered") == 0) {
            Serial.println("✅ Device registration confirmed");
          }
          else if (strcmp(msgType, "heartbeat_ack") == 0) {
            // Heartbeat acknowledged
          }
          else if (strcmp(msgType, "ping") == 0) {
            // Respond to server ping
            String pong = "{\"type\":\"pong\",\"timestamp\":" + String(millis()) + "}";
            monitorClient.sendTXT(pong);
          }
          else if (strcmp(msgType, "command") == 0) {
            // Handle commands from server
            const char* command = doc["command"];
            
            if (strcmp(command, "restart") == 0) {
              Serial.println("🔄 Remote restart requested");
              delay(1000);
              ESP.restart();
            }
            else if (strcmp(command, "updateConfig") == 0) {
              Serial.println("⚙️ Remote config update");
              
              if (doc.containsKey("stationCode")) {
                String station = doc["stationCode"].as<String>();
                station.toUpperCase();
                strncpy(config.stationCode, station.c_str(), sizeof(config.stationCode) - 1);
              }
              if (doc.containsKey("refreshInterval")) {
                config.refreshInterval = doc["refreshInterval"];
              }
              if (doc.containsKey("useCallingAt")) {
                config.useCallingAt = doc["useCallingAt"];
              }
              if (doc.containsKey("showStationName")) {
                config.showStationName = doc["showStationName"];
              }
              if (doc.containsKey("extraServices")) {
                config.extraServices = doc["extraServices"];
              }
              if (doc.containsKey("scrollSpeed")) {
                config.scrollSpeed = doc["scrollSpeed"];
              }
              if (doc.containsKey("rotationSpeed")) {
                config.rotationSpeed = doc["rotationSpeed"];
              }
              
              config.save();
              
              // Reset alternating service
              if (config.useCallingAt) {
                displayState.currentAlternatingService = 1;
              } else if (!config.showStationName) {
                displayState.currentAlternatingService = 3;
              } else {
                displayState.currentAlternatingService = 2;
              }
              
              // Clear data and force refresh
              displayState.serviceCount = 0;
              fetchStateData.lastSuccess = 0;
              fetchStateData.lastAttempt = 0;
            }
            else if (strcmp(command, "otaUpdate") == 0) {
              Serial.println("📦 OTA update from monitoring server");
              const char* url = doc["url"];
              
              displayMessage("OTA Update", "Starting...");
              
              WiFiClientSecure client;
              client.setInsecure();
              
              t_httpUpdate_return ret = httpUpdate.update(client, url);
              
              switch(ret) {
                case HTTP_UPDATE_FAILED:
                  Serial.printf("❌ OTA failed: %s\n", httpUpdate.getLastErrorString().c_str());
                  displayMessage("OTA Failed", httpUpdate.getLastErrorString().c_str());
                  delay(3000);
                  break;
                  
                case HTTP_UPDATE_NO_UPDATES:
                  Serial.println("⚠️ No updates available");
                  displayMessage("No Updates", "");
                  delay(2000);
                  break;
                  
                case HTTP_UPDATE_OK:
                  Serial.println("✅ OTA complete - restarting");
                  displayMessage("Update Complete", "Restarting...");
                  delay(1000);
                  ESP.restart();
                  break;
              }
            }
          }
        }
      }
      break;
      
    case WStype_ERROR:
      Serial.println("❌ Monitor WebSocket error");
      break;
      
    case WStype_PING:
    case WStype_PONG:
      // Ignore ping/pong
      break;
  }
}

void sendMonitorHeartbeat() {
  if (!monitoringState.connected) return;
  
  String heartbeat = "{";
  heartbeat += "\"type\":\"heartbeat\",";
  heartbeat += "\"deviceId\":\"" + config.deviceId + "\",";
  heartbeat += "\"name\":\"Board-" + config.deviceId.substring(0, 8) + "\",";
  heartbeat += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  heartbeat += "\"firmwareVersion\":\"" + config.firmwareVersion + "\",";
  heartbeat += "\"stationCode\":\"" + String(config.stationCode) + "\",";
  heartbeat += "\"displayState.stationName\":\"" + String(displayState.stationName) + "\",";
  heartbeat += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  heartbeat += "\"uptime\":" + String(millis() / 1000) + ",";
  heartbeat += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  heartbeat += "\"services\":" + String(displayState.serviceCount);
  heartbeat += "}";
  
  monitorClient.sendTXT(heartbeat);
}

// ============ END MONITORING Functions ============

void addClient(uint8_t num) {
  if (wsClients.count < 10) {
    wsClients.clients[wsClients.count++] = num;
    Serial.printf("✅ Client #%u added (total: %u)\n", num, wsClients.count);
  }
}

void removeClient(uint8_t num) {
  for (int i = 0; i < wsClients.count; i++) {
    if (wsClients.clients[i] == num) {
      for (int j = i; j < wsClients.count - 1; j++) {
        wsClients.clients[j] = wsClients.clients[j + 1];
      }
      wsClients.count--;
      Serial.printf("❌ Client #%u removed (total: %u)\n", num, wsClients.count);
      break;
    }
  }
}

void broadcastStatus(const char* message, const char* level = "info") {
  if (wsClients.count == 0) return;
  
  String json = "{";
  json += "\"type\":\"status\",";
  json += "\"message\":\"" + String(message) + "\",";
  json += "\"level\":\"" + String(level) + "\",";
  json += "\"timestamp\":" + String(millis());
  json += "}";
  
  webSocket.broadcastTXT(json);
  Serial.println("📡 Broadcast status: " + String(message));
}

void broadcastTrainUpdate() {
  if (wsClients.count == 0) return;
  
  String json = "{";
  json += "\"type\":\"train_update\",";
  json += "\"timestamp\":" + String(millis()) + ",";
  json += "\"station\":\"" + String(displayState.stationName) + "\",";
  json += "\"stationCode\":\"" + String(config.stationCode) + "\",";
  json += "\"services\":" + String(displayState.serviceCount) + ",";
  json += "\"trains\":[";
  
  for (int i = 0; i < displayState.serviceCount && i < 6; i++) {
    if (i > 0) json += ",";
    json += "{";
    json += "\"std\":\"" + String(displayState.services[i].std) + "\",";
    json += "\"etd\":\"" + String(displayState.services[i].etd) + "\",";
    json += "\"destination\":\"" + String(displayState.services[i].destination) + "\"";
    if (config.useCallingAt && i == 0 && strlen(displayState.services[i].callingPoints) > 0) {
      json += ",\"callingAt\":\"" + String(displayState.services[i].callingPoints) + "\"";
    }
    json += "}";
  }
  
  json += "]}";
  
  webSocket.broadcastTXT(json);
  Serial.println("📡 Broadcast train update to " + String(wsClients.count) + " clients");
}

void broadcastMetrics() {
  if (wsClients.count == 0) return;
  
  String json = "{";
  json += "\"type\":\"metrics\",";
  json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"services\":" + String(displayState.serviceCount) + ",";
  json += "\"station\":\"" + String(displayState.stationName) + "\",";
  json += "\"lastUpdate\":" + String(fetchStateData.lastSuccess / 1000);
  json += "}";
  
  webSocket.broadcastTXT(json);
}

void sendCurrentState(uint8_t num) {
  String json = "{";
  json += "\"type\":\"state\",";
  json += "\"station\":\"" + String(config.stationCode) + "\",";
  json += "\"displayState.stationName\":\"" + String(displayState.stationName) + "\",";
  json += "\"services\":" + String(displayState.serviceCount) + ",";
  json += "\"useCallingAt\":" + String(config.useCallingAt ? "true" : "false") + ",";
  json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  json += "\"uptime\":" + String(millis() / 1000);
  json += "}";
  
  webSocket.sendTXT(num, json);
}

void handleWebSocketCommand(uint8_t num, char* payload) {
  String cmd = String(payload);
  
  if (cmd.indexOf("\"command\":\"getState\"") >= 0) {
    sendCurrentState(num);
  }
  else if (cmd.indexOf("\"command\":\"refresh\"") >= 0) {
    if (fetchStateData.state == FETCH_IDLE) {
      fetchStateData.lastSuccess = 0;
      fetchStateData.lastAttempt = 0;
      broadcastStatus("Manual refresh triggered", "info");
    } else {
      broadcastStatus("Fetch already in progress", "warning");
    }
  }
  else if (cmd.indexOf("\"command\":\"restart\"") >= 0) {
    broadcastStatus("Restarting device in 3 seconds...", "warning");
    delay(3000);
    ESP.restart();
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("❌ Client #%u disconnected\n", num);
      removeClient(num);
      break;
      
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("✅ Client #%u connected from %s\n", num, ip.toString().c_str());
      addClient(num);
      sendCurrentState(num);
      break;
    }
    
    case WStype_TEXT:
      handleWebSocketCommand(num, (char*)payload);
      break;
      
    case WStype_ERROR:
      Serial.printf("❌ WebSocket error from #%u\n", num);
      break;
      
    default:
      break;
  }
}

void broadcastDisplaySnapshot() {
  if (wsClients.count == 0) return;
  
  String json = "{";
  json += "\"type\":\"display_snapshot\",";
  json += "\"timestamp\":" + String(millis()) + ",";
  json += "\"displayState.stationName\":\"" + String(displayState.stationName) + "\",";
  json += "\"mode\":\"" + String(config.useCallingAt ? "calling_at" : "normal") + "\",";
  json += "\"displayState.serviceCount\":" + String(displayState.serviceCount) + ",";
  json += "\"services\":[";
  
  // Send up to 3 services for the preview
  for (int i = 0; i < min(displayState.serviceCount, 3); i++) {
    if (i > 0) json += ",";
    json += "{";
    json += "\"std\":\"" + String(displayState.services[i].std) + "\",";
    json += "\"etd\":\"" + String(displayState.services[i].etd) + "\",";
    json += "\"destination\":\"" + String(displayState.services[i].destination) + "\"";
    json += "}";
  }
  
  json += "],";
  
  // Include calling points if in calling at mode
  if (config.useCallingAt && displayState.serviceCount > 0 && strlen(displayState.services[0].callingPoints) > 0) {
    json += "\"callingPoints\":\"" + String(displayState.services[0].callingPoints) + "\",";
  }
  
  // Current time
  time_t now = time(nullptr);
  if (now > 100000) {
    struct tm* timeInfo = localtime(&now);
    char timeString[9];
    strftime(timeString, sizeof(timeString), "%H:%M:%S", timeInfo);
    json += "\"time\":\"" + String(timeString) + "\",";
  }
  
  json += "\"alternatingService\":" + String(displayState.currentAlternatingService) + ",";
  json += "\"displayState.isAnimating\":" + String(displayState.isAnimating ? "true" : "false");
  json += "}";
  
  webSocket.broadcastTXT(json);
}

// Display Functions - keeping the working versions from original
void displaySplashScreen() {
  u8g2.clearBuffer();
  u8g2.drawXBMP(28, 12, 200, 40, logo_bitmap);
  u8g2.sendBuffer();
  delay(3000);
}

void displayProgress(const char* step, int currentStep, int totalSteps, int progress) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_t0_12_tf);
  
  String stepText = "Step " + String(currentStep) + "/" + String(totalSteps);
  u8g2.setCursor(5, 12);
  u8g2.print(stepText);
  
  String percentText = String(progress) + "%";
  int percentWidth = u8g2.getUTF8Width(percentText.c_str());
  u8g2.setCursor(Display::WIDTH -percentWidth - 5, 12);
  u8g2.print(percentText);
  
  u8g2.setCursor(5, 28);
  u8g2.print(step);

  int segments = Display::PROGRESS_SEGMENTS;
  int segmentWidth = Display::PROGRESS_SEGMENT_WIDTH;
  int segmentHeight = Display::PROGRESS_SEGMENT_HEIGHT;
  int spacing = Display::PROGRESS_SPACING;
  int startX = Display::PROGRESS_START_X;
  int startY = Display::PROGRESS_START_Y;
  
  int filledSegments = (progress * segments) / 100;
  
  for (int i = 0; i < segments; i++) {
    int x = startX + (i * (segmentWidth + spacing));
    if (i < filledSegments) {
      u8g2.drawRBox(x, startY, segmentWidth, segmentHeight, 3);
    } else {
      u8g2.drawRFrame(x, startY, segmentWidth, segmentHeight, 3);
    }
  }
  
  u8g2.sendBuffer();
}

void displayWelcomeScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB12_tr);
  
  String welcome = "Welcome!";
  int width = u8g2.getUTF8Width(welcome.c_str());
  u8g2.setCursor((Display::WIDTH -width) / 2, 16);
  u8g2.print(welcome);
  
  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.setCursor(10, 32);
  u8g2.print("First time setup required");
  
  u8g2.setCursor(10, 46);
  u8g2.print("Starting configuration mode...");
  
  u8g2.sendBuffer();
  delay(3000);
}

void displayAPScreen() {
  u8g2.clearBuffer();
  
  // Header bar
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, 0, Display::WIDTH,18);
  u8g2.setDrawColor(0);
  u8g2.setFont(u8g2_font_helvB10_tr);
  const char *title = "SETUP MODE";
  int titleWidth = u8g2.getUTF8Width(title);
  u8g2.setCursor((Display::WIDTH -titleWidth) / 2, 13);
  u8g2.print(title);

  // WiFi icon in header
  u8g2.drawDisc(20, 9, 2, U8G2_DRAW_ALL);
  u8g2.drawCircle(20, 9, 4, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
  u8g2.drawCircle(20, 9, 7, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);

  u8g2.setDrawColor(1);

  // WiFi credentials card
  int cardX = 10;
  int cardY = 24;
  int cardW = 236;
  int cardH = 20;
  u8g2.drawRFrame(cardX, cardY, cardW, cardH, 4);
  
  u8g2.setFont(u8g2_font_t0_11b_tf);
  u8g2.setCursor(cardX + 6, cardY + 9);
  u8g2.print("WiFi");
  
  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.setCursor(cardX + 36, cardY + 9);
  u8g2.print(": TrainBoard_AP");
  
  u8g2.setFont(u8g2_font_t0_11b_tf);
  u8g2.setCursor(cardX + 6, cardY + 18);
  u8g2.print("Password");
  
  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.setCursor(cardX + 64, cardY + 18);
  u8g2.print(": config123");

  // URL card
  cardY = 48;
  cardH = 12;
  u8g2.drawRFrame(cardX, cardY, cardW, cardH, 3);
  
  u8g2.setFont(u8g2_font_t0_11b_tf);
  IPAddress ip = WiFi.softAPIP();
  String url = ip.toString();
  int urlWidth = u8g2.getUTF8Width(url.c_str());
  u8g2.setCursor((Display::WIDTH -urlWidth) / 2, cardY + 9);
  u8g2.print(url);

  u8g2.sendBuffer();
}

void displayMessage(const char* line1, const char* line2) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.setCursor(0, 12);
  u8g2.print(line1);
  if (strlen(line2) > 0) {
    u8g2.setCursor(0, 24);
    u8g2.print(line2);
  }
  u8g2.sendBuffer();
}

void displayStatus(const char* status) {
  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.setDrawColor(0);
  u8g2.drawBox(200, 0, 56, 12);
  u8g2.setDrawColor(1);
  u8g2.setCursor(200, 12);
  u8g2.print(status);
}

void displayReadyScreen(const IPAddress& ip) {
  u8g2.clearBuffer();
  
  // Draw decorative border
  u8g2.drawFrame(0, 0, Display::WIDTH,64);
  u8g2.drawFrame(2, 2, 252, 60);
  
  // Status indicator with checkmark and READY text
  u8g2.setFont(u8g2_font_helvB14_tr);
  u8g2.setCursor(8, 20);
  u8g2.print("✓");
  
  u8g2.setFont(u8g2_font_helvB10_tr);
  u8g2.setCursor(26, 20);
  u8g2.print("READY");
  
  // Separator line
  u8g2.drawHLine(10, 26, 236);
  
  // IP Address - very prominent and centered
  u8g2.setFont(u8g2_font_helvB14_tr);
  String ipStr = ip.toString();
  int ipWidth = u8g2.getUTF8Width(ipStr.c_str());
  u8g2.setCursor((Display::WIDTH -ipWidth) / 2, 45);
  u8g2.print(ipStr);
  
  // Instructions
  u8g2.setFont(u8g2_font_t0_11_tf);
  String instruction = "Open browser to configure";
  int instrWidth = u8g2.getUTF8Width(instruction.c_str());
  u8g2.setCursor((Display::WIDTH -instrWidth) / 2, 59);
  u8g2.print(instruction);
  
  u8g2.sendBuffer();
}

// Utility Functions
String extractTagValue(String xml, String tag, String ns) {
  String openTag = "<" + (ns != "" ? ns + ":" : "") + tag + ">";
  String closeTag = "</" + (ns != "" ? ns + ":" : "") + tag + ">";
  int start = xml.indexOf(openTag);
  if (start == -1) return "";
  start += openTag.length();
  int end = xml.indexOf(closeTag, start);
  if (end == -1) return "";
  return xml.substring(start, end);
}

String decodeHTMLEntities(String text) {
  text.replace("&amp;", "&");
  text.replace("&lt;", "<");
  text.replace("&gt;", ">");
  text.replace("&quot;", "\"");
  text.replace("&#39;", "'");
  text.replace("&apos;", "'");
  return text;
}

String generateDeviceId() {
  uint64_t chipid = ESP.getEfuseMac();
  char id[17];
  sprintf(id, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
  return String(id);
}

bool checkFirstBoot() {
  if (!SPIFFS.exists("/config.json")) {
    Serial.println("🆕 First boot detected");
    return true;
  }
  
  if (SPIFFS.exists("/firstboot.flag")) {
    Serial.println("🆕 First boot flag found");
    SPIFFS.remove("/firstboot.flag");
    return true;
  }
  
  return false;
}

// Note: formatETD is now defined in helpers.h
// String formatETD(String etd) {
//   if (etd.length() == 5 && etd.indexOf(":") != -1) {
//     return "Exp " + etd;
//   }
//   return etd;
// }

String fitTextToWidth(String text, int maxWidth) {
  if (u8g2.getUTF8Width(text.c_str()) <= maxWidth) {
    return text;
  }
  
  // Account for the "." we'll add - single period instead of "..." for more space
  int ellipsisWidth = u8g2.getUTF8Width(".");
  
  while (u8g2.getUTF8Width(text.c_str()) > maxWidth - ellipsisWidth && text.length() > 0) {
    text.remove(text.length() - 1);
  }
  
  if (text.length() > 0) {
    text += ".";
  }
  
  return text;
}

// WiFi Functions
bool initializeWiFi() {
  if (strlen(config.wifiSSID) == 0) {
    Serial.println("⚠️ WiFi SSID not configured");
    return false;
  }

  Serial.println("📡 Connecting to WiFi: " + String(config.wifiSSID));

  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  delay(100);

  WiFi.begin(config.wifiSSID, config.wifiPassword);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 100) {
    delay(300);
    Serial.print(".");
    attempts++;
    if (attempts % 4 == 0) {
      int progress = (attempts * 100) / 30;
      displayProgress("Connecting to WiFi...", 2, 5, progress);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected! IP: " + WiFi.localIP().toString());
    systemFlags.apMode = false;
    return true;
  } else {
    Serial.println("\n❌ WiFi Connection Failed");
    return false;
  }
  // ← FIXED: Removed unreachable "return connected;"
}

void startAccessPoint() {
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  delay(100);
  WiFi.softAP("TrainBoard_AP", "config123");
  IPAddress ip = WiFi.softAPIP();
  Serial.println("🔧 AP Mode Started");
  Serial.println("SSID: TrainBoard_AP");
  Serial.println("Pass: config123");
  Serial.println("IP: " + ip.toString());
  systemFlags.apMode = true;
  setupWebServer();
}

void initializeTimeSync() {
  Serial.println("🕒 Syncing time with NTP...");
  configTime(0, 0, Data::NTP_SERVER);
  unsigned long startTime = millis();
  while (!time(nullptr) && millis() - startTime < Net::NTP_SYNC_TIMEOUT) {
    delay(500);
    Serial.print(".");
  }
  if (time(nullptr)) Serial.println("\n✅ Time synchronized");
  else Serial.println("\n❌ Time sync failed");
}

void setupOTA() {
  ArduinoOTA.setHostname("trainboard");
  ArduinoOTA.setPassword("trainboard2024");
  
  ArduinoOTA.onStart([]() {
    displayMessage("OTA Update", "Starting...");
  });
  
  ArduinoOTA.onEnd([]() {
    displayMessage("Update Complete!", "Restarting...");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int percent = (progress * 100) / total;
    displayProgress("OTA Update", 1, 1, percent);
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    const char* errorMsg;
    if (error == OTA_AUTH_ERROR) errorMsg = "Auth Failed";
    else if (error == OTA_BEGIN_ERROR) errorMsg = "Begin Failed";
    else if (error == OTA_CONNECT_ERROR) errorMsg = "Connect Failed";
    else if (error == OTA_RECEIVE_ERROR) errorMsg = "Receive Failed";
    else if (error == OTA_END_ERROR) errorMsg = "End Failed";
    else errorMsg = "Unknown Error";
    
    displayMessage("OTA Failed", errorMsg);
    delay(3000);
  });
  
  ArduinoOTA.begin();
  Serial.println("✅ OTA Ready");
}

// Data Fetching - using the WORKING logic from original
bool asyncFetchStart() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi not connected");
    displayStatus("OFF");
    return false;
  }

  Serial.println("📡 Fetching: " + String(config.stationCode));
  
  // Update display before blocking operations
  unsigned long currentTime = millis();
  handleAlternatingService(currentTime);
  updateDisplay();

  // Connect with periodic display updates
  Serial.println("🔌 Connecting to API...");
  unsigned long connectStart = millis();
  
  // Non-blocking connect attempt with display updates
  if (!fetchClient.connect(apiHost, 443)) {
    Serial.println("❌ API connection failed");
    displayStatus("FAIL");
    return false;
  }
  
  Serial.println("✅ Connected (" + String(millis() - connectStart) + "ms)");
  
  // Update display after connection
  currentTime = millis();
  handleAlternatingService(currentTime);
  updateDisplay();

  broadcastStatus("Fetching train data...", "info");

  // Always fetch maximum services so we have data available when settings change
  // Maximum useful services: 1 or 2 base + 4 extra = 5 or 6 total  
  // Fetch 8 to have buffer for any configuration
  int numRows = 8;
  
  // Build SOAP request more efficiently
  String soapRequest;
  soapRequest.reserve(512);  // Pre-allocate to avoid reallocations
  soapRequest = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
  soapRequest += "<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\">";
  soapRequest += "<soap:Header><AccessToken xmlns=\"http://thalesgroup.com/RTTI/2013-11-28/Token/types\">";
  soapRequest += "<TokenValue>" + String(apiToken) + "</TokenValue></AccessToken></soap:Header>";
  
  if (config.useCallingAt) {
    soapRequest += "<soap:Body><GetDepBoardWithDetailsRequest xmlns=\"http://thalesgroup.com/RTTI/2016-02-16/ldb/\">";
    soapRequest += "<numRows>" + String(numRows) + "</numRows><crs>" + String(config.stationCode) + "</crs>";
    soapRequest += "</GetDepBoardWithDetailsRequest></soap:Body></soap:Envelope>";
  } else {
    soapRequest += "<soap:Body><GetDepartureBoardRequest xmlns=\"http://thalesgroup.com/RTTI/2016-02-16/ldb/\">";
    soapRequest += "<numRows>" + String(numRows) + "</numRows><crs>" + String(config.stationCode) + "</crs>";
    soapRequest += "</GetDepartureBoardRequest></soap:Body></soap:Envelope>";
  }
  
  // Use Connection: close - API server doesn't support keep-alive properly
  fetchClient.print("POST " + String(apiPath) + " HTTP/1.1\r\n"
                    "Host: " + String(apiHost) + "\r\n"
                    "Content-Type: text/xml\r\n"
                    "Content-Length: " + String(soapRequest.length()) + "\r\n"
                    "Connection: close\r\n\r\n" + soapRequest);

  fetchStateData.startTime = millis();
  fetchStateData.buffer = "";
  fetchStateData.state = FETCH_WAITING;
  displayStatus("Fetch");
  
  // One more display update after sending request
  currentTime = millis();
  handleAlternatingService(currentTime);
  updateDisplay();
  
  return true;
}

void handleFetchStateMachine() {
  switch (fetchStateData.state) {
    case FETCH_WAITING:
      if (fetchClient.connected() || fetchClient.available()) {
        fetchStateData.state = FETCH_READING;
      } else if (millis() - fetchStateData.startTime > 8000) {
        fetchClient.stop();
        Serial.println("❌ Timeout WAITING");
        displayStatus("TIMEOUT");
        fetchStateData.state = FETCH_FAIL;
      }
      break;

    case FETCH_READING:
      {
        // Track when we last updated display
        static unsigned long lastDisplayUpdate = 0;
        unsigned long currentTime = millis();
        
        // Read in chunks for much better performance (10-20x faster than char-by-char)
        while (fetchClient.available()) {
          // Read up to 512 bytes at a time
          uint8_t buffer[512];
          int bytesRead = fetchClient.read(buffer, sizeof(buffer));
          if (bytesRead > 0) {
            fetchStateData.buffer.concat((const char*)buffer, bytesRead);
          }
          
          // Update display every 200ms while reading to keep clock and animations alive
          currentTime = millis();
          if (currentTime - lastDisplayUpdate > 200) {
            handleAlternatingService(currentTime);
            updateDisplay();
            lastDisplayUpdate = currentTime;
          }
        }
        
        // Check if done - connection closed and no more data
        if (!fetchClient.connected() && !fetchClient.available()) {
          fetchClient.stop();  // Ensure clean disconnect
          if (fetchStateData.buffer.length() > 100) {  // Valid response is always >100 bytes
            Serial.println("✅ Fetched: " + String(fetchStateData.buffer.length()) + " bytes in " + String(millis() - fetchStateData.startTime) + "ms");
            fetchStateData.state = FETCH_DONE;
          } else {
            Serial.println("❌ Invalid response size: " + String(fetchStateData.buffer.length()) + " bytes");
            fetchStateData.state = FETCH_FAIL;
          }
        }
        
        // Timeout for reading - API server can be slow with large responses
        if (millis() - fetchStateData.startTime > 15000) {
          fetchClient.stop();
          if (fetchStateData.buffer.length() > 100) {
            // Got data but took too long - still use it
            Serial.println("⚠️ Slow fetch (" + String(fetchStateData.buffer.length()) + " bytes) - using anyway");
            fetchStateData.state = FETCH_DONE;
          } else {
            Serial.println("❌ Timeout READING");
            displayStatus("TIMEOUT");
            fetchStateData.state = FETCH_FAIL;
          }
        }
      }
      break;

    case FETCH_DONE:
      if (parseAndDisplayResponse(fetchStateData.buffer)) {
        displayStatus("OK");
        fetchStateData.lastSuccess = millis();  
        lastDataUpdate = millis();
        Serial.println("✅ Parse successful");
      } else {
        displayStatus("ERR");
        Serial.println("❌ Parse failed");
      }
      fetchStateData.buffer = "";
      fetchStateData.state = FETCH_IDLE;
      break;

    case FETCH_FAIL:
      displayStatus("FAIL");
      fetchClient.stop();
      fetchStateData.buffer = "";
      fetchStateData.state = FETCH_IDLE;
      lastDataUpdate = millis();
      broadcastStatus("Failed to fetch train data", "error");
      break;

    default:
      break;
  }
}

// Parse function - using the WORKING logic from original
bool parseAndDisplayResponse(String response) {
  // Use temporary counter during parsing to avoid showing partial data
  int newServiceCount = 0;

  Serial.println("📊 Processing (" + String(response.length()) + " bytes)");
  
  // Check if response seems too short
  if (response.length() < 500) {
    Serial.println("⚠️ Response seems unusually short!");
    Serial.println("🔍 Full response:");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.println(response);
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  }
  
  // DEBUG: Uncomment the next 3 lines to see the full response
  // Serial.println("🔍 Full response:");
  // Serial.println(response);
  // Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  if (response.indexOf("soap:Fault") != -1) {
    Serial.println("❌ SOAP Fault detected");
    String faultMsg = extractTagValue(response, "faultstring", "");
    if (faultMsg.length() > 0) {
      Serial.println("   Message: " + faultMsg);
    }
    return false;
  }

  String station = extractTagValue(response, "locationName", "lt4");
  if (station == "") station = extractTagValue(response, "locationName", "lt5");
  
  if (station.length() > 0) {
    station.toCharArray(displayState.stationName, sizeof(displayState.stationName));
    Serial.println("📍 " + station);
  }

  int servicesStart = response.indexOf("<lt5:trainServices>");
  if (servicesStart == -1) servicesStart = response.indexOf("<lt4:trainServices>");
  
  if (servicesStart == -1) {
    Serial.println("❌ No services tag found");
    Serial.println("🔍 Debug: Showing first 1000 chars of response:");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.println(response.substring(0, min(1000, (int)response.length())));
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    // Check for specific error messages
    if (response.indexOf("<faultstring>") != -1) {
      String faultMsg = extractTagValue(response, "faultstring", "");
      Serial.println("❌ SOAP Fault: " + faultMsg);
    }
    if (response.indexOf("nrcc:") != -1) {
      Serial.println("⚠️ Response contains nrcc namespace - message field present");
    }
    
    return false;
  }

  int servicesEnd = response.indexOf("</lt5:trainServices>", servicesStart);
  if (servicesEnd == -1) servicesEnd = response.indexOf("</lt4:trainServices>", servicesStart);
  if (servicesEnd == -1) servicesEnd = response.length();

  String trainServices = response.substring(servicesStart, servicesEnd);
  
  String serviceTag = "<lt5:service>";
  String serviceEndTag = "</lt5:service>";
  
  int pos = 0;
  // Always parse maximum services (up to 8) so data is available when settings change
  int maxServices = 8;

  while (newServiceCount < maxServices) {
    int serviceStart = trainServices.indexOf(serviceTag, pos);
    if (serviceStart == -1) break;
    
    int serviceEnd = trainServices.indexOf(serviceEndTag, serviceStart);
    if (serviceEnd == -1) {
      int nextServiceStart = trainServices.indexOf(serviceTag, serviceStart + 1);
      if (nextServiceStart != -1) {
        serviceEnd = nextServiceStart;
      } else {
        serviceEnd = trainServices.length();
      }
    } else {
      serviceEnd += serviceEndTag.length();
    }

    String block = trainServices.substring(serviceStart, serviceEnd);

    String std = extractTagValue(block, "std", "lt4");
    if (std == "") std = extractTagValue(block, "std", "lt5");

    String etd = extractTagValue(block, "etd", "lt4");
    if (etd == "") etd = extractTagValue(block, "etd", "lt5");

    String destBlock = extractTagValue(block, "destination", "lt5");
    if (destBlock == "") destBlock = extractTagValue(block, "destination", "lt4");

    String destination = extractTagValue(destBlock, "locationName", "lt4");
    if (destination == "") destination = extractTagValue(destBlock, "locationName", "lt5");
    destination = decodeHTMLEntities(destination);

    if (std != "" && destination != "") {
      std.toCharArray(displayState.services[newServiceCount].std, sizeof(displayState.services[newServiceCount].std));
      etd.toCharArray(displayState.services[newServiceCount].etd, sizeof(displayState.services[newServiceCount].etd));
      destination.toCharArray(displayState.services[newServiceCount].destination, sizeof(displayState.services[newServiceCount].destination));
      displayState.services[newServiceCount].callingPoints[0] = '\0';
      
      Serial.println("🚂 " + String(newServiceCount + 1) + ": " + std + " → " + destination);
      
      if (config.useCallingAt && newServiceCount == 0) {
        int cpListIdx = block.indexOf("<lt5:subsequentCallingPoints>");
        if (cpListIdx == -1) cpListIdx = block.indexOf("<lt4:subsequentCallingPoints>");
        
        if (cpListIdx != -1) {
          int cpListEndIdx = block.indexOf("</lt5:subsequentCallingPoints>", cpListIdx);
          if (cpListEndIdx == -1) cpListEndIdx = block.indexOf("</lt4:subsequentCallingPoints>", cpListIdx);
          
          if (cpListEndIdx != -1) {
            String cpSection = block.substring(cpListIdx, cpListEndIdx);
            
            int cpListStart = cpSection.indexOf("<lt4:callingPointList>");
            if (cpListStart == -1) cpListStart = cpSection.indexOf("<lt5:callingPointList>");
            
            if (cpListStart != -1) {
              int cpListEnd = cpSection.indexOf("</lt4:callingPointList>", cpListStart);
              if (cpListEnd == -1) cpListEnd = cpSection.indexOf("</lt5:callingPointList>", cpListStart);
              
              if (cpListEnd != -1) {
                String cpList = cpSection.substring(cpListStart, cpListEnd);
                
                String cpTag = "<lt4:callingPoint>";
                String cpEndTag = "</lt4:callingPoint>";
                
                if (cpList.indexOf(cpTag) == -1) {
                  cpTag = "<lt5:callingPoint>";
                  cpEndTag = "</lt5:callingPoint>";
                }
                
                String callingPoints = "";
                int cpPos = 0;
                
                while ((cpPos = cpList.indexOf(cpTag, cpPos)) != -1) {
                  int cpEnd = cpList.indexOf(cpEndTag, cpPos);
                  if (cpEnd == -1) break;
                  
                  String cpBlock = cpList.substring(cpPos, cpEnd);
                  
                  String cpName = extractTagValue(cpBlock, "locationName", "lt4");
                  if (cpName == "") cpName = extractTagValue(cpBlock, "locationName", "lt5");
                  cpName = decodeHTMLEntities(cpName);
                  
                  String cpTime = extractTagValue(cpBlock, "st", "lt4");
                  if (cpTime == "") cpTime = extractTagValue(cpBlock, "st", "lt5");
                  
                  if (cpName != "") {
                    if (callingPoints != "") callingPoints += ", ";
                    callingPoints += cpName;
                    if (cpTime != "") callingPoints += " (" + cpTime + ")";
                  }
                  
                  cpPos = cpEnd;
                  
                  // Allow other tasks to run during long calling points lists
                  yield();
                }
                
                if (callingPoints != "" && callingPoints.length() < 500) {
                  callingPoints.toCharArray(displayState.services[newServiceCount].callingPoints, 500);
                  Serial.println("  ✅ Stored calling points");
                } else if (callingPoints == "") {
                  String fallback = "No further stops available";
                  fallback.toCharArray(displayState.services[newServiceCount].callingPoints, 500);
                }
              }
            }
          }
        } else {
          String fallback = "No further stops available";
          fallback.toCharArray(displayState.services[newServiceCount].callingPoints, 500);
        }
      }
      
      newServiceCount++;
      
      // Keep display alive during parsing - update clock and animations with OLD data
      // This prevents the display from freezing during long parsing operations
      unsigned long currentTime = millis();
      handleAlternatingService(currentTime);
      updateDisplay();
      yield();
    }
    
    pos = serviceEnd;
  }

  Serial.println("✅ " + String(newServiceCount) + " services");

  // Only update displayState.serviceCount after parsing is complete
  // This ensures old data stays visible during parsing
  displayState.serviceCount = newServiceCount;

  if (displayState.serviceCount > 0) {
    displayState.fetchingNewStation = false;  // Clear loading flag - we have data now
    broadcastTrainUpdate();  // Sends full train data to clients
    broadcastStatus("Train data updated", "success");
  }

  return displayState.serviceCount > 0;
}

// Animation - using WORKING logic from original
void handleAlternatingService(unsigned long currentTime) {
  // Don't animate if no extra services configured
  if (config.extraServices == 0) {
    return;
  }

  int textHeight = u8g2.getAscent() - u8g2.getDescent();
  const int scrollStep = 2;
  const int maxOffset = textHeight;

  // When station name is hidden, we shift all service indices by 1
  int serviceOffset = config.showStationName ? 0 : 1;
  
  int minServicesForAlt, maxServiceIndex, startIndex;
  if (config.useCallingAt) {
    // Calling At mode: need at least 2 services on bottom line to rotate
    minServicesForAlt = config.extraServices + 2 + serviceOffset;
    maxServiceIndex = config.extraServices + serviceOffset;
    startIndex = 1 + serviceOffset;
  } else {
    // Standard mode: need at least 2 services on bottom line to rotate
    minServicesForAlt = config.extraServices + 3 + serviceOffset;
    maxServiceIndex = config.extraServices + 1 + serviceOffset;
    startIndex = 2 + serviceOffset;
  }
  
  unsigned long rotationInterval = config.rotationSpeed * 1000UL;  // Use config setting
  
  if (currentTime - displayState.lastRotation >= rotationInterval && displayState.serviceCount >= minServicesForAlt && !displayState.isAnimating) {
    displayState.isAnimating = true;
    displayState.animationOffset = 0;
    displayState.lastRotation = currentTime;
  }

  if (displayState.isAnimating) {
    displayState.animationOffset += scrollStep;
    if (displayState.animationOffset >= maxOffset) {
      displayState.isAnimating = false;
      
      displayState.currentAlternatingService++;
      int maxIndex = (config.useCallingAt ? config.extraServices + 1 : config.extraServices + 2) + serviceOffset;
      if (displayState.currentAlternatingService > maxIndex) {
        displayState.currentAlternatingService = startIndex;
      }
      
      displayState.animationOffset = 0;
    }
  }
}

void handleSystemError() {
  static unsigned long lastBlink = 0;
  static bool showMessage = true;
  if (millis() - lastBlink > 2000) {
    showMessage = !showMessage;
    lastBlink = millis();
    if (showMessage) displayMessage("System Error", "Check Serial");
    else displayMessage("Restart Device", "Or Reconfigure");
  }
  server.handleClient();
}

// Display Update - using WORKING logic with continuous updates
// Note: We always parse up to 6 services, so changing display settings
// (calling at mode, extra services count) works immediately without re-fetching
void updateDisplay() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_t0_11_tf);

  // 1. Display station name OR first service at top
  if (config.showStationName) {
    displayStationName(displayState.stationName);
  } else if (displayState.serviceCount > 0) {
    displayServiceLine(displayState.services[0], "1st ", config.yPosTop, u8g2);
  }

  // 2. Handle no services case
  if (displayState.serviceCount == 0) {
    displayNoServicesMessage(displayState.fetchingNewStation);
  }
  // 3. Handle calling at mode
  else if (config.useCallingAt && displayState.serviceCount > 0) {
    int serviceOffset = getServiceOffset(config.showStationName);

    // Show first service if station name is visible
    if (config.showStationName) {
      displayServiceLine(displayState.services[0], "1st ", config.yPos1st, u8g2);
      // Show calling points at yPos2nd
      displayCallingPoints(displayState.services[0].callingPoints, config.yPos2nd,
                          displayState.callingAtScrollOffset, displayState.lastCallingAtScroll);
    } else {
      // Station name hidden: first service already shown at top
      // Show calling points at yPos1st
      displayCallingPoints(displayState.services[0].callingPoints, config.yPos1st,
                          displayState.callingAtScrollOffset, displayState.lastCallingAtScroll);
      // Show second service at yPos2nd if available
      if (displayState.serviceCount > 1) {
        displayServiceLine(displayState.services[1], "2nd ", config.yPos2nd, u8g2);
      }
    }

    // Display alternating services on bottom line if enough services
    int minServices = getMinServicesForAlternating(config.useCallingAt, config.extraServices, config.showStationName);
    int startIndex = getAlternatingStartIndex(config.useCallingAt, config.showStationName);
    int maxIndex = getAlternatingMaxIndex(config.useCallingAt, config.extraServices, config.showStationName);

    if (displayState.serviceCount >= 2 + serviceOffset) {
      int indexA = displayState.currentAlternatingService;
      int indexB = (indexA + 1 > maxIndex) ? startIndex : indexA + 1;

      String labelA = getServiceLabel(indexA, serviceOffset);
      String labelB = getServiceLabel(indexB, serviceOffset);

      displayAlternatingServices(displayState.services[indexA], displayState.services[indexB],
                                labelA.c_str(), labelB.c_str(), config.yPosAlt,
                                displayState.animationOffset,
                                displayState.isAnimating && displayState.serviceCount >= minServices);
    }
  }
  // 4. Handle standard mode (no calling at)
  else {
    int serviceOffset = getServiceOffset(config.showStationName);

    // Display first 2 services in fixed positions
    for (int i = 0; i < min(displayState.serviceCount - serviceOffset, 2); i++) {
      int serviceIdx = i + serviceOffset;
      if (serviceIdx >= displayState.serviceCount) break;

      int yPos = (i == 0) ? config.yPos1st : config.yPos2nd;
      String label = getServiceLabel(i, serviceOffset);

      displayServiceLine(displayState.services[serviceIdx], label.c_str(), yPos, u8g2);
    }

    // Display alternating services on bottom line if enough services
    int minServices = getMinServicesForAlternating(config.useCallingAt, config.extraServices, config.showStationName);
    int startIndex = getAlternatingStartIndex(config.useCallingAt, config.showStationName);
    int maxIndex = getAlternatingMaxIndex(config.useCallingAt, config.extraServices, config.showStationName);

    if (displayState.serviceCount >= 3 + serviceOffset) {
      int indexA = displayState.currentAlternatingService;
      int indexB = (indexA + 1 > maxIndex) ? startIndex : indexA + 1;

      String labelA = getServiceLabel(indexA, serviceOffset);
      String labelB = getServiceLabel(indexB, serviceOffset);

      displayAlternatingServices(displayState.services[indexA], displayState.services[indexB],
                                labelA.c_str(), labelB.c_str(), config.yPosAlt,
                                displayState.animationOffset,
                                displayState.isAnimating && displayState.serviceCount >= minServices);
    }
  }

  // 5. Display clock at bottom
  displayClock();

  u8g2.sendBuffer();
}


void setupWebServer() {
  server.on("/", HTTP_GET, []() {
    String html = FPSTR(CONFIG_PAGE_TEMPLATE);
    
    html.replace("{SSID}", String(config.wifiSSID));
    html.replace("{STATION}", String(config.stationCode));
    html.replace("{STATION_NAME}", String(displayState.stationName));
    html.replace("{INTERVAL}", String(config.refreshInterval));
    html.replace("{MODE_SEL_0}", config.useCallingAt ? "" : " selected");
    html.replace("{MODE_SEL_1}", config.useCallingAt ? " selected" : "");
    html.replace("{SHOWSTATION_SEL_1}", config.showStationName ? " selected" : "");
    html.replace("{SHOWSTATION_SEL_0}", !config.showStationName ? " selected" : "");
    html.replace("{EXTRA_SEL_0}", config.extraServices == 0 ? " selected" : "");
    html.replace("{EXTRA_SEL_1}", config.extraServices == 1 ? " selected" : "");
    html.replace("{EXTRA_SEL_2}", config.extraServices == 2 ? " selected" : "");
    html.replace("{EXTRA_SEL_3}", config.extraServices == 3 ? " selected" : "");
    html.replace("{EXTRA_SEL_4}", config.extraServices == 4 ? " selected" : "");
    html.replace("{SCROLL}", String(config.scrollSpeed));
    html.replace("{ROTATION}", String(config.rotationSpeed));
    html.replace("{YTOP}", String(config.yPosTop));
    html.replace("{Y1}", String(config.yPos1st));
    html.replace("{Y2}", String(config.yPos2nd));
    html.replace("{Y3}", String(config.yPosAlt));
    html.replace("{IP}", WiFi.localIP().toString());
    html.replace("{DEVICE_ID}", config.deviceId);
    
    server.send(200, "text/html", html);
  });

  server.on("/save", HTTP_POST, []() {
    Serial.println("📝 /save endpoint called");

    // Validate SSID
    if (server.hasArg("ssid")) {
      String ssid = server.arg("ssid");
      Serial.printf("  Validating SSID: '%s'\n", ssid.c_str());
      ValidationResult result = validateSSID(ssid);
      if (!result.valid) {
        Serial.printf("  ❌ SSID validation failed: %s\n", result.message.c_str());
        server.send(400, "text/plain", "Invalid SSID: " + result.message);
        return;
      }
      Serial.println("  ✅ SSID valid");
      safeStrCopy(config.wifiSSID, ssid, sizeof(config.wifiSSID));
    }

    // Validate password
    if (server.hasArg("password") && !server.arg("password").isEmpty()) {
      String password = server.arg("password");
      Serial.println("  Validating password (hidden)");
      ValidationResult result = validatePassword(password);
      if (!result.valid) {
        Serial.printf("  ❌ Password validation failed: %s\n", result.message.c_str());
        server.send(400, "text/plain", "Invalid password: " + result.message);
        return;
      }
      Serial.println("  ✅ Password valid");
      safeStrCopy(config.wifiPassword, password, sizeof(config.wifiPassword));
    }

    // Validate station code
    if (server.hasArg("station")) {
      String station = sanitizeStationCode(server.arg("station"));
      Serial.printf("  Validating station code: '%s'\n", station.c_str());
      ValidationResult result = validateStationCode(station);
      if (!result.valid) {
        Serial.printf("  ❌ Station code validation failed: %s\n", result.message.c_str());
        server.send(400, "text/plain", "Invalid station code: " + result.message);
        return;
      }
      Serial.println("  ✅ Station code valid");
      safeStrCopy(config.stationCode, station, sizeof(config.stationCode));
    }

    // Validate refresh interval
    if (server.hasArg("interval")) {
      int interval = server.arg("interval").toInt();
      ValidationResult result = validateRange(interval, Data::MIN_REFRESH_INTERVAL, Data::MAX_REFRESH_INTERVAL, "Refresh interval");
      if (!result.valid) {
        server.send(400, "text/plain", result.message);
        return;
      }
      config.refreshInterval = interval;
    }

    if (server.hasArg("mode")) config.useCallingAt = (server.arg("mode") == "1");
    if (server.hasArg("showstation")) config.showStationName = (server.arg("showstation") == "1");

    // Validate extra services
    if (server.hasArg("extra")) {
      int extra = server.arg("extra").toInt();
      config.extraServices = constrainToRange(extra, 0, Data::MAX_EXTRA_SERVICES);
    }

    // Validate scroll speed
    if (server.hasArg("scrollspeed")) {
      int speed = server.arg("scrollspeed").toInt();
      ValidationResult result = validateRange(speed, Data::MIN_SCROLL_SPEED, Data::MAX_SCROLL_SPEED, "Scroll speed");
      if (!result.valid) {
        server.send(400, "text/plain", result.message);
        return;
      }
      config.scrollSpeed = speed;
    }

    // Validate rotation speed
    if (server.hasArg("rotationspeed")) {
      int speed = server.arg("rotationspeed").toInt();
      ValidationResult result = validateRange(speed, Data::MIN_ROTATION_SPEED, Data::MAX_ROTATION_SPEED, "Rotation speed");
      if (!result.valid) {
        server.send(400, "text/plain", result.message);
        return;
      }
      config.rotationSpeed = speed;
    }

    if (server.hasArg("ytop")) config.yPosTop = server.arg("ytop").toInt();
    if (server.hasArg("y1")) config.yPos1st = server.arg("y1").toInt();
    if (server.hasArg("y2")) config.yPos2nd = server.arg("y2").toInt();
    if (server.hasArg("y3")) config.yPosAlt = server.arg("y3").toInt();
    
    config.save();
    
    broadcastStatus("Device restarting - settings saved", "warning");
    
    String html = FPSTR(SAVE_SUCCESS_PAGE);
    server.send(200, "text/html", html);
    delay(2000);
    ESP.restart();
  });

  server.on("/apply", HTTP_POST, []() {
    Serial.println("⚙️  /apply endpoint called");

    String oldSSID = String(config.wifiSSID);
    String oldPassword = String(config.wifiPassword);
    String oldStation = String(config.stationCode);
    bool oldCallingAt = config.useCallingAt;
    int oldExtraServices = config.extraServices;

    // Validate SSID
    if (server.hasArg("ssid")) {
      String ssid = server.arg("ssid");
      Serial.printf("  Validating SSID: '%s'\n", ssid.c_str());
      ValidationResult result = validateSSID(ssid);
      if (!result.valid) {
        Serial.printf("  ❌ SSID validation failed: %s\n", result.message.c_str());
        server.send(400, "text/plain", "Invalid SSID: " + result.message);
        return;
      }
      Serial.println("  ✅ SSID valid");
      safeStrCopy(config.wifiSSID, ssid, sizeof(config.wifiSSID));
    }

    // Validate password
    if (server.hasArg("password") && !server.arg("password").isEmpty()) {
      String password = server.arg("password");
      ValidationResult result = validatePassword(password);
      if (!result.valid) {
        server.send(400, "text/plain", "Invalid password: " + result.message);
        return;
      }
      safeStrCopy(config.wifiPassword, password, sizeof(config.wifiPassword));
    }

    // Validate station code
    if (server.hasArg("station")) {
      String station = sanitizeStationCode(server.arg("station"));
      ValidationResult result = validateStationCode(station);
      if (!result.valid) {
        server.send(400, "text/plain", "Invalid station code: " + result.message);
        return;
      }
      safeStrCopy(config.stationCode, station, sizeof(config.stationCode));
    }

    // Validate refresh interval
    if (server.hasArg("interval")) {
      int interval = server.arg("interval").toInt();
      ValidationResult result = validateRange(interval, Data::MIN_REFRESH_INTERVAL, Data::MAX_REFRESH_INTERVAL, "Refresh interval");
      if (!result.valid) {
        server.send(400, "text/plain", result.message);
        return;
      }
      config.refreshInterval = interval;
    }

    if (server.hasArg("mode")) config.useCallingAt = (server.arg("mode") == "1");
    if (server.hasArg("showstation")) config.showStationName = (server.arg("showstation") == "1");

    // Validate extra services
    if (server.hasArg("extra")) {
      int extra = server.arg("extra").toInt();
      config.extraServices = constrainToRange(extra, 0, Data::MAX_EXTRA_SERVICES);
    }

    // Validate scroll speed
    if (server.hasArg("scrollspeed")) {
      int speed = server.arg("scrollspeed").toInt();
      ValidationResult result = validateRange(speed, Data::MIN_SCROLL_SPEED, Data::MAX_SCROLL_SPEED, "Scroll speed");
      if (!result.valid) {
        server.send(400, "text/plain", result.message);
        return;
      }
      config.scrollSpeed = speed;
    }

    // Validate rotation speed
    if (server.hasArg("rotationspeed")) {
      int speed = server.arg("rotationspeed").toInt();
      ValidationResult result = validateRange(speed, Data::MIN_ROTATION_SPEED, Data::MAX_ROTATION_SPEED, "Rotation speed");
      if (!result.valid) {
        server.send(400, "text/plain", result.message);
        return;
      }
      config.rotationSpeed = speed;
    }

    if (server.hasArg("ytop")) config.yPosTop = server.arg("ytop").toInt();
    if (server.hasArg("y1")) config.yPos1st = server.arg("y1").toInt();
    if (server.hasArg("y2")) config.yPos2nd = server.arg("y2").toInt();
    if (server.hasArg("y3")) config.yPosAlt = server.arg("y3").toInt();
    
    // Initialize rotation index based on display mode
    if (config.useCallingAt) {
      if (!config.showStationName) {
        displayState.currentAlternatingService = 2;  // Calling at mode, station hidden: start from service[2]
      } else {
        displayState.currentAlternatingService = 1;  // Calling at mode, station shown: start from service[1]
      }
    } else if (!config.showStationName) {
      displayState.currentAlternatingService = 3;  // Standard mode, station hidden: start from service[3]
    } else {
      displayState.currentAlternatingService = 2;  // Standard mode, station shown: start from service[2]
    }
    displayState.callingAtScrollOffset = 0;
    
    config.save();
    
    bool stationChanged = (oldStation != String(config.stationCode));
    bool displayModeChanged = (oldCallingAt != config.useCallingAt) || (oldExtraServices != config.extraServices);
    bool switchedToCallingAt = (!oldCallingAt && config.useCallingAt);

    // Clear calling points when switching TO calling at mode
    // This ensures we fetch fresh data with the detailed API endpoint
    // The display will show "Loading stops..." until data arrives
    if (switchedToCallingAt) {
      for (int i = 0; i < 6; i++) {
        displayState.services[i].callingPoints[0] = '\0';
      }
      // Force immediate fetch to get calling points
      if (fetchStateData.state != FETCH_IDLE) {
        fetchClient.stop();
        fetchStateData.state = FETCH_IDLE;
      }
      fetchStateData.lastSuccess = 0;
      fetchStateData.lastAttempt = 0;
    }

    // Notify WebSocket clients immediately
    broadcastStatus("Settings applied successfully", "success");
    
    // If display mode changed, broadcast updated display snapshot immediately
    if (displayModeChanged) {
      broadcastDisplaySnapshot();
      broadcastTrainUpdate();
    }

    String html = FPSTR(APPLY_SUCCESS_PAGE);
    server.send(200, "text/html", html);
    
    // Force immediate data fetch when station changes OR when switching to calling at
    if (stationChanged || switchedToCallingAt) {
      displayState.serviceCount = 0;
      displayState.fetchingNewStation = true;  // Mark that we're loading new station data
      
      if (fetchStateData.state != FETCH_IDLE) {
        fetchClient.stop();
        fetchStateData.state = FETCH_IDLE;
      }
      
      fetchStateData.lastSuccess = 0;
      fetchStateData.lastAttempt = 0;
      
      if (stationChanged) {
        Serial.println("🔄 Station changed to " + String(config.stationCode) + " - fetching immediately");
        broadcastStatus("Station changed - fetching new data...", "info");
      } else if (switchedToCallingAt) {
        Serial.println("🔄 Switched to Calling At mode - fetching detailed data...");
        broadcastStatus("Fetching calling points...", "info");
      }
    }
  });

  server.on("/reset", HTTP_GET, []() {
    if (SPIFFS.exists("/config.json")) {
      SPIFFS.remove("/config.json");
    }
    
    File flagFile = SPIFFS.open("/firstboot.flag", "w");
    if (flagFile) {
      flagFile.println("1");
      flagFile.close();
    }
    
    broadcastStatus("Factory reset initiated", "warning");
    
    String html = FPSTR(RESET_SUCCESS_PAGE);
    server.send(200, "text/html", html);
    
    delay(2000);
    ESP.restart();
  });

  server.on("/scan", HTTP_GET, []() {
    String json = "{\"networks\":[";
    int n = WiFi.scanNetworks();
    
    for (int i = 0; i < n; i++) {
      if (i > 0) json += ",";
      json += "{";
      json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
      json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
      json += "\"encryption\":" + String(WiFi.encryptionType(i));
      json += "}";
    }
    
    json += "]}";
    WiFi.scanDelete();
    server.send(200, "application/json", json);
  });

  server.on("/api/status", HTTP_GET, []() {
    String json = "{";
    json += "\"station\":\"" + String(config.stationCode) + "\",";
    json += "\"displayState.stationName\":\"" + String(displayState.stationName) + "\",";
    json += "\"services\":" + String(displayState.serviceCount) + ",";
    json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    json += "\"uptime\":" + String(millis() / 1000) + ",";
    json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"firmwareVersion\":\"" + config.firmwareVersion + "\",";
    json += "\"deviceId\":\"" + config.deviceId + "\",";
    json += "\"useCallingAt\":" + String(config.useCallingAt ? "true" : "false") + ",";
    json += "\"extraServices\":" + String(config.extraServices) + ",";
    json += "\"rotationSpeed\":" + String(config.rotationSpeed) + ",";
    json += "\"lastUpdate\":" + String(fetchStateData.lastSuccess / 1000);
    json += "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("✅ HTTP server started on port 80");
  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("✅ WebSocket server started on port 81");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== Train Departure Board Starting ===");
  
  // Initialize display
  u8g2.begin();
  u8g2.setFont(u8g2_font_helvB08_tr);
  
  displaySplashScreen();
  
  // Initialize SPIFFS
  displayProgress("Initializing storage...", 1, 5, 0);
  if (!SPIFFS.begin(true)) {
    Serial.println("❌ SPIFFS Mount Failed");
    systemFlags.systemError = true;
    return;
  }
  
  // Check for first boot
  systemFlags.firstBoot = checkFirstBoot();
  
  // Load or create configuration
  displayProgress("Loading configuration...", 2, 5, 20);
  config.load();
  
  // Ensure device ID is set
  if (config.deviceId.length() == 0 || config.deviceId == "") {
    config.deviceId = generateDeviceId();
    config.save();
    Serial.println("🆔 Generated Device ID: " + config.deviceId);
  }
  
  // If first boot, show welcome screen and start AP mode
  if (systemFlags.firstBoot) {
    displayWelcomeScreen();
    startAccessPoint();
    displayAPScreen();
    return;
  }
  
  // Initialize WiFi
  displayProgress("Connecting to WiFi...", 3, 5, 40);
  if (!initializeWiFi()) {
    Serial.println("❌ WiFi connection failed - Starting AP mode");
    startAccessPoint();
    displayAPScreen();
    return;
  }
  
  // Initialize time sync
  displayProgress("Syncing time...", 4, 5, 60);
  initializeTimeSync();
  
  // Setup web server and WebSocket
  displayProgress("Starting services...", 5, 5, 80);
  setupWebServer();
  
  // Setup fetch client once for better performance
  fetchClient.setInsecure();
  fetchClient.setTimeout(8000);  // Reduced from 15s to 8s for faster failure detection
  fetchStateData.buffer.reserve(16384);  // Pre-allocate buffer
  
  // Setup OTA
  setupOTA();

  displayProgress("System ready!", 5, 5, 100);
  delay(500);

  IPAddress localIp = WiFi.localIP();
  displayReadyScreen(localIp);
  delay(4000);

  Serial.println("\n✅ Setup complete!");
  Serial.println("📍 IP: " + WiFi.localIP().toString());
  Serial.println("🌐 WebSocket ready on port 81");
  Serial.println("📊 Free heap: " + String(ESP.getFreeHeap()) + " bytes");
  
  // Connect to monitoring server
  if (monitoringState.enabled && !systemFlags.apMode) {
    Serial.println("🔌 Connecting to monitoring server...");
    Serial.println("   Host: " + monitoringState.serverHost + ":" + String(monitoringState.serverPort));
    monitorClient.begin(monitoringState.serverHost, monitoringState.serverPort, "/ws");
    monitorClient.onEvent(monitorWebSocketEvent);
    monitorClient.setReconnectInterval(5000);
    delay(500);
  }
  
  // Initial data fetch will happen automatically in loop
}

void loop() {
  server.handleClient();
  webSocket.loop();
  
  ArduinoOTA.handle();

  if (systemFlags.systemError) {
    handleSystemError();
    return;
  }

  if (systemFlags.apMode) {
    static unsigned long lastAPUpdate = 0;
    if (millis() - lastAPUpdate > Timing::AP_DISPLAY_UPDATE) {
      displayAPScreen();
      lastAPUpdate = millis();
    }
    delay(100);
    return;
  }

  unsigned long currentTime = millis();

  // Fetch with regular intervals - ← FIXED: Proper indentation
  if (fetchStateData.state == FETCH_IDLE) {
    unsigned long timeSinceLastSuccess = currentTime - fetchStateData.lastSuccess;
    unsigned long timeSinceLastAttempt = currentTime - fetchStateData.lastAttempt;
    
    bool shouldFetch = (timeSinceLastSuccess >= config.refreshInterval * 1000UL);
    bool enoughTimeSinceAttempt = (timeSinceLastAttempt >= Net::MIN_FETCH_RETRY_INTERVAL);
    bool forceFetch = (fetchStateData.lastAttempt == 0 && fetchStateData.lastSuccess == 0);
    
    if ((shouldFetch && enoughTimeSinceAttempt) || forceFetch) {
      if (WiFi.status() == WL_CONNECTED) {
        fetchStateData.lastAttempt = currentTime;
        if (!asyncFetchStart()) {
          displayStatus("ERR");
        }
      } else {
        Serial.println("❌ WiFi disconnected");
        displayStatus("OFF");
        fetchStateData.lastAttempt = currentTime;
        if (!initializeWiFi()) startAccessPoint();
      }
    }
  }

  handleFetchStateMachine();
  handleAlternatingService(currentTime);
  updateDisplay();  // CRITICAL: Called every loop for smooth animations and clock updates!
  
  // Handle monitoring server connection
  if (monitoringState.enabled) {
    // Throttle monitoring loop to prevent blocking on reconnection attempts
    static unsigned long lastMonitorLoop = 0;
    
    // Skip monitoring loop for 2 seconds after disconnect to prevent immediate 
    // reconnection blocking the display
    bool recentlyDisconnected = (currentTime - monitoringState.lastDisconnect < 2000);
    
    // Only call loop() every 500ms to reduce blocking impact, and not right after disconnect
    if (!recentlyDisconnected && currentTime - lastMonitorLoop > 500) {
      monitorClient.loop();
      lastMonitorLoop = currentTime;
    }
    
    // Send heartbeat to monitoring server (only if connected)
    if (monitoringState.connected && currentTime - monitoringState.lastHeartbeat >= Timing::MONITOR_HEARTBEAT_INTERVAL) {
      sendMonitorHeartbeat();
      monitoringState.lastHeartbeat = currentTime;
    }
  }
  
  // Broadcast metrics periodically
  if (currentTime - wsClients.lastMetricsBroadcast >= Timing::METRICS_BROADCAST_INTERVAL) {
    broadcastMetrics();
    wsClients.lastMetricsBroadcast = currentTime;
  }

  // Broadcast display snapshot for live preview
  if (currentTime - displayState.lastSnapshot >= Timing::SNAPSHOT_BROADCAST_INTERVAL) {
    broadcastDisplaySnapshot();
    displayState.lastSnapshot = currentTime;
  }

  delay(Timing::LOOP_DELAY);
}