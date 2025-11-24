# ESP32 Board Integration Guide

## Overview

StationBoards now includes a monitoring system that tracks when boards are online and their health status. When your ESP32 board boots up and periodically during operation, it should send heartbeat requests to the server.

---

## Heartbeat API

### Endpoint

```
POST https://www.stationboards.co.uk/api/board-heartbeat
```

### Request Format

```json
{
  "board_id": "ESP32-12345678"
}
```

### Response Format

**Success (200 OK):**
```json
{
  "success": true,
  "board_id": "ESP32-12345678",
  "status": "active",
  "first_activation": false,
  "timestamp": "2025-01-24T12:34:56.789Z"
}
```

**Error (400 Bad Request):**
```json
{
  "success": false,
  "error": "Missing or invalid board_id"
}
```

**Error (500 Internal Server Error):**
```json
{
  "success": false,
  "error": "Database error",
  "details": "..."
}
```

---

## ESP32 Arduino Example

### Using ESP8266HTTPClient

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Your board's unique ID (ESP32 chip ID)
String boardId;

void setup() {
  Serial.begin(115200);

  // Get ESP32 chip ID
  uint64_t chipid = ESP.getEfuseMac();
  boardId = "ESP32-" + String((uint32_t)(chipid >> 32), HEX) + String((uint32_t)chipid, HEX);
  boardId.toUpperCase();

  Serial.println("Board ID: " + boardId);

  // Connect to WiFi (your WiFi code here)
  connectToWiFi();

  // Send initial heartbeat
  sendHeartbeat();
}

void loop() {
  // Send heartbeat every 5 minutes
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat >= 300000) { // 5 minutes
    sendHeartbeat();
    lastHeartbeat = millis();
  }

  // Your main loop code here...
}

void sendHeartbeat() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, skipping heartbeat");
    return;
  }

  HTTPClient http;

  // Configure request
  http.begin("https://www.stationboards.co.uk/api/board-heartbeat");
  http.addHeader("Content-Type", "application/json");

  // Build JSON payload
  StaticJsonDocument<200> doc;
  doc["board_id"] = boardId;

  String payload;
  serializeJson(doc, payload);

  // Send POST request
  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("Heartbeat response: " + response);

    if (httpCode == 200) {
      // Parse response
      StaticJsonDocument<300> responseDoc;
      deserializeJson(responseDoc, response);

      bool success = responseDoc["success"];
      bool firstActivation = responseDoc["first_activation"];

      if (success) {
        Serial.println("Heartbeat successful");
        if (firstActivation) {
          Serial.println("🎉 Board activated for the first time!");
          // Maybe show a special animation on the OLED?
        }
      }
    }
  } else {
    Serial.println("Heartbeat failed: " + String(httpCode));
  }

  http.end();
}

void connectToWiFi() {
  const char* ssid = "YOUR_WIFI_SSID";
  const char* password = "YOUR_WIFI_PASSWORD";

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
```

---

## Heartbeat Timing Recommendations

### Boot Sequence
1. **Immediate**: Send heartbeat within 30 seconds of boot
2. **Retry**: If first heartbeat fails, retry after 1 minute
3. **Regular**: Once connected, send heartbeat every 5 minutes

### Frequency Guidelines

| Scenario | Heartbeat Interval | Reason |
|----------|-------------------|---------|
| **Normal operation** | 5 minutes | Efficient, low bandwidth |
| **Testing/Development** | 1 minute | Quick feedback |
| **Production** | 5-10 minutes | Balanced monitoring |
| **Low power mode** | 15-30 minutes | Battery conservation |

### Offline Detection

The admin dashboard considers a board **offline** if no heartbeat received in the last 5 minutes.

---

## What Happens When Board Sends Heartbeat

### First Heartbeat (Board Activation)
1. Board status changes from `shipped` → `active`
2. `activated_at` timestamp recorded
3. `last_seen` timestamp updated
4. Customer order history logs "board_activated" event
5. Admin dashboard shows board as online (green ●)

### Subsequent Heartbeats
1. `last_seen` timestamp updated
2. `last_ip` address recorded
3. Admin dashboard shows board online
4. No database events logged (just timestamp update)

---

## Board Status Lifecycle

```
in_stock → assigned → shipped → active
                              ↓
                           (board sends first heartbeat)
```

**Status Details:**
- `in_stock`: Board in inventory, not assigned to any order
- `assigned`: Board assigned to an order but not yet shipped
- `shipped`: Order has been shipped to customer
- `active`: Board is powered on and connected (first heartbeat received)

---

## Error Handling

### Common Errors

**1. Board ID Not Found**
```json
{
  "success": false,
  "error": "Board not found",
  "board_id": "ESP32-UNKNOWN"
}
```

**Solution**: Ensure board ID is registered in admin dashboard

**2. Network Timeout**
- Check WiFi connection
- Verify DNS resolution
- Check firewall/proxy settings

**3. SSL Certificate Issues**
- Ensure ESP32 has root CA certificates
- Use `http.setInsecure()` for testing (NOT for production)

### Retry Logic Example

```cpp
bool sendHeartbeatWithRetry(int maxRetries = 3) {
  for (int attempt = 0; attempt < maxRetries; attempt++) {
    if (sendHeartbeat()) {
      return true;
    }

    Serial.println("Heartbeat failed, retry " + String(attempt + 1));
    delay(5000); // Wait 5 seconds before retry
  }

  return false;
}
```

---

## Testing Your Integration

### 1. Test in Admin Dashboard

1. Login to admin: `https://www.stationboards.co.uk/admin`
2. Go to **Boards** tab
3. Add test board: `ESP32-TEST001`
4. Power on your ESP32 with that board ID
5. Watch for board status change from `in_stock` → `active`
6. Check "Online" column shows green ●
7. Verify "Last Seen" shows "Just now"

### 2. Monitor Serial Output

```
Board ID: ESP32-1A2B3C4D
Connecting to WiFi...
WiFi connected!
IP address: 192.168.1.100
Sending heartbeat...
Heartbeat response: {"success":true,"board_id":"ESP32-1A2B3C4D","status":"active","first_activation":true,"timestamp":"2025-01-24T12:34:56.789Z"}
Heartbeat successful
🎉 Board activated for the first time!
```

### 3. Test Offline Detection

1. Disconnect board power
2. Wait 6 minutes
3. Refresh admin dashboard
4. Online indicator should show gray ○
5. Last Seen should show "6m ago"

---

## Production Recommendations

### 1. Secure Connection
Always use HTTPS in production. Include root CA certificate:

```cpp
// Root CA certificate for Let's Encrypt (used by Vercel)
const char* rootCACertificate = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFFjCCAv6gAwIBAgIRAJErCErPDBinU/bWLiWnX1owDQYJKoZIhvcNAQELBQAw
...
-----END CERTIFICATE-----
)EOF";

http.begin("https://www.stationboards.co.uk/api/board-heartbeat", rootCACertificate);
```

### 2. Handle WiFi Reconnection

```cpp
void ensureWiFiConnected() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, reconnecting...");
    WiFi.reconnect();

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      attempts++;
    }
  }
}
```

### 3. Watchdog Timer

Implement a watchdog to reboot if heartbeat fails for too long:

```cpp
#include <esp_task_wdt.h>

void setup() {
  // Enable watchdog (10 minutes)
  esp_task_wdt_init(600, true);
  esp_task_wdt_add(NULL);
}

void loop() {
  // Reset watchdog after successful heartbeat
  if (sendHeartbeat()) {
    esp_task_wdt_reset();
  }
}
```

### 4. Local Health Check

Before sending heartbeat, verify board is healthy:

```cpp
bool isBoardHealthy() {
  // Check display is working
  if (!displayTest()) return false;

  // Check WiFi signal strength
  if (WiFi.RSSI() < -80) return false;

  // Check free heap
  if (ESP.getFreeHeap() < 10000) return false;

  return true;
}

void sendHeartbeat() {
  if (!isBoardHealthy()) {
    Serial.println("Board health check failed, skipping heartbeat");
    return;
  }

  // Send heartbeat...
}
```

---

## Monitoring Dashboard Features

### Admin View

**Boards Tab:**
- Online/offline status (● indicator)
- Last seen timestamp (Just now, 5m ago, etc.)
- Board ID, firmware version, hardware revision
- Assigned order number

**Order Detail:**
- Shows which boards are assigned to each order
- Real-time online status for each board
- Customer can see their board is working

### Customer View (Coming Soon)

Public tracking page where customers can:
- Enter order number + email
- See board status (shipped/active)
- View last online timestamp
- Confirm board is working before arrival

---

## Troubleshooting

### Board Not Showing as Online

**Check:**
1. Board ID matches database (`SELECT * FROM boards WHERE board_id = 'ESP32-...'`)
2. Heartbeat request succeeds (check Serial monitor)
3. Less than 5 minutes since last heartbeat
4. Board status is not `faulty` or `returned`

### Heartbeat Succeeds but Dashboard Shows Offline

**Solution:**
- Refresh admin dashboard (F5)
- Check browser console for JavaScript errors
- Verify `last_seen` column in database is updating

### First Activation Not Detected

**Check:**
1. Board must have `status = 'shipped'` or be assigned to an order
2. `activated_at` must be NULL before first heartbeat
3. Check order_history table for 'board_activated' event

---

## Future Enhancements

### Planned Features

1. **Extended Telemetry**
   - WiFi signal strength (RSSI)
   - Free heap memory
   - Uptime
   - Display errors
   - API call counts

2. **Alerts**
   - Email when board goes offline for >1 hour
   - Low WiFi signal warnings
   - Crash detection (expected heartbeat missed)

3. **Health Metrics**
   - Uptime percentage
   - Average response time
   - Connection reliability score

4. **Remote Commands**
   - Restart board
   - Update display content
   - Change WiFi credentials
   - Push firmware updates

---

## Support

If you encounter issues with board monitoring:

1. **Check Logs**: Serial monitor output
2. **Verify API**: Use curl to test heartbeat endpoint
3. **Database**: Check Supabase dashboard
4. **Contact**: hello@stationboards.co.uk

---

## Example: Complete Arduino Sketch

See `/firmware/stationboard-monitor/stationboard-monitor.ino` for a complete working example with:
- WiFi management
- Heartbeat with retry logic
- OLED display updates
- National Rail API integration
- Error handling
- Watchdog timer

---

**Monitoring System Status:** ✅ Production Ready

Last Updated: 2025-01-24
