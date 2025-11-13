# ESP32 Firmware Modifications for Remote Monitoring

## Changes Required to main.cpp

Add these modifications to your existing main.cpp file to enable remote monitoring:

### 1. Add WebSocket Client Library

At the top of main.cpp, add:
```cpp
#include <WebSocketsClient.h>
```

And in platformio.ini, add:
```ini
lib_deps = 
    ...existing libraries...
    links2004/WebSocketsClient@^2.3.6
```

### 2. Add Global Variables

After your existing global variables (around line 55), add:
```cpp
// Monitoring Server Configuration
String monitorServerHost = "192.168.1.100";  // Change to your PC's IP
int monitorServerPort = 3000;
bool monitoringEnabled = true;

WebSocketsClient webSocket;
unsigned long lastHeartbeat = 0;
const unsigned long HEARTBEAT_INTERVAL = 30000;  // 30 seconds
bool webSocketConnected = false;
```

### 3. Add WebSocket Event Handler

Add this function before setup():
```cpp
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("❌ Monitoring server disconnected");
      webSocketConnected = false;
      break;
      
    case WStype_CONNECTED:
      Serial.println("✅ Connected to monitoring server");
      webSocketConnected = true;
      
      // Register device
      {
        String registerMsg = "{\"type\":\"register\",\"data\":{";
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
        registerMsg += "}}";
        webSocket.sendTXT(registerMsg);
      }
      break;
      
    case WStype_TEXT:
      Serial.printf("📨 Received: %s\n", payload);
      handleServerCommand((char*)payload);
      break;
      
    case WStype_ERROR:
      Serial.println("❌ WebSocket error");
      break;
  }
}

void handleServerCommand(char* payload) {
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, payload);
  
  if (error) {
    Serial.println("❌ JSON parse error");
    return;
  }
  
  const char* cmd = doc["command"];
  
  if (strcmp(cmd, "restart") == 0) {
    Serial.println("🔄 Remote restart requested");
    delay(1000);
    ESP.restart();
  }
  else if (strcmp(cmd, "updateConfig") == 0) {
    Serial.println("⚙️ Remote config update");
    JsonObject data = doc["data"];
    
    if (data.containsKey("stationCode")) {
      String station = data["stationCode"].as<String>();
      station.toUpperCase();
      strncpy(config.stationCode, station.c_str(), sizeof(config.stationCode) - 1);
    }
    if (data.containsKey("refreshInterval")) {
      config.refreshInterval = data["refreshInterval"];
    }
    if (data.containsKey("useCallingAt")) {
      config.useCallingAt = data["useCallingAt"];
    }
    if (data.containsKey("showStationName")) {
      config.showStationName = data["showStationName"];
    }
    if (data.containsKey("extraServices")) {
      config.extraServices = data["extraServices"];
    }
    if (data.containsKey("scrollSpeed")) {
      config.scrollSpeed = data["scrollSpeed"];
    }
    if (data.containsKey("rotationSpeed")) {
      config.rotationSpeed = data["rotationSpeed"];
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
  }
  else if (strcmp(cmd, "otaUpdate") == 0) {
    Serial.println("📦 OTA update requested");
    const char* url = doc["url"];
    
    // Implement OTA update
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
        Serial.println("⚠️ No updates");
        break;
        
      case HTTP_UPDATE_OK:
        Serial.println("✅ OTA complete");
        break;
    }
  }
}

void sendHeartbeat() {
  if (!webSocketConnected) return;
  
  String heartbeatMsg = "{\"type\":\"heartbeat\",\"data\":{";
  heartbeatMsg += "\"deviceId\":\"" + config.deviceId + "\",";
  heartbeatMsg += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  heartbeatMsg += "\"firmwareVersion\":\"" + config.firmwareVersion + "\",";
  heartbeatMsg += "\"stationCode\":\"" + String(config.stationCode) + "\",";
  heartbeatMsg += "\"stationName\":\"" + String(stationName) + "\",";
  heartbeatMsg += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  heartbeatMsg += "\"uptime\":" + String(millis() / 1000) + ",";
  heartbeatMsg += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  heartbeatMsg += "\"services\":" + String(serviceCount);
  heartbeatMsg += "}}";
  
  webSocket.sendTXT(heartbeatMsg);
}
```

### 4. Initialize WebSocket in setup()

In your setup() function, after WiFi is connected and before the main loop, add:
```cpp
  // Initialize monitoring connection
  if (monitoringEnabled && WiFi.status() == WL_CONNECTED) {
    Serial.println("🔌 Connecting to monitoring server...");
    webSocket.begin(monitorServerHost, monitorServerPort, "/socket.io/?EIO=4&transport=websocket");
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
    delay(1000);
  }
```

### 5. Add to loop()

In your main loop() function, add these calls:
```cpp
  // Handle WebSocket
  if (monitoringEnabled) {
    webSocket.loop();
    
    // Send heartbeat
    if (millis() - lastHeartbeat >= HEARTBEAT_INTERVAL) {
      sendHeartbeat();
      lastHeartbeat = millis();
    }
  }
```

### 6. Add to platformio.ini

Add HTTPUpdate library:
```ini
lib_deps = 
    ...existing libraries...
    links2004/WebSocketsClient@^2.3.6
```

And at the top of main.cpp:
```cpp
#include <HTTPUpdate.h>
```

### 7. Configuration via Web Interface

Add these fields to your web configuration page:
```javascript
// In your config page HTML, add:
<div class="mb-3">
  <label class="form-label">Monitoring Server</label>
  <input type="text" class="form-control" name="monitorHost" 
         value="{MONITOR_HOST}" placeholder="192.168.1.100">
</div>
<div class="mb-3">
  <label class="form-label">Enable Monitoring</label>
  <select class="form-select" name="monitorEnabled">
    <option value="1" {MONITOR_ENABLED}>Enabled</option>
    <option value="0" {MONITOR_DISABLED}>Disabled</option>
  </select>
</div>
```

### 8. Important Notes

- Replace `192.168.1.100` with your Windows PC's actual IP address
- The monitoring server must be running and accessible from the ESP32
- Make sure firewall allows connections on port 3000
- WebSocket reconnects automatically if connection is lost
