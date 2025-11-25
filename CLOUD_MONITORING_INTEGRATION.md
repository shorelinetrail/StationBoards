# Cloud Monitoring Integration Guide

## Overview

This document explains how the **local backend monitoring system** integrates with the **cloud monitoring dashboard** at stationboards.co.uk.

---

## Architecture

### Complete Data Flow

```
┌─────────────┐     WebSocket      ┌──────────────────┐     HTTPS POST     ┌─────────────────┐
│             │ ──────────────────> │                  │ ─────────────────> │                 │
│  ESP32      │                     │  Backend Server  │                    │  Cloud API      │
│  Board      │ <────────────────── │  (Local PC/RPi)  │ <───────────────── │  (Website)      │
│             │   Config/Commands   │                  │    Success/Error   │                 │
└─────────────┘                     └──────────────────┘                    └─────────────────┘
   Customer's                          Customer's                              stationboards
   Home WiFi                            Local Network                          .co.uk
```

### Step-by-Step Flow

1. **ESP32 Board** sends heartbeat via WebSocket to local backend
   ```
   ws://192.168.1.100:3000/ws
   ```

2. **Backend Server** receives heartbeat and:
   - Updates local SQLite database
   - Updates local dashboard (Socket.IO)
   - **Forwards to cloud** (HTTPS POST)

3. **Cloud API** receives heartbeat and:
   - Updates board `last_seen` timestamp
   - Updates board status (shipped → active)
   - Tracks IP address
   - Logs first activation
   - Shows board as online in admin dashboard

---

## Why This Architecture?

### Problem: ESP32 Boards Behind Home Routers

ESP32 boards are on customer home networks behind NAT/routers. They **cannot** directly reach the internet without:
- Port forwarding (security risk)
- UPnP (unreliable)
- VPN (complex)

### Solution: Local Backend as Bridge

The backend server:
- ✅ Runs on customer's local network
- ✅ ESP32 boards connect via local IP (no internet needed for basic operation)
- ✅ Backend server has internet access (standard for PCs/Raspberry Pis)
- ✅ Backend forwards heartbeats to cloud securely via HTTPS

### Benefits

**For Customers:**
- Works behind any router (no port forwarding needed)
- No security risks
- Works offline (local monitoring continues)
- Low latency for local dashboard

**For Admin:**
- Real-time board status on website
- Customer service visibility
- Remote troubleshooting
- Fleet management

---

## Setup Instructions

### 1. Backend Server Configuration

#### Option A: Environment Variable (Recommended)

Set the cloud monitoring URL as an environment variable:

**Windows:**
```cmd
set CLOUD_MONITORING_URL=https://stationboards.co.uk/api/board-heartbeat
npm start
```

**Linux/Mac:**
```bash
export CLOUD_MONITORING_URL=https://stationboards.co.uk/api/board-heartbeat
npm start
```

**Railway.app (for hosted backend):**
```
Dashboard → Variables → Add Variable
Name: CLOUD_MONITORING_URL
Value: https://stationboards.co.uk/api/board-heartbeat
```

#### Option B: .env File

Create `/backend/.env`:
```bash
CLOUD_MONITORING_URL=https://stationboards.co.uk/api/board-heartbeat
```

Then start normally:
```bash
npm start
```

#### Option C: Local Only (No Cloud)

Don't set `CLOUD_MONITORING_URL` at all. The backend will work in local-only mode.

```bash
npm start
# Console output: 📍 Cloud monitoring disabled (local only)
```

### 2. Verify Integration

When starting the backend with cloud monitoring enabled, you should see:

```
✅ StationBoards Monitor Server Running
☁️  Cloud monitoring enabled: https://stationboards.co.uk/api/board-heartbeat
📊 Web Dashboard: http://localhost:3000
```

When a board sends a heartbeat, you'll see:

```
✓ Device heartbeat: ESP32-A1B2C3D4
☁️  Cloud heartbeat sent for ESP32-A1B2C3D4
```

### 3. Check Cloud Dashboard

1. Go to https://stationboards.co.uk/admin
2. Login with admin credentials
3. Click "Boards" tab
4. Find your board (must exist in database first)
5. Check "Last Seen" column
6. Should show "Just now" with green ● indicator

---

## How It Works Internally

### Backend Server (server.js)

```javascript
// Configuration
const CLOUD_MONITORING_URL = process.env.CLOUD_MONITORING_URL || '';
const CLOUD_MONITORING_ENABLED = !!CLOUD_MONITORING_URL;

// Forward heartbeat to cloud
async function forwardHeartbeatToCloud(deviceId) {
  if (!CLOUD_MONITORING_ENABLED) return;

  const data = JSON.stringify({ board_id: deviceId });

  // HTTPS POST to cloud API
  const req = https.request({
    hostname: 'stationboards.co.uk',
    path: '/api/board-heartbeat',
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Content-Length': data.length
    }
  }, handleResponse);

  req.write(data);
  req.end();
}

// Called from heartbeat handler
function handleDeviceHeartbeat(data) {
  // 1. Update local database
  db.run(updateSQL, values);

  // 2. Forward to cloud (if enabled)
  forwardHeartbeatToCloud(data.deviceId);

  // 3. Broadcast to local dashboard
  io.emit('deviceUpdate', data);
}
```

### Cloud API (/api/board-heartbeat.js)

```javascript
// Receives POST request from backend server
// Updates board in Supabase database
// Returns success status
const { board_id } = req.body;

await supabase.rpc('board_heartbeat', {
  p_board_id: board_id,
  p_ip: clientIp
});

return { success: true, board_id, timestamp: new Date() };
```

### Database Function (Supabase)

```sql
CREATE FUNCTION board_heartbeat(p_board_id TEXT, p_ip TEXT)
RETURNS JSON AS $$
BEGIN
  -- Update board
  UPDATE boards
  SET
    last_seen = NOW(),
    last_ip = p_ip,
    activated_at = COALESCE(activated_at, NOW()),
    status = CASE
      WHEN status = 'shipped' THEN 'active'
      WHEN status = 'assigned' THEN 'active'
      ELSE status
    END
  WHERE board_id = p_board_id;

  -- Return success
  RETURN json_build_object('success', true);
END;
$$ LANGUAGE plpgsql;
```

---

## Troubleshooting

### Backend Server Shows "Cloud monitoring disabled"

**Problem:** `CLOUD_MONITORING_URL` environment variable not set

**Solution:**
```bash
# Set environment variable
export CLOUD_MONITORING_URL=https://stationboards.co.uk/api/board-heartbeat

# Restart backend
npm start
```

### Backend Logs "Cloud heartbeat network error"

**Problem:** Backend server cannot reach the internet

**Check:**
1. Internet connection working?
2. Firewall blocking outbound HTTPS?
3. DNS resolution working? (test: `ping stationboards.co.uk`)

### Backend Logs "Cloud heartbeat HTTP 400/500"

**Problem:** API rejecting request

**Check:**
1. Board exists in cloud database (add via admin dashboard)
2. Board ID matches exactly (case-sensitive)
3. API endpoint correct (https://stationboards.co.uk/api/board-heartbeat)

### Board Not Showing as Online in Cloud Dashboard

**Problem:** Heartbeat not reaching cloud or board doesn't exist

**Check:**
1. Backend server has `CLOUD_MONITORING_URL` set?
2. Backend logs show "Cloud heartbeat sent"?
3. Board exists in admin dashboard → Boards tab?
4. Add board if missing: Admin → Boards → Add Board
5. Wait 5 minutes (boards shown as online if seen < 5 minutes)

### Backend Logs "Cloud heartbeat parse error"

**Problem:** Cloud API returned invalid JSON

**Check:**
1. API endpoint correct?
2. Check website status (API might be down)
3. Check for redirects (HTTP → HTTPS)

---

## Heartbeat Frequency

### Recommended Settings

**ESP32 to Backend:** Every 30-60 seconds
- Fast enough for real-time monitoring
- Not too frequent to overwhelm backend

**Backend to Cloud:** Every heartbeat (real-time)
- Forwarded immediately when received
- No batching needed
- Minimal bandwidth usage

### ESP32 Firmware Configuration

```cpp
// In main.cpp
const unsigned long HEARTBEAT_INTERVAL = 60000; // 60 seconds

void loop() {
  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    sendHeartbeat();
    lastHeartbeat = millis();
  }
}
```

---

## Security Considerations

### Authentication

Currently, the cloud API endpoint (`/api/board-heartbeat`) accepts anonymous requests to allow boards to report status.

**Security measures in place:**
1. Board must exist in database (pre-registered)
2. Only `last_seen` and `status` can be updated
3. RLS policies prevent data leaks
4. IP address logged for audit trail
5. Rate limiting on API endpoint

### Future Enhancements (Optional)

1. **API Key Authentication:**
   ```javascript
   headers: {
     'Authorization': `Bearer ${CLOUD_API_KEY}`
   }
   ```

2. **Board-Specific Tokens:**
   Each board gets unique token for authentication

3. **Certificate Pinning:**
   Verify SSL certificate to prevent MITM attacks

---

## Monitoring Dashboard Features

### Local Dashboard (Backend)

**URL:** http://localhost:3000

**Features:**
- Real-time device list
- Online/offline status
- WiFi signal strength (RSSI)
- Memory usage, uptime
- Live display preview
- Remote configuration
- Command execution
- Event logs

### Cloud Dashboard (Website)

**URL:** https://stationboards.co.uk/admin → Boards tab

**Features:**
- Fleet overview (all customer boards)
- Online/offline indicators
- Last seen timestamps
- IP address tracking
- First activation tracking
- Board edit functionality
- Order integration
- Customer support view

---

## Testing the Integration

### Test Script

The test script simulates the complete flow:

```bash
# 1. Start backend server with cloud monitoring
cd backend
CLOUD_MONITORING_URL=https://stationboards.co.uk/api/board-heartbeat npm start

# 2. Connect an ESP32 board
# (Board will send heartbeat via WebSocket)

# 3. Check backend logs for:
✓ Device heartbeat: ESP32-TEST001
☁️  Cloud heartbeat sent for ESP32-TEST001

# 4. Check cloud dashboard:
# Admin → Boards → Look for ESP32-TEST001
# Should show "Just now" with green ● indicator
```

### Manual Test (Without ESP32)

You can manually test the cloud forwarding:

```bash
# From backend directory
node -e "
const https = require('https');
const data = JSON.stringify({ board_id: 'ESP32-TEST001' });
const req = https.request({
  hostname: 'stationboards.co.uk',
  path: '/api/board-heartbeat',
  method: 'POST',
  headers: {
    'Content-Type': 'application/json',
    'Content-Length': data.length
  }
}, (res) => {
  let body = '';
  res.on('data', (chunk) => body += chunk);
  res.on('end', () => console.log('Response:', body));
});
req.write(data);
req.end();
"
```

Expected output:
```json
{
  "success": true,
  "board_id": "ESP32-TEST001",
  "status": "active",
  "first_activation": false,
  "timestamp": "2025-01-24T12:34:56Z"
}
```

---

## Deployment Scenarios

### Scenario 1: Customer Home Network (Most Common)

```
┌─────────────────────────────────────────────┐
│         Customer Home Network               │
│                                             │
│  ┌──────────┐      ┌────────────────────┐  │
│  │  ESP32   │──────│  Windows PC/Mac    │  │
│  │  Board   │ WiFi │  Running Backend   │──┼─→ Internet → Cloud
│  └──────────┘      └────────────────────┘  │
│                                             │
└─────────────────────────────────────────────┘
```

**Setup:**
- Backend runs on customer's PC/Mac
- ESP32 connects to backend via local IP
- Backend forwards to cloud

### Scenario 2: Raspberry Pi (Headless)

```
┌─────────────────────────────────────────────┐
│         Customer Home Network               │
│                                             │
│  ┌──────────┐      ┌────────────────────┐  │
│  │  ESP32   │──────│  Raspberry Pi      │  │
│  │  Board   │ WiFi │  Running Backend   │──┼─→ Internet → Cloud
│  └──────────┘      │  (Headless)        │  │
│                    └────────────────────┘  │
└─────────────────────────────────────────────┘
```

**Setup:**
- Backend runs on Raspberry Pi (systemd service)
- Always-on monitoring
- Accessed remotely via SSH or local web UI

### Scenario 3: Cloud-Hosted Backend (Railway/Heroku)

```
┌────────────────┐                    ┌──────────────────┐
│  Customer Home │                    │   Railway.app    │
│                │                    │                  │
│  ┌──────────┐  │   Internet (VPN)  │  ┌────────────┐  │
│  │  ESP32   │──┼───────────────────┼──│  Backend   │  │
│  └──────────┘  │                    │  │  Server    │  │
│                │                    │  └─────┬──────┘  │
└────────────────┘                    └────────┼─────────┘
                                               │
                                               ↓
                                      stationboards.co.uk
```

**Setup:**
- Backend hosted on Railway/Heroku
- ESP32 connects via internet (requires WireGuard or similar)
- More complex but centralized

**Note:** This scenario requires VPN setup for ESP32 boards, which is beyond the scope of this guide.

---

## Summary

✅ **The monitoring integration is now complete:**

1. **ESP32 boards** send heartbeats to **local backend** (WebSocket)
2. **Backend server** forwards to **cloud API** (HTTPS)
3. **Cloud dashboard** shows real-time board status
4. **Local dashboard** continues to work offline

**Configuration:** Set `CLOUD_MONITORING_URL` environment variable on backend server

**No changes needed for ESP32 firmware** - the forwarding is transparent to boards.
