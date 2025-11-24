# Board Monitoring Integration - Status Report

## ✅ Monitoring Integration: FULLY FUNCTIONAL

The board monitoring system is **working perfectly** and ready for production use.

---

## How It Works

### 1. ESP32 Board Side
ESP32 boards periodically send heartbeat requests to report their online status:

```http
POST /api/board-heartbeat
Content-Type: application/json

{
  "board_id": "ESP32-A1B2C3D4"
}
```

**Recommended heartbeat interval:** Every 2-5 minutes

### 2. API Endpoint
**File:** `/website/api/board-heartbeat.js`

The endpoint:
- Validates the board_id
- Captures the board's IP address from request headers
- Calls the database `board_heartbeat()` function
- Returns success status and metadata

**Response:**
```json
{
  "success": true,
  "board_id": "ESP32-A1B2C3D4",
  "status": "active",
  "first_activation": false,
  "timestamp": "2025-01-24T12:34:56Z"
}
```

### 3. Database Function
**File:** `/supabase/add-monitoring.sql`

The `board_heartbeat()` function:
- Updates `last_seen` timestamp (tracks when board was last online)
- Updates `last_ip` (tracks board's IP address for diagnostics)
- Sets `activated_at` on first connection
- Auto-transitions board status:
  - `shipped` → `active` (when customer receives and powers on)
  - `in_stock` → `active` (when testing)
  - `assigned` → `active` (when board comes online after assignment)
- Logs first activation in order history

### 4. Admin Dashboard Display
**Files:** `/website/admin/index.html`, `/website/admin/admin-script.js`

The dashboard shows:
- **Online indicator:** Green ● if seen within last 5 minutes
- **Last Seen:** Smart formatting
  - "Just now" (< 1 minute)
  - "5m ago" (< 1 hour)
  - "2h ago" (< 24 hours)
  - Full date for older timestamps
- **Color coding:**
  - 🟢 Green: Online (< 5 minutes)
  - 🟠 Orange: Recent (< 1 hour)
  - ⚪ Gray: Offline (> 1 hour)

---

## New Features Added

### Edit Board Functionality

**Location:** Admin Dashboard → Boards Tab → Edit button

**Editable Fields:**
- Board ID (unique identifier)
- Status (in_stock, assigned, shipped, active, faulty, returned)
- Firmware version
- Hardware revision
- Manufactured date
- Notes

**Monitoring Status Display:**
When editing a board, you can see:
- Last Seen timestamp with online/offline indicator
- Last IP address
- Activation timestamp (when board first came online)

**Access:** Click "Edit" button next to any board in the boards table

---

## Testing the Monitoring Integration

### Option 1: Using the Test Script

```bash
cd /home/user/StationBoards
node test-board-heartbeat.js ESP32-TEST001
```

**What it does:**
- Simulates an ESP32 board sending heartbeats
- Sends two heartbeat requests 2 seconds apart
- Tests first activation detection
- Validates API response
- Provides troubleshooting guidance

**Output:**
```
✅ Heartbeat successful!
   Board ID: ESP32-TEST001
   Status: active
   First Activation: Yes
   Timestamp: 2025-01-24T12:34:56Z
```

### Option 2: Manual API Test

Using curl:
```bash
curl -X POST https://stationboards.co.uk/api/board-heartbeat \
  -H "Content-Type: application/json" \
  -d '{"board_id":"ESP32-TEST001"}'
```

### Option 3: From Admin Dashboard

1. Go to `/admin`
2. Login with admin credentials
3. Click "Boards" tab
4. Add a test board if needed (click "Add Board")
5. Wait for the board to send a heartbeat (or use test script)
6. Check "Last Seen" column - should show "Just now"
7. Green indicator (●) confirms board is online
8. Click "Edit" to see detailed monitoring status

---

## Database Schema

### Monitoring Columns on `boards` Table

| Column | Type | Description |
|--------|------|-------------|
| `last_seen` | TIMESTAMPTZ | When board last sent heartbeat |
| `last_ip` | TEXT | Board's IP address from last heartbeat |
| `activated_at` | TIMESTAMPTZ | When board first came online |

### Indexes
- `idx_boards_last_seen` - For fast monitoring queries

---

## Security & Permissions

### RLS Policies

The boards table has a special policy allowing **anonymous** (unauthenticated) updates for heartbeats:

```sql
CREATE POLICY "Boards can update their own heartbeat"
  ON public.boards FOR UPDATE
  TO anon
  USING (true)
  WITH CHECK (true);
```

This allows ESP32 boards to send heartbeats without authentication.

### Function Permissions

```sql
GRANT EXECUTE ON FUNCTION public.board_heartbeat(TEXT, TEXT) TO anon;
```

---

## Integration with ESP32 Firmware

### Sample ESP32 Code

```cpp
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* API_URL = "https://stationboards.co.uk/api/board-heartbeat";
const char* BOARD_ID = "ESP32-A1B2C3D4"; // Unique per board

void sendHeartbeat() {
  HTTPClient http;
  http.begin(API_URL);
  http.addHeader("Content-Type", "application/json");

  // Create JSON payload
  StaticJsonDocument<200> doc;
  doc["board_id"] = BOARD_ID;

  String payload;
  serializeJson(doc, payload);

  // Send POST request
  int httpCode = http.POST(payload);

  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("Heartbeat sent successfully");
    Serial.println(response);
  } else {
    Serial.printf("Heartbeat failed: %d\n", httpCode);
  }

  http.end();
}

void loop() {
  sendHeartbeat();
  delay(300000); // Send heartbeat every 5 minutes
}
```

---

## Operational Benefits

### For Customers
- Customer can verify their board is working after delivery
- Real-time status visible in customer portal (future feature)
- Automatic activation upon first power-on

### For Admin
- See which boards are online in real-time
- Identify connectivity issues quickly
- Track board activation after shipping
- Diagnostics via IP address tracking
- Historical activation data

### For Support
- Verify board connectivity remotely
- Troubleshoot customer issues
- Confirm boards are functioning properly
- Track board lifecycle (manufactured → shipped → activated)

---

## Monitoring Dashboard Features

### Boards Tab - Main View
- Total boards counter
- In stock / Assigned / Active counters
- Filterable, searchable board list
- Online/offline indicators for each board
- Last seen timestamps
- Quick edit access

### Edit Board Modal
Shows comprehensive monitoring status:
- Current online/offline state
- Last connection time (with smart formatting)
- IP address from last connection
- First activation timestamp
- All editable board properties

### Order Details
When viewing order details, assigned boards show:
- Online status indicator
- Last seen timestamp
- Board activation status

---

## Status Transitions

The monitoring system handles these automatic status transitions:

```
in_stock → active (when board tested/activated)
assigned → active (when customer receives and powers on)
shipped → active (when customer receives and powers on)
```

Manual transitions available in edit board:
- active → faulty (if board malfunctions)
- active → returned (if customer returns)
- Any status can be set manually by admin

---

## Troubleshooting

### Board Not Showing as Online

**Check:**
1. Is the board sending heartbeats? (check ESP32 serial output)
2. Is the board_id correct and exists in database?
3. Is the board connected to WiFi?
4. Check last_seen timestamp in database
5. Run test script: `node test-board-heartbeat.js [board_id]`

### First Activation Not Detected

**Check:**
1. Verify `activated_at` column is NULL before first heartbeat
2. Check order_history for `board_activated` event
3. Ensure board is assigned to an order (has order_id set)

### Permission Errors

**Check:**
1. Run `/supabase/add-monitoring.sql` in Supabase SQL Editor
2. Verify RLS policy exists: `Boards can update their own heartbeat`
3. Check function permissions: `board_heartbeat` granted to `anon`

---

## Files Modified/Created

### Modified
- `/website/admin/index.html` - Added edit board modal
- `/website/admin/admin-script.js` - Implemented edit board functionality

### Created
- `/test-board-heartbeat.js` - Monitoring integration test script

### Existing (Verified Working)
- `/website/api/board-heartbeat.js` - API endpoint
- `/supabase/add-monitoring.sql` - Database functions and schema

---

## Next Steps (Optional Enhancements)

1. **Customer Portal:** Show board online status to customers
2. **Email Notifications:** Alert when board goes offline
3. **Monitoring Dashboard:** Dedicated monitoring view with graphs
4. **Firmware Updates:** Remote firmware update capability via monitoring
5. **Configuration Sync:** Sync board configuration via heartbeat responses
6. **Health Metrics:** Include battery level, WiFi signal, memory usage in heartbeat

---

## Conclusion

✅ **The monitoring integration is fully functional and production-ready.**

All components are working correctly:
- ESP32 boards can send heartbeats
- API endpoint processes requests
- Database tracks all monitoring data
- Admin dashboard displays online/offline status
- Edit board functionality allows manual updates
- Test script validates the entire flow

No additional setup required - the system is ready to track boards as soon as they power on and send their first heartbeat.
