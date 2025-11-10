#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>
#include <time.h>
#include <ArduinoOTA.h>
#include <WebSocketsServer.h>  // ← NEW: WebSocket support
#include <WebSocketsClient.h>  // ← MONITORING: WebSocket client for monitoring server
#include <HTTPUpdate.h>        // ← MONITORING: For OTA updates from monitoring server
#include "config.h"
#include "web_pages.h"

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

// ---------------- Display ----------------
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R0, 5, 16, 17);

// ---------------- Web Server ----------------
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);  // ← NEW: WebSocket on port 81

// ---------------- Configuration ----------------
Config config;

// ---------------- API Settings ----------------
const char* apiHost = "lite.realtime.nationalrail.co.uk";
const char* apiPath = "/OpenLDBWS/ldb9.asmx";
const char* apiToken = "73ee3834-af35-4f22-9b8b-480b70571c39";

// ---------------- Data ----------------
struct ServiceData {
  char std[6];
  char etd[10];
  char destination[30];
  char callingPoints[500];
};

ServiceData services[6];
int serviceCount = 0;
unsigned long lastDataUpdate = 0;
unsigned long lastRotation = 0;
char stationName[50] = "Station";
int currentAlternatingService = 2;
bool isAnimating = false;
int animationOffset = 0;
int callingAtScrollOffset = 0;
unsigned long lastCallingAtScroll = 0;
bool apMode = false;
bool systemError = false;
bool firstBoot = false;
bool fetchingNewStation = false;  // Track when we're fetching data for a station change
unsigned long lastDisplaySnapshot = 0;

enum FetchState {
  FETCH_IDLE,
  FETCH_START,
  FETCH_WAITING,
  FETCH_READING,
  FETCH_DONE,
  FETCH_FAIL
};

FetchState fetchState = FETCH_IDLE;
WiFiClientSecure fetchClient;
String fetchBuffer;
unsigned long fetchStartTime = 0;
unsigned long lastFetchAttempt = 0;
unsigned long lastSuccessfulFetch = 0;

// ← NEW: WebSocket connected clients
uint8_t connectedClients[10];
uint8_t clientCount = 0;
unsigned long lastMetricsBroadcast = 0;

// ← MONITORING: Remote monitoring server configuration
String monitorServerHost = "192.168.0.75";  // Change to your PC's IP address
int monitorServerPort = 3000;
bool monitoringEnabled = true;
WebSocketsClient monitorClient;
unsigned long lastMonitorHeartbeat = 0;
const unsigned long MONITOR_HEARTBEAT_INTERVAL = 30000;  // 30 seconds
bool monitorConnected = false;

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
String extractTagValue(String xml, String tag, String ns = "");
String decodeHTMLEntities(String text);
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
String generateDeviceId();
void setupOTA();
String formatETD(String etd);
String fitTextToWidth(String text, int maxWidth);
void monitorWebSocketEvent(WStype_t type, uint8_t * payload, size_t length);
void sendMonitorHeartbeat();

// ============ NEW: WebSocket Functions ============

// ============ MONITORING: Remote Monitoring Functions ============

void monitorWebSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("❌ Monitoring server disconnected");
      monitorConnected = false;
      break;
      
    case WStype_CONNECTED:
      {
        Serial.println("✅ Connected to monitoring server");
        monitorConnected = true;
        
        // Register device
        String registerMsg = "{";
        registerMsg += "\"type\":\"register\",";
        registerMsg += "\"deviceId\":\"" + config.deviceId + "\",";
        registerMsg += "\"name\":\"Board-" + config.deviceId.substring(0, 8) + "\",";
        registerMsg += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
        registerMsg += "\"firmwareVersion\":\"" + config.firmwareVersion + "\",";
        registerMsg += "\"stationCode\":\"" + String(config.stationCode) + "\",";
        registerMsg += "\"stationName\":\"" + String(stationName) + "\",";
        registerMsg += "\"rssi\":" + String(WiFi.RSSI()) + ",";
        registerMsg += "\"uptime\":" + String(millis() / 1000) + ",";
        registerMsg += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
        registerMsg += "\"services\":" + String(serviceCount);
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
                currentAlternatingService = 1;
              } else if (!config.showStationName) {
                currentAlternatingService = 3;
              } else {
                currentAlternatingService = 2;
              }
              
              // Clear data and force refresh
              serviceCount = 0;
              lastSuccessfulFetch = 0;
              lastFetchAttempt = 0;
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
  if (!monitorConnected) return;
  
  String heartbeat = "{";
  heartbeat += "\"type\":\"heartbeat\",";
  heartbeat += "\"deviceId\":\"" + config.deviceId + "\",";
  heartbeat += "\"name\":\"Board-" + config.deviceId.substring(0, 8) + "\",";
  heartbeat += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  heartbeat += "\"firmwareVersion\":\"" + config.firmwareVersion + "\",";
  heartbeat += "\"stationCode\":\"" + String(config.stationCode) + "\",";
  heartbeat += "\"stationName\":\"" + String(stationName) + "\",";
  heartbeat += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  heartbeat += "\"uptime\":" + String(millis() / 1000) + ",";
  heartbeat += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  heartbeat += "\"services\":" + String(serviceCount);
  heartbeat += "}";
  
  monitorClient.sendTXT(heartbeat);
}

// ============ END MONITORING Functions ============

void addClient(uint8_t num) {
  if (clientCount < 10) {
    connectedClients[clientCount++] = num;
    Serial.printf("✅ Client #%u added (total: %u)\n", num, clientCount);
  }
}

void removeClient(uint8_t num) {
  for (int i = 0; i < clientCount; i++) {
    if (connectedClients[i] == num) {
      for (int j = i; j < clientCount - 1; j++) {
        connectedClients[j] = connectedClients[j + 1];
      }
      clientCount--;
      Serial.printf("❌ Client #%u removed (total: %u)\n", num, clientCount);
      break;
    }
  }
}

void broadcastStatus(const char* message, const char* level = "info") {
  if (clientCount == 0) return;
  
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
  if (clientCount == 0) return;
  
  String json = "{";
  json += "\"type\":\"train_update\",";
  json += "\"timestamp\":" + String(millis()) + ",";
  json += "\"station\":\"" + String(stationName) + "\",";
  json += "\"stationCode\":\"" + String(config.stationCode) + "\",";
  json += "\"services\":" + String(serviceCount) + ",";
  json += "\"trains\":[";
  
  for (int i = 0; i < serviceCount && i < 6; i++) {
    if (i > 0) json += ",";
    json += "{";
    json += "\"std\":\"" + String(services[i].std) + "\",";
    json += "\"etd\":\"" + String(services[i].etd) + "\",";
    json += "\"destination\":\"" + String(services[i].destination) + "\"";
    if (config.useCallingAt && i == 0 && strlen(services[i].callingPoints) > 0) {
      json += ",\"callingAt\":\"" + String(services[i].callingPoints) + "\"";
    }
    json += "}";
  }
  
  json += "]}";
  
  webSocket.broadcastTXT(json);
  Serial.println("📡 Broadcast train update to " + String(clientCount) + " clients");
}

void broadcastMetrics() {
  if (clientCount == 0) return;
  
  String json = "{";
  json += "\"type\":\"metrics\",";
  json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"services\":" + String(serviceCount) + ",";
  json += "\"station\":\"" + String(stationName) + "\",";
  json += "\"lastUpdate\":" + String(lastSuccessfulFetch / 1000);
  json += "}";
  
  webSocket.broadcastTXT(json);
}

void sendCurrentState(uint8_t num) {
  String json = "{";
  json += "\"type\":\"state\",";
  json += "\"station\":\"" + String(config.stationCode) + "\",";
  json += "\"stationName\":\"" + String(stationName) + "\",";
  json += "\"services\":" + String(serviceCount) + ",";
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
    if (fetchState == FETCH_IDLE) {
      lastSuccessfulFetch = 0;
      lastFetchAttempt = 0;
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
  if (clientCount == 0) return;
  
  String json = "{";
  json += "\"type\":\"display_snapshot\",";
  json += "\"timestamp\":" + String(millis()) + ",";
  json += "\"stationName\":\"" + String(stationName) + "\",";
  json += "\"mode\":\"" + String(config.useCallingAt ? "calling_at" : "normal") + "\",";
  json += "\"serviceCount\":" + String(serviceCount) + ",";
  json += "\"services\":[";
  
  // Send up to 3 services for the preview
  for (int i = 0; i < min(serviceCount, 3); i++) {
    if (i > 0) json += ",";
    json += "{";
    json += "\"std\":\"" + String(services[i].std) + "\",";
    json += "\"etd\":\"" + String(services[i].etd) + "\",";
    json += "\"destination\":\"" + String(services[i].destination) + "\"";
    json += "}";
  }
  
  json += "],";
  
  // Include calling points if in calling at mode
  if (config.useCallingAt && serviceCount > 0 && strlen(services[0].callingPoints) > 0) {
    json += "\"callingPoints\":\"" + String(services[0].callingPoints) + "\",";
  }
  
  // Current time
  time_t now = time(nullptr);
  if (now > 100000) {
    struct tm* timeInfo = localtime(&now);
    char timeString[9];
    strftime(timeString, sizeof(timeString), "%H:%M:%S", timeInfo);
    json += "\"time\":\"" + String(timeString) + "\",";
  }
  
  json += "\"alternatingService\":" + String(currentAlternatingService) + ",";
  json += "\"isAnimating\":" + String(isAnimating ? "true" : "false");
  json += "}";
  
  webSocket.broadcastTXT(json);
}

// Display Functions - keeping the working versions from original
void displaySplashScreen() {
  u8g2.clearBuffer();
  u8g2.drawXBMP(28, 12, 200, 40, logo_bitmap);
  u8g2.sendBuffer();
  delay(5000);
}

void displayProgress(const char* step, int currentStep, int totalSteps, int progress) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_t0_12_tf);
  
  String stepText = "Step " + String(currentStep) + "/" + String(totalSteps);
  u8g2.setCursor(5, 12);
  u8g2.print(stepText);
  
  String percentText = String(progress) + "%";
  int percentWidth = u8g2.getUTF8Width(percentText.c_str());
  u8g2.setCursor(256 - percentWidth - 5, 12);
  u8g2.print(percentText);
  
  u8g2.setCursor(5, 28);
  u8g2.print(step);
  
  int segments = 10;
  int segmentWidth = 22;
  int segmentHeight = 16;
  int spacing = 2;
  int startX = 5;
  int startY = 36;
  
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
  u8g2.setCursor((256 - width) / 2, 16);
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
  u8g2.setFont(u8g2_font_t0_11b_tf);

  u8g2.setCursor(5, 10);
  u8g2.print("SETUP MODE");

  u8g2.setFont(u8g2_font_helvB10_tr);
  u8g2.drawFrame(5, 18, 246, 26);
  u8g2.setCursor(10, 30);
  u8g2.print("SSID: TrainBoard_AP");
  u8g2.setCursor(10, 42);
  u8g2.print("Pass: config123");

  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.setCursor(5, 56);
  u8g2.print("Visit: ");
  u8g2.setFont(u8g2_font_t0_11b_tf);
  IPAddress ip = WiFi.softAPIP();
  u8g2.print(ip.toString().c_str());

  u8g2.sendBuffer();
}

void displayReadyScreen(const IPAddress& ip) {
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_helvB12_tr);
  const char* title = "Connected!";
  int titleWidth = u8g2.getUTF8Width(title);
  u8g2.setCursor((256 - titleWidth) / 2, 18);
  u8g2.print(title);

  u8g2.setFont(u8g2_font_t0_11b_tf);
  String ipLine = "IP: " + ip.toString();
  int ipWidth = u8g2.getUTF8Width(ipLine.c_str());
  u8g2.setCursor((256 - ipWidth) / 2, 34);
  u8g2.print(ipLine);

  u8g2.setFont(u8g2_font_t0_11_tf);
  String visitLine1 = "Visit " + ip.toString() + " in your";
  String visitLine2 = "browser to change settings...";

  u8g2.setCursor(10, 50);
  u8g2.print(visitLine1);
  u8g2.setCursor(10, 62);
  u8g2.print(visitLine2);

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

String formatETD(String etd) {
  if (etd.length() == 5 && etd.indexOf(":") != -1) {
    return "Exp " + etd;
  }
  return etd;
}

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
    apMode = false;
    return true;
  } else {
    Serial.println("\n❌ WiFi Connection Failed");
    return false;
  }
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
  apMode = true;
  setupWebServer();
}

void initializeTimeSync() {
  Serial.println("🕒 Syncing time with NTP...");
  configTime(0, 0, "pool.ntp.org");
  unsigned long startTime = millis();
  while (!time(nullptr) && millis() - startTime < 10000) {
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
  
  fetchClient.setInsecure();
  fetchClient.setTimeout(15000);

  if (!fetchClient.connect(apiHost, 443)) {
    Serial.println("❌ API connection failed");
    displayStatus("FAIL");
    return false;
  }

  broadcastStatus("Fetching train data...", "info");

  // Always fetch maximum services so we have data available when settings change
  // Maximum useful services: 1 or 2 base + 3 extra = 4 or 5 total
  // Fetch 6 to have buffer for any configuration
  int numRows = 6;
  
  String soapRequest = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
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
  
  fetchClient.print("POST " + String(apiPath) + " HTTP/1.1\r\n"
                    "Host: " + String(apiHost) + "\r\n"
                    "Content-Type: text/xml\r\n"
                    "Content-Length: " + String(soapRequest.length()) + "\r\n"
                    "Connection: close\r\n\r\n" + soapRequest);

  fetchStartTime = millis();
  fetchBuffer = "";
  fetchState = FETCH_WAITING;
  displayStatus("Fetch");
  return true;
}

void handleFetchStateMachine() {
  switch (fetchState) {
    case FETCH_WAITING:
      if (fetchClient.connected() || fetchClient.available()) {
        fetchState = FETCH_READING;
      } else if (millis() - fetchStartTime > 15000) {
        fetchClient.stop();
        Serial.println("❌ Timeout WAITING");
        displayStatus("TIMEOUT");
        fetchState = FETCH_FAIL;
      }
      break;

    case FETCH_READING:
      while (fetchClient.available()) {
        char c = fetchClient.read();
        fetchBuffer += c;
      }
      
      // Check if done - connection closed and no more data
      if (!fetchClient.connected() && !fetchClient.available()) {
        fetchClient.stop();
        if (fetchBuffer.length() > 100) {  // Valid response is always >100 bytes
          Serial.println("✅ Fetched: " + String(fetchBuffer.length()) + " bytes");
          fetchState = FETCH_DONE;
        } else {
          Serial.println("❌ Invalid response size: " + String(fetchBuffer.length()) + " bytes");
          fetchState = FETCH_FAIL;
        }
      }
      
      // Timeout protection
      if (millis() - fetchStartTime > 20000) {
        fetchClient.stop();
        if (fetchBuffer.length() > 100) {
          // Got data but took too long - still use it
          Serial.println("⚠️ Slow fetch (" + String(fetchBuffer.length()) + " bytes) - using anyway");
          fetchState = FETCH_DONE;
        } else {
          Serial.println("❌ Timeout READING");
          displayStatus("TIMEOUT");
          fetchState = FETCH_FAIL;
        }
      }
      break;

    case FETCH_DONE:
    if (parseAndDisplayResponse(fetchBuffer)) {
      displayStatus("OK");
      lastSuccessfulFetch = millis();  
      lastDataUpdate = millis();
      Serial.println("✅ Parse successful");
    } else {
        displayStatus("ERR");
        Serial.println("❌ Parse failed");
      }
      fetchBuffer = "";
      fetchState = FETCH_IDLE;
      break;

    case FETCH_FAIL:
      displayStatus("FAIL");
      fetchClient.stop();
      fetchBuffer = "";
      fetchState = FETCH_IDLE;
      lastDataUpdate = millis();
      broadcastStatus("Failed to fetch train data", "error");
      break;

    default:
      break;
  }
}

// Parse function - using the WORKING logic from original
bool parseAndDisplayResponse(String response) {
  serviceCount = 0;

  Serial.println("📊 Processing (" + String(response.length()) + " bytes)");

  if (response.indexOf("soap:Fault") != -1) {
    Serial.println("❌ SOAP Fault");
    return false;
  }

  String station = extractTagValue(response, "locationName", "lt4");
  if (station == "") station = extractTagValue(response, "locationName", "lt5");
  
  if (station.length() > 0) {
    station.toCharArray(stationName, sizeof(stationName));
    Serial.println("📍 " + station);
  }

  int servicesStart = response.indexOf("<lt5:trainServices>");
  if (servicesStart == -1) servicesStart = response.indexOf("<lt4:trainServices>");
  
  if (servicesStart == -1) {
    Serial.println("❌ No services");
    return false;
  }

  int servicesEnd = response.indexOf("</lt5:trainServices>", servicesStart);
  if (servicesEnd == -1) servicesEnd = response.indexOf("</lt4:trainServices>", servicesStart);
  if (servicesEnd == -1) servicesEnd = response.length();

  String trainServices = response.substring(servicesStart, servicesEnd);
  
  String serviceTag = "<lt5:service>";
  String serviceEndTag = "</lt5:service>";
  
  int pos = 0;
  // Always parse maximum services (up to 6) so data is available when settings change
  int maxServices = 6;

  while (serviceCount < maxServices) {
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
      std.toCharArray(services[serviceCount].std, sizeof(services[serviceCount].std));
      etd.toCharArray(services[serviceCount].etd, sizeof(services[serviceCount].etd));
      destination.toCharArray(services[serviceCount].destination, sizeof(services[serviceCount].destination));
      services[serviceCount].callingPoints[0] = '\0';
      
      Serial.println("🚂 " + String(serviceCount + 1) + ": " + std + " → " + destination);
      
      if (config.useCallingAt && serviceCount == 0) {
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
                  callingPoints.toCharArray(services[serviceCount].callingPoints, 500);
                  Serial.println("  ✅ Stored calling points");
                } else if (callingPoints == "") {
                  String fallback = "No further stops available";
                  fallback.toCharArray(services[serviceCount].callingPoints, 500);
                }
              }
            }
          }
        } else {
          String fallback = "No further stops available";
          fallback.toCharArray(services[serviceCount].callingPoints, 500);
        }
      }
      
      serviceCount++;
      
      // Allow display updates and other tasks to run
      // This prevents the display from freezing during long parsing operations
      yield();
    }
    
    pos = serviceEnd;
  }

  Serial.println("✅ " + String(serviceCount) + " services");

  if (serviceCount > 0) {
    fetchingNewStation = false;  // Clear loading flag - we have data now
    broadcastTrainUpdate();  // Sends full train data to clients
    broadcastStatus("Train data updated", "success");
  }

  return serviceCount > 0;
}

// Animation - using WORKING logic from original
void handleAlternatingService(unsigned long currentTime) {
  int textHeight = u8g2.getAscent() - u8g2.getDescent();
  const int scrollStep = 2;
  const int maxOffset = textHeight;

  int minServicesForAlt, maxServiceIndex;
  if (config.useCallingAt) {
    minServicesForAlt = config.extraServices + 2;  // Need at least extra + 2
    maxServiceIndex = config.extraServices;
  } else {
    minServicesForAlt = config.extraServices + 3;  // Need at least extra + 3
    maxServiceIndex = config.extraServices + 1;
  }
  
  unsigned long rotationInterval = config.rotationSpeed * 1000UL;  // Use config setting
  
  if (currentTime - lastRotation >= rotationInterval && serviceCount >= minServicesForAlt && !isAnimating) {
    isAnimating = true;
    animationOffset = 0;
    lastRotation = currentTime;
  }

  if (isAnimating) {
    animationOffset += scrollStep;
    if (animationOffset >= maxOffset) {
      isAnimating = false;
      
      if (config.useCallingAt) {
        currentAlternatingService++;
        if (currentAlternatingService > config.extraServices) currentAlternatingService = 1;
      } else {
        currentAlternatingService++;
        if (currentAlternatingService > config.extraServices + 1) currentAlternatingService = 2;
      }
      
      animationOffset = 0;
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

  String displayName = stationName;
  int nameWidth = u8g2.getUTF8Width(displayName.c_str());
  
  if (nameWidth > 250) {
    while (u8g2.getUTF8Width(displayName.c_str()) > 250 && displayName.length() > 3) {
      displayName.remove(displayName.length() - 1);
    }
    displayName += "...";
    nameWidth = u8g2.getUTF8Width(displayName.c_str());
  }
  
  u8g2.setCursor((256 - nameWidth) / 2, 12);
  u8g2.print(displayName);

  // Display message when no services available
  if (serviceCount == 0) {
    u8g2.setFont(u8g2_font_helvB10_tr);
    String msg = fetchingNewStation ? "Loading station data..." : "No trains scheduled";
    int msgWidth = u8g2.getUTF8Width(msg.c_str());
    u8g2.setCursor((256 - msgWidth) / 2, 35);
    u8g2.print(msg);
  }
  else if (config.useCallingAt && serviceCount > 0) {
    const int ETD_RIGHT_X = 251;  // Fixed position for right-aligned ETD
    
    int yPos = config.yPos1st;
    String leftSide = "1st " + String(services[0].std) + " ";
    String rightSide = formatETD(String(services[0].etd));

    int leftWidth = u8g2.getUTF8Width(leftSide.c_str());
    int rightWidth = u8g2.getUTF8Width(rightSide.c_str());
    int availableWidth = 256 - leftWidth - rightWidth - 10;

    String destination = fitTextToWidth(String(services[0].destination), availableWidth);

    u8g2.setCursor(1, yPos);
    u8g2.print(leftSide + destination);
    u8g2.setCursor(ETD_RIGHT_X - rightWidth, yPos);
    u8g2.print(rightSide);

    if (strlen(services[0].callingPoints) > 0) {
      String label = "Calling at: ";
      int labelWidth = u8g2.getUTF8Width(label.c_str());
      
      String callingText = String(services[0].callingPoints);
      String loopingText = callingText + " * " + callingText;
      
      int fullTextWidth = u8g2.getUTF8Width(callingText.c_str());
      int availableSpace = 256 - labelWidth - 5;

      bool needsScroll = fullTextWidth > availableSpace;

      if (needsScroll) {
        unsigned long currentTime = millis();
        if (currentTime - lastCallingAtScroll > config.scrollSpeed) {
          callingAtScrollOffset++;
          lastCallingAtScroll = currentTime;
        }

        if (callingAtScrollOffset > fullTextWidth + 15) {
          callingAtScrollOffset = 0;
        }

        u8g2.setCursor(1, config.yPos2nd);
        u8g2.print(label);

        u8g2.setClipWindow(labelWidth + 2, 0, 255, 63);
        u8g2.setCursor(labelWidth + 2 - callingAtScrollOffset, config.yPos2nd);
        u8g2.print(loopingText);
        u8g2.setMaxClipWindow();
      } else {
        u8g2.setCursor(1, config.yPos2nd);
        u8g2.print(label);
        u8g2.print(callingText);
        callingAtScrollOffset = 0;
      }
    } else {
      // Show loading message when calling points aren't available yet
      u8g2.setCursor(1, config.yPos2nd);
      u8g2.print("Calling at: Loading stops...");
      callingAtScrollOffset = 0;
    }

    int minServices = 2 + config.extraServices;
    if (serviceCount >= 2) {
      int baselineY = config.yPosAlt;
      int ascent = u8g2.getAscent();
      int descent = u8g2.getDescent();
      int textHeight = ascent - descent;

      int indexA = currentAlternatingService;
      int indexB;
      
      // Calculate next index in rotation
      indexB = indexA + 1;
      if (indexB > config.extraServices) indexB = 1;

      String labelA = String(indexA == 1 ? "2nd " : (indexA == 2 ? "3rd " : "4th ")) + String(services[indexA].std) + " ";
      String rightA = formatETD(String(services[indexA].etd));

      int leftAWidth = u8g2.getUTF8Width(labelA.c_str());
      int rightAWidth = u8g2.getUTF8Width(rightA.c_str());
      int availA = 256 - leftAWidth - rightAWidth - 10;

      String destA = fitTextToWidth(String(services[indexA].destination), availA);

      u8g2.setClipWindow(0, baselineY - ascent, 255, baselineY - descent);

      if (isAnimating && serviceCount >= minServices) {
        int offsetY = animationOffset;

        u8g2.setCursor(1, baselineY - offsetY);
        u8g2.print(labelA + destA);
        u8g2.setCursor(ETD_RIGHT_X - rightAWidth, baselineY - offsetY);
        u8g2.print(rightA);

        String labelB = String(indexB == 1 ? "2nd " : (indexB == 2 ? "3rd " : "4th ")) + String(services[indexB].std) + " ";
        String rightB = formatETD(String(services[indexB].etd));

        int leftBWidth = u8g2.getUTF8Width(labelB.c_str());
        int rightBWidth = u8g2.getUTF8Width(rightB.c_str());
        int availB = 256 - leftBWidth - rightBWidth - 10;

        String destB = fitTextToWidth(String(services[indexB].destination), availB);

        u8g2.setCursor(1, baselineY + textHeight - offsetY);
        u8g2.print(labelB + destB);
        u8g2.setCursor(ETD_RIGHT_X - rightBWidth, baselineY + textHeight - offsetY);
        u8g2.print(rightB);
      } else {
        u8g2.setCursor(1, baselineY);
        u8g2.print(labelA + destA);
        u8g2.setCursor(ETD_RIGHT_X - rightAWidth, baselineY);
        u8g2.print(rightA);
      }

      u8g2.setMaxClipWindow();
    }

  } else {
    const int ETD_RIGHT_X = 251;  // Fixed position for right-aligned ETD
    
    for (int i = 0; i < min(serviceCount, 2); i++) {
      int yPos = (i == 0) ? config.yPos1st : config.yPos2nd;
      String leftSide = (i == 0 ? "1st " : "2nd ") + String(services[i].std) + " ";
      String rightSide = formatETD(String(services[i].etd));

      int leftWidth = u8g2.getUTF8Width(leftSide.c_str());
      int rightWidth = u8g2.getUTF8Width(rightSide.c_str());
      int availableWidth = 256 - leftWidth - rightWidth - 10;

      String destination = fitTextToWidth(String(services[i].destination), availableWidth);

      u8g2.setCursor(1, yPos);
      u8g2.print(leftSide + destination);
      u8g2.setCursor(ETD_RIGHT_X - rightWidth, yPos);
      u8g2.print(rightSide);
    }

    int minServices = 3 + config.extraServices;
    if (serviceCount >= 3) {
      int baselineY = config.yPosAlt;
      int ascent = u8g2.getAscent();
      int descent = u8g2.getDescent();
      int textHeight = ascent - descent;

      int indexA = currentAlternatingService;
      int indexB;
      
      // Calculate next index in rotation
      indexB = indexA + 1;
      if (indexB > config.extraServices + 1) indexB = 2;

      String labelA = String((indexA == 2 ? "3rd " : (indexA == 3 ? "4th " : "5th "))) + String(services[indexA].std) + " ";
      String rightA = formatETD(String(services[indexA].etd));

      int leftAWidth = u8g2.getUTF8Width(labelA.c_str());
      int rightAWidth = u8g2.getUTF8Width(rightA.c_str());
      int availA = 256 - leftAWidth - rightAWidth - 10;

      String destA = fitTextToWidth(String(services[indexA].destination), availA);

      u8g2.setClipWindow(0, baselineY - ascent, 255, baselineY - descent);

      if (isAnimating && serviceCount >= minServices) {
        int offsetY = animationOffset;

        u8g2.setCursor(1, baselineY - offsetY);
        u8g2.print(labelA + destA);
        u8g2.setCursor(ETD_RIGHT_X - rightAWidth, baselineY - offsetY);
        u8g2.print(rightA);

        String labelB = String((indexB == 2 ? "3rd " : (indexB == 3 ? "4th " : "5th "))) + String(services[indexB].std) + " ";
        String rightB = formatETD(String(services[indexB].etd));

        int leftBWidth = u8g2.getUTF8Width(labelB.c_str());
        int rightBWidth = u8g2.getUTF8Width(rightB.c_str());
        int availB = 256 - leftBWidth - rightBWidth - 10;

        String destB = fitTextToWidth(String(services[indexB].destination), availB);

        u8g2.setCursor(1, baselineY + textHeight - offsetY);
        u8g2.print(labelB + destB);
        u8g2.setCursor(ETD_RIGHT_X - rightBWidth, baselineY + textHeight - offsetY);
        u8g2.print(rightB);
      } else {
        u8g2.setCursor(1, baselineY);
        u8g2.print(labelA + destA);
        u8g2.setCursor(ETD_RIGHT_X - rightAWidth, baselineY);
        u8g2.print(rightA);
      }

      u8g2.setMaxClipWindow();
    }
  }

  // CRITICAL: Clock display - this was missing in refactored version!
  time_t now = time(nullptr);
  if (now > 100000) {
    struct tm* timeInfo = localtime(&now);
    char timeString[9];
    strftime(timeString, sizeof(timeString), "%H:%M:%S", timeInfo);
    u8g2.setFont(u8g2_font_t0_11_tf);
    int width = u8g2.getUTF8Width(timeString);
    u8g2.setCursor((256 - width) / 2, 64);
    u8g2.print(timeString);
  }

  u8g2.sendBuffer();
}


void setupWebServer() {
  server.on("/", HTTP_GET, []() {
    String html = FPSTR(CONFIG_PAGE_TEMPLATE);
    
    html.replace("{SSID}", String(config.wifiSSID));
    html.replace("{STATION}", String(config.stationCode));
    html.replace("{STATION_NAME}", String(stationName));
    html.replace("{INTERVAL}", String(config.refreshInterval));
    html.replace("{MODE_SEL_0}", config.useCallingAt ? "" : " selected");
    html.replace("{MODE_SEL_1}", config.useCallingAt ? " selected" : "");
    html.replace("{SHOWSTATION_SEL_1}", config.showStationName ? " selected" : "");
    html.replace("{SHOWSTATION_SEL_0}", !config.showStationName ? " selected" : "");
    html.replace("{EXTRA_SEL_1}", config.extraServices == 1 ? " selected" : "");
    html.replace("{EXTRA_SEL_2}", config.extraServices == 2 ? " selected" : "");
    html.replace("{EXTRA_SEL_3}", config.extraServices == 3 ? " selected" : "");
    html.replace("{SCROLL}", String(config.scrollSpeed));
    html.replace("{ROTATION}", String(config.rotationSpeed));
    html.replace("{Y1}", String(config.yPos1st));
    html.replace("{Y2}", String(config.yPos2nd));
    html.replace("{Y3}", String(config.yPosAlt));
    html.replace("{IP}", WiFi.localIP().toString());
    html.replace("{DEVICE_ID}", config.deviceId);
    
    server.send(200, "text/html", html);
  });

  server.on("/save", HTTP_POST, []() {
    if (server.hasArg("ssid")) {
      strncpy(config.wifiSSID, server.arg("ssid").c_str(), sizeof(config.wifiSSID) - 1);
      config.wifiSSID[sizeof(config.wifiSSID) - 1] = '\0';
    }
    if (server.hasArg("password") && !server.arg("password").isEmpty()) {
      strncpy(config.wifiPassword, server.arg("password").c_str(), sizeof(config.wifiPassword) - 1);
      config.wifiPassword[sizeof(config.wifiPassword) - 1] = '\0';
    }
    if (server.hasArg("station")) {
      String station = server.arg("station");
      station.trim();  // Remove any whitespace
      station.toUpperCase();
      // Extract only the 3-letter code if longer string provided
      if (station.length() >= 3) {
        station = station.substring(0, 3);
      }
      // Only apply if we have exactly 3 characters
      if (station.length() == 3) {
        strncpy(config.stationCode, station.c_str(), sizeof(config.stationCode) - 1);
        config.stationCode[3] = '\0';  // Ensure null termination
      }
    }
    if (server.hasArg("interval")) config.refreshInterval = server.arg("interval").toInt();
    if (server.hasArg("mode")) config.useCallingAt = (server.arg("mode") == "1");
    if (server.hasArg("showstation")) config.showStationName = (server.arg("showstation") == "1");
    if (server.hasArg("extra")) {
      config.extraServices = server.arg("extra").toInt();
      if (config.extraServices < 1) config.extraServices = 1;
      if (config.extraServices > 3) config.extraServices = 3;
    }
    if (server.hasArg("scrollspeed")) config.scrollSpeed = server.arg("scrollspeed").toInt();
    if (server.hasArg("rotationspeed")) {
      config.rotationSpeed = server.arg("rotationspeed").toInt();
      if (config.rotationSpeed < 5) config.rotationSpeed = 5;
      if (config.rotationSpeed > 60) config.rotationSpeed = 60;
    }
    if (server.hasArg("y1")) config.yPos1st = server.arg("y1").toInt();
    if (server.hasArg("y2")) config.yPos2nd = server.arg("y2").toInt();
    if (server.hasArg("y3")) config.yPosAlt = server.arg("y3").toInt();
    
    config.save();
    
    // ← NEW: Notify WebSocket clients before restart
    broadcastStatus("Device restarting - settings saved", "warning");
    
    String html = FPSTR(SAVE_SUCCESS_PAGE);
    server.send(200, "text/html", html);
    delay(2000);
    ESP.restart();
  });

  server.on("/apply", HTTP_POST, []() {
    String oldSSID = String(config.wifiSSID);
    String oldPassword = String(config.wifiPassword);
    String oldStation = String(config.stationCode);
    bool oldCallingAt = config.useCallingAt;
    int oldExtraServices = config.extraServices;
    
    if (server.hasArg("ssid")) {
      strncpy(config.wifiSSID, server.arg("ssid").c_str(), sizeof(config.wifiSSID) - 1);
      config.wifiSSID[sizeof(config.wifiSSID) - 1] = '\0';
    }
    if (server.hasArg("password") && !server.arg("password").isEmpty()) {
      strncpy(config.wifiPassword, server.arg("password").c_str(), sizeof(config.wifiPassword) - 1);
      config.wifiPassword[sizeof(config.wifiPassword) - 1] = '\0';
    }
    if (server.hasArg("station")) {
      String station = server.arg("station");
      station.trim();  // Remove any whitespace
      station.toUpperCase();
      // Extract only the 3-letter code if longer string provided
      if (station.length() >= 3) {
        station = station.substring(0, 3);
      }
      // Only apply if we have exactly 3 characters
      if (station.length() == 3) {
        strncpy(config.stationCode, station.c_str(), sizeof(config.stationCode) - 1);
        config.stationCode[3] = '\0';  // Ensure null termination
      }
    }
    if (server.hasArg("interval")) config.refreshInterval = server.arg("interval").toInt();
    if (server.hasArg("mode")) config.useCallingAt = (server.arg("mode") == "1");
    if (server.hasArg("showstation")) config.showStationName = (server.arg("showstation") == "1");
    if (server.hasArg("extra")) {
      config.extraServices = server.arg("extra").toInt();
      if (config.extraServices < 1) config.extraServices = 1;
      if (config.extraServices > 3) config.extraServices = 3;
    }
    if (server.hasArg("scrollspeed")) config.scrollSpeed = server.arg("scrollspeed").toInt();
    if (server.hasArg("rotationspeed")) {
      config.rotationSpeed = server.arg("rotationspeed").toInt();
      if (config.rotationSpeed < 5) config.rotationSpeed = 5;
      if (config.rotationSpeed > 60) config.rotationSpeed = 60;
    }
    if (server.hasArg("y1")) config.yPos1st = server.arg("y1").toInt();
    if (server.hasArg("y2")) config.yPos2nd = server.arg("y2").toInt();
    if (server.hasArg("y3")) config.yPosAlt = server.arg("y3").toInt();
    
    // Initialize rotation index based on display mode
    if (config.useCallingAt) {
      if (!config.showStationName) {
        currentAlternatingService = 2;  // Calling at mode, station hidden: start from service[2]
      } else {
        currentAlternatingService = 1;  // Calling at mode, station shown: start from service[1]
      }
    } else if (!config.showStationName) {
      currentAlternatingService = 3;  // Standard mode, station hidden: start from service[3]
    } else {
      currentAlternatingService = 2;  // Standard mode, station shown: start from service[2]
    }
    callingAtScrollOffset = 0;
    
    config.save();
    
    bool stationChanged = (oldStation != String(config.stationCode));
    bool displayModeChanged = (oldCallingAt != config.useCallingAt) || (oldExtraServices != config.extraServices);
    bool switchedToCallingAt = (!oldCallingAt && config.useCallingAt);

    // Clear calling points when switching TO calling at mode
    // This ensures we fetch fresh data with the detailed API endpoint
    // The display will show "Loading stops..." until data arrives
    if (switchedToCallingAt) {
      for (int i = 0; i < 6; i++) {
        services[i].callingPoints[0] = '\0';
      }
      // Force immediate fetch to get calling points
      if (fetchState != FETCH_IDLE) {
        fetchClient.stop();
        fetchState = FETCH_IDLE;
      }
      lastSuccessfulFetch = 0;
      lastFetchAttempt = 0;
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
      serviceCount = 0;
      fetchingNewStation = true;  // Mark that we're loading new station data
      
      if (fetchState != FETCH_IDLE) {
        fetchClient.stop();
        fetchState = FETCH_IDLE;
      }
      
      lastSuccessfulFetch = 0;
      lastFetchAttempt = 0;
      
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
    json += "\"stationName\":\"" + String(stationName) + "\",";
    json += "\"services\":" + String(serviceCount) + ",";
    json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    json += "\"uptime\":" + String(millis() / 1000) + ",";
    json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"firmwareVersion\":\"" + config.firmwareVersion + "\",";
    json += "\"deviceId\":\"" + config.deviceId + "\",";
    json += "\"useCallingAt\":" + String(config.useCallingAt ? "true" : "false") + ",";
    json += "\"extraServices\":" + String(config.extraServices) + ",";
    json += "\"rotationSpeed\":" + String(config.rotationSpeed) + ",";
    json += "\"lastUpdate\":" + String(lastSuccessfulFetch / 1000);
    json += "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("✅ HTTP server started on port 80");
  
  // ← NEW: Start WebSocket server
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
    systemError = true;
    return;
  }
  
  // Check for first boot
  firstBoot = checkFirstBoot();
  
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
  if (firstBoot) {
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
  
  // ← MONITORING: Connect to monitoring server
  if (monitoringEnabled && !apMode) {
    Serial.println("🔌 Connecting to monitoring server...");
    Serial.println("   Host: " + monitorServerHost + ":" + String(monitorServerPort));
    monitorClient.begin(monitorServerHost, monitorServerPort, "/ws");  // Plain WebSocket path
    monitorClient.onEvent(monitorWebSocketEvent);
    monitorClient.setReconnectInterval(5000);
    delay(500);
  }
  
  // Initial data fetch will happen automatically in loop
}

void loop() {
  server.handleClient();
  webSocket.loop();  // ← NEW: Handle WebSocket events
  
  ArduinoOTA.handle();

  if (systemError) {
    handleSystemError();
    return;
  }

  if (apMode) {
    static unsigned long lastAPUpdate = 0;
    if (millis() - lastAPUpdate > 30000) {
      displayAPScreen();
      lastAPUpdate = millis();
    }
    delay(100);
    return;
  }

  unsigned long currentTime = millis();

  // Fetch with regular intervals
  if (fetchState == FETCH_IDLE) {
  unsigned long timeSinceLastSuccess = currentTime - lastSuccessfulFetch;
  unsigned long timeSinceLastAttempt = currentTime - lastFetchAttempt;
  
  bool shouldFetch = (timeSinceLastSuccess >= config.refreshInterval * 1000UL);
  bool enoughTimeSinceAttempt = (timeSinceLastAttempt >= 30000UL);
  bool forceFetch = (lastFetchAttempt == 0 && lastSuccessfulFetch == 0); // New condition
  
  if ((shouldFetch && enoughTimeSinceAttempt) || forceFetch) {
    if (WiFi.status() == WL_CONNECTED) {
      lastFetchAttempt = currentTime;
      if (!asyncFetchStart()) {
        displayStatus("ERR");
      }
    } else {
      Serial.println("❌ WiFi disconnected");
      displayStatus("OFF");
      lastFetchAttempt = currentTime;
      if (!initializeWiFi()) startAccessPoint();
    }
  }
}

  handleFetchStateMachine();
  handleAlternatingService(currentTime);
  updateDisplay();  // CRITICAL: Called every loop for smooth animations and clock updates!
  
  // ← MONITORING: Handle monitoring server connection
  if (monitoringEnabled) {
    monitorClient.loop();
    
    // Send heartbeat to monitoring server
    if (currentTime - lastMonitorHeartbeat >= MONITOR_HEARTBEAT_INTERVAL) {
      sendMonitorHeartbeat();
      lastMonitorHeartbeat = currentTime;
    }
  }
  
  // ← NEW: Broadcast metrics periodically
  if (currentTime - lastMetricsBroadcast >= 10000) {  // Every 10 seconds
    broadcastMetrics();
    lastMetricsBroadcast = currentTime;
  }

  // Broadcast display snapshot for live preview
  if (currentTime - lastDisplaySnapshot >= 2000) {  // Every 2 seconds
    broadcastDisplaySnapshot();
    lastDisplaySnapshot = currentTime;
  }
  
  delay(20);
}

// NOTE: In your actual parseAndDisplayResponse function, add this after successful parse:
// broadcastTrainUpdate();  // ← Notify WebSocket clients of new data
// broadcastStatus("Train data updated", "success");

// NOTE: When fetch fails, add:
// broadcastStatus("Failed to fetch train data", "error");

// NOTE: When fetch starts, add:
// broadcastStatus("Fetching train data...", "info");