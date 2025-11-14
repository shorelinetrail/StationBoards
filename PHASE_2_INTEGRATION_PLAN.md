# Phase 2: Integration Plan

## Overview

Phase 2 integrates the code quality improvements into the existing codebase incrementally. Each step is designed to be:
- ✅ **Safe** - Can be tested independently
- ✅ **Reversible** - Easy to rollback if issues arise
- ✅ **Incremental** - Small changes with immediate testing
- ✅ **Low Risk** - Minimal chance of breaking functionality

**Estimated Time**: 2-4 hours (with testing)
**Risk Level**: LOW to MEDIUM
**Prerequisites**: Phase 1 complete ✅

---

## Step 1: Add Header Includes (5 minutes) - LOWEST RISK

### What to do:
Add the new header files to `main.cpp` after existing includes.

### Code Changes:

**Location**: `src/main.cpp` - Top of file after existing includes

```cpp
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>
#include <time.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <WebSocketsServer.h>
#include <WebSocketsClient.h>
#include <HTTPUpdate.h>
#include "config.h"
#include "web_pages.h"

// ADD THESE NEW INCLUDES:
#include "constants.h"    // Named constants for all magic numbers
#include "types.h"        // Data structures to organize globals
#include "helpers.h"      // Validation and utility functions
#include "display_functions.h"  // Display component functions
```

### Test:
```bash
# Compile to ensure headers are found
pio run

# Should compile without errors
```

### Success Criteria:
- ✅ Project compiles successfully
- ✅ No new warnings
- ✅ No linker errors

### Rollback:
Remove the four new include lines if issues occur.

**Risk Level**: ⚪ MINIMAL - Just adding includes, no logic changes

---

## Step 2: Replace Magic Numbers with Constants (30 minutes) - LOW RISK

This step replaces hardcoded numbers with named constants. Start with the easiest ones first.

### 2A: Replace Network Timeouts

**Location**: `src/main.cpp:2049-2050`

**Before**:
```cpp
fetchClient.setInsecure();
fetchClient.setTimeout(8000);  // Reduced from 15s to 8s
```

**After**:
```cpp
fetchClient.setInsecure();
fetchClient.setTimeout(Network::API_CONNECT_TIMEOUT);
```

**Location**: `src/main.cpp:980`

**Before**:
```cpp
} else if (millis() - fetchStartTime > 8000) {
```

**After**:
```cpp
} else if (millis() - fetchStartTime > Network::API_CONNECT_TIMEOUT) {
```

**Location**: `src/main.cpp:1025`

**Before**:
```cpp
if (millis() - fetchStartTime > 15000) {
```

**After**:
```cpp
if (millis() - fetchStartTime > Network::API_RESPONSE_TIMEOUT) {
```

### 2B: Replace Buffer Sizes

**Location**: `src/main.cpp:2051`

**Before**:
```cpp
fetchBuffer.reserve(16384);  // Pre-allocate buffer
```

**After**:
```cpp
fetchBuffer.reserve(Network::FETCH_BUFFER_SIZE);
```

**Location**: `src/main.cpp:939`

**Before**:
```cpp
String soapRequest;
soapRequest.reserve(512);
```

**After**:
```cpp
String soapRequest;
soapRequest.reserve(Network::SOAP_REQUEST_SIZE);
```

### 2C: Replace Display Constants

**Location**: Throughout `updateDisplay()` function

**Before**:
```cpp
const int ETD_RIGHT_X = 251;  // Multiple occurrences
```

**After**:
```cpp
const int ETD_RIGHT_X = Display::ETD_RIGHT_X;
```

Or better, just use `Display::ETD_RIGHT_X` directly everywhere.

**Find and Replace**:
- Find: `251` (in display context) → Replace: `Display::ETD_RIGHT_X`
- Find: `256` (width) → Replace: `Display::WIDTH`
- Find: `64` (height) → Replace: `Display::HEIGHT`
- Find: `255` (clip window) → Replace: `Display::CLIP_WINDOW_END`

### 2D: Replace Timing Constants

**Location**: `src/main.cpp:2155`

**Before**:
```cpp
if (currentTime - lastMetricsBroadcast >= 10000) {
```

**After**:
```cpp
if (currentTime - lastMetricsBroadcast >= Timing::METRICS_BROADCAST_INTERVAL) {
```

**Location**: `src/main.cpp:2161`

**Before**:
```cpp
if (currentTime - lastDisplaySnapshot >= 2000) {
```

**After**:
```cpp
if (currentTime - lastDisplaySnapshot >= Timing::SNAPSHOT_BROADCAST_INTERVAL) {
```

**Location**: `src/main.cpp:2166`

**Before**:
```cpp
delay(20);
```

**After**:
```cpp
delay(Timing::LOOP_DELAY);
```

### Test After Step 2:
```bash
# Compile
pio run

# Upload and test
pio run --target upload
pio device monitor

# Verify:
# - Display updates normally
# - API fetching works
# - Timeouts still work correctly
# - No visual changes
```

### Success Criteria:
- ✅ Compiles without warnings
- ✅ Display updates at same rate
- ✅ API fetches complete successfully
- ✅ No functional changes observed

**Risk Level**: 🟡 LOW - Numeric replacements, compiler will catch type mismatches

---

## Step 3: Add Input Validation to Web Handlers (45 minutes) - LOW-MEDIUM RISK

Add validation to user input in web server endpoints. This improves security and stability.

### 3A: Validate Station Code

**Location**: `src/main.cpp:1764-1777` (in `/save` handler)

**Before**:
```cpp
if (server.hasArg("station")) {
  String station = server.arg("station");
  station.trim();
  station.toUpperCase();
  if (station.length() >= 3) {
    station = station.substring(0, 3);
  }
  if (station.length() == 3) {
    strncpy(config.stationCode, station.c_str(), sizeof(config.stationCode) - 1);
    config.stationCode[3] = '\0';
  }
}
```

**After**:
```cpp
if (server.hasArg("station")) {
  String station = sanitizeStationCode(server.arg("station"));
  ValidationResult result = validateStationCode(station);

  if (result.valid) {
    safeStrCopy(config.stationCode, station, sizeof(config.stationCode));
    logInfo(("Station code set to: " + station).c_str());
  } else {
    logWarning(result.message.c_str());
    // Optionally send error response
  }
}
```

**Do the same in `/apply` handler** at lines 1822-1835.

### 3B: Validate WiFi Credentials

**Location**: `src/main.cpp:1756-1762` (in `/save` handler)

**Before**:
```cpp
if (server.hasArg("ssid")) {
  strncpy(config.wifiSSID, server.arg("ssid").c_str(), sizeof(config.wifiSSID) - 1);
  config.wifiSSID[sizeof(config.wifiSSID) - 1] = '\0';
}
if (server.hasArg("password") && !server.arg("password").isEmpty()) {
  strncpy(config.wifiPassword, server.arg("password").c_str(), sizeof(config.wifiPassword) - 1);
  config.wifiPassword[sizeof(config.wifiPassword) - 1] = '\0';
}
```

**After**:
```cpp
if (server.hasArg("ssid")) {
  String ssid = server.arg("ssid");
  ValidationResult ssidResult = validateSSID(ssid);

  if (ssidResult.valid) {
    safeStrCopy(config.wifiSSID, ssid, sizeof(config.wifiSSID));
  } else {
    logWarning(ssidResult.message.c_str());
  }
}

if (server.hasArg("password") && !server.arg("password").isEmpty()) {
  String password = server.arg("password");
  ValidationResult pwResult = validatePassword(password);

  if (pwResult.valid) {
    safeStrCopy(config.wifiPassword, password, sizeof(config.wifiPassword));
  } else {
    logWarning(pwResult.message.c_str());
  }
}
```

### 3C: Validate Numeric Ranges

**Location**: `src/main.cpp:1778, 1836` (refresh interval)

**Before**:
```cpp
if (server.hasArg("interval")) config.refreshInterval = server.arg("interval").toInt();
```

**After**:
```cpp
if (server.hasArg("interval")) {
  int interval = server.arg("interval").toInt();
  config.refreshInterval = constrainToRange(
    interval,
    Data::MIN_REFRESH_INTERVAL,
    Data::MAX_REFRESH_INTERVAL
  );
}
```

**Do the same for**:
- `scrollSpeed` (lines 1786, 1844)
- `rotationSpeed` (lines 1787-1791, 1845-1849)
- Y positions (lines 1792-1795, 1850-1853)

### 3D: Replace Serial.println with Logging Helpers

**Throughout the file**, replace:

**Before**:
```cpp
Serial.println("❌ WiFi connection failed");
Serial.println("✅ WiFi Connected!");
Serial.println("⚠️ Response seems short");
```

**After**:
```cpp
logError(ERROR_WIFI_NOT_CONNECTED, "WiFi connection failed");
logSuccess("WiFi Connected!");
logWarning("Response seems short");
```

This can be done gradually. Start with critical error messages first.

### Test After Step 3:
```bash
# Upload and test
pio run --target upload
pio device monitor

# Test validation:
# 1. Try invalid station code (e.g., "abc") - should be rejected
# 2. Try empty SSID - should be rejected
# 3. Try short password - should be rejected
# 4. Try out-of-range interval - should be constrained
# 5. Check serial logs show new format
```

### Success Criteria:
- ✅ Invalid inputs are rejected or constrained
- ✅ Logging shows consistent format
- ✅ Config saves with validated values
- ✅ No crashes from invalid input

**Risk Level**: 🟡 LOW-MEDIUM - Adding validation can only improve stability

---

## Step 4: Migrate to Structs (1 hour) - MEDIUM RISK

This is the most significant change. Replace global variables with struct instances.

### 4A: Declare Struct Instances

**Location**: `src/main.cpp` - After existing globals (around line 89)

**Add these declarations**:
```cpp
// ============ REFACTORED: State Management Structs ============
DisplayState displayState;
FetchStateData fetchStateData;
WebSocketClients wsClients;
MonitoringState monitoringState;
SystemFlags systemFlags;
```

### 4B: Replace FetchState Variables (Start Small)

**Old globals to replace** (lines 60-74):
```cpp
FetchState fetchState = FETCH_IDLE;
WiFiClientSecure fetchClient;
String fetchBuffer;
unsigned long fetchStartTime = 0;
unsigned long lastFetchAttempt = 0;
unsigned long lastSuccessfulFetch = 0;
```

**Strategy**: Keep old globals temporarily, copy values to new struct:

```cpp
// OLD - Keep for now
FetchState fetchState = FETCH_IDLE;
String fetchBuffer;
unsigned long fetchStartTime = 0;
unsigned long lastFetchAttempt = 0;
unsigned long lastSuccessfulFetch = 0;

// NEW - Sync with old values
FetchStateData fetchStateData;
```

**Then throughout the code**, add sync points:
```cpp
// After modifying fetchState
fetchStateData.state = fetchState;

// After modifying fetchBuffer
fetchStateData.buffer = fetchBuffer;
```

This allows testing both systems in parallel before full cutover.

### 4C: Replace DisplayState Variables

**Old globals** (lines 37-58):
```cpp
ServiceData services[8];
int serviceCount = 0;
char stationName[50] = "Station";
int currentAlternatingService = 2;
bool isAnimating = false;
int animationOffset = 0;
// ... etc
```

**Strategy**: Same parallel approach:
```cpp
// OLD - Keep temporarily
ServiceData services[8];
int serviceCount = 0;
// ... etc

// NEW
DisplayState displayState;

// SYNC function
void syncDisplayState() {
  displayState.serviceCount = serviceCount;
  memcpy(displayState.services, services, sizeof(services));
  strcpy(displayState.stationName, stationName);
  displayState.currentAlternatingService = currentAlternatingService;
  // ... etc
}
```

Call `syncDisplayState()` after any modifications.

### 4D: Gradually Replace References

Start with one function at a time. For example, in `updateDisplay()`:

**Before**:
```cpp
if (serviceCount == 0) {
  // show message
}
```

**After**:
```cpp
if (displayState.serviceCount == 0) {
  // show message
}
```

Test after each function conversion.

### Test After Step 4:
```bash
# This is the most critical testing phase

# 1. Compile and upload
pio run --target upload
pio device monitor

# 2. Test ALL functionality:
# - Display updates correctly
# - Services rotate properly
# - Calling points scroll
# - Station name shows/hides
# - API fetching works
# - WebSocket works
# - Configuration saves/loads
# - OTA updates work

# 3. Monitor for:
# - Memory leaks (check free heap)
# - Crashes or reboots
# - Incorrect display rendering
# - Timing issues
```

### Success Criteria:
- ✅ All functionality works identically
- ✅ No memory leaks (heap stable)
- ✅ No crashes
- ✅ Display renders correctly
- ✅ Can run for 24+ hours without issues

### Rollback:
If issues occur, remove references to new structs and revert to old globals.

**Risk Level**: 🟠 MEDIUM - Significant refactoring, requires thorough testing

---

## Step 5: Refactor Large Functions (1 hour) - MEDIUM RISK

Break up `updateDisplay()` using the display helper functions.

### 5A: Extract Clock Display

**Location**: `src/main.cpp:1710-1720` (in `updateDisplay()`)

**Before**:
```cpp
// Clock at bottom
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
```

**After**:
```cpp
// Clock at bottom
displayClock();
```

Test to ensure clock still displays correctly.

### 5B: Extract Station Name Display

**Location**: `src/main.cpp:1352-1366`

**Before**:
```cpp
if (config.showStationName) {
  String displayName = stationName;
  int nameWidth = u8g2.getUTF8Width(displayName.c_str());

  if (nameWidth > 250) {
    // ... truncation logic
  }

  u8g2.setCursor((256 - nameWidth) / 2, 12);
  u8g2.print(displayName);
}
```

**After**:
```cpp
if (config.showStationName) {
  displayStationName(displayState.stationName);
}
```

### 5C: Extract Service Line Display

Wherever you see the pattern of displaying a service (time + destination + ETD):

**Before** (~15 lines each time):
```cpp
int yPos = config.yPos1st;
String leftSide = "1st " + String(services[0].std) + " ";
String rightSide = formatETD(String(services[0].etd));
// ... 12 more lines
```

**After**:
```cpp
displayServiceLine(displayState.services[0], "1st ", config.yPos1st, u8g2);
```

### Continue incrementally
Extract one function at a time, test each change.

### Test After Step 5:
```bash
# Upload and visually inspect display
pio run --target upload

# Verify:
# - All services display correctly
# - Positioning is exact (compare to before)
# - Animations work smoothly
# - Scrolling works
# - Clock displays
# - Station name shows/hides correctly
```

### Success Criteria:
- ✅ Display looks identical to before
- ✅ All animations smooth
- ✅ Code is much more readable
- ✅ Functions are < 50 lines each

**Risk Level**: 🟠 MEDIUM - Display changes are highly visible

---

## Step 6: Final Testing & Cleanup (30 minutes)

### 6A: Extended Runtime Test
```bash
# Let device run for 24-48 hours
# Monitor:
# - Heap doesn't decrease (no memory leaks)
# - No crashes or reboots
# - API fetching continues working
# - Display updates continue
```

### 6B: Remove Old Globals
Once everything works with new structs:
```cpp
// DELETE these old globals:
// ServiceData services[8];  // Now in displayState
// int serviceCount = 0;      // Now in displayState
// etc.
```

### 6C: Remove Sync Code
Delete any temporary sync functions added in Step 4.

### 6D: Final Code Review
- Remove any commented-out code
- Ensure all TODOs are addressed
- Check for any remaining magic numbers
- Verify consistent use of new helpers

---

## Testing Checklist

After each major step, verify:

### Basic Functionality
- [ ] Device boots successfully
- [ ] WiFi connects
- [ ] Display initializes
- [ ] API fetching works
- [ ] Data parses correctly
- [ ] Services display
- [ ] Time displays

### Display Features
- [ ] Station name shows/hides
- [ ] Service rotation works
- [ ] Calling points scroll
- [ ] Animations smooth
- [ ] Clock updates
- [ ] No visual glitches

### Web Interface
- [ ] Can access config page
- [ ] Can save settings
- [ ] Can apply settings
- [ ] Validation works
- [ ] WebSocket updates
- [ ] Network scan works

### Edge Cases
- [ ] No trains available
- [ ] WiFi disconnect/reconnect
- [ ] API timeout
- [ ] Invalid station code
- [ ] First boot
- [ ] Factory reset

### Performance
- [ ] Heap stable (no leaks)
- [ ] CPU usage acceptable
- [ ] No unexpected delays
- [ ] Smooth animations

---

## Rollback Plan

If any step causes issues:

1. **Git Revert**: Each step should be a separate commit
   ```bash
   git log  # Find commit hash
   git revert <hash>
   ```

2. **Partial Rollback**: Comment out problem code
   ```cpp
   // Temporarily disable new code
   // displayClock();

   // Use old code
   time_t now = time(nullptr);
   // ... old clock code
   ```

3. **Full Rollback**: Return to Phase 1
   ```bash
   git checkout claude/review-code-01MUitzRY6y7rFy8PVGpyZzb
   ```

---

## Success Criteria for Phase 2 Complete

- ✅ All new headers integrated
- ✅ All magic numbers replaced
- ✅ Input validation active
- ✅ Globals migrated to structs
- ✅ Large functions refactored
- ✅ All tests passing
- ✅ 24hr stability test passed
- ✅ Code cleaned up
- ✅ Documentation updated

---

## Estimated Timeline

| Step | Time | Risk | Can Skip? |
|------|------|------|-----------|
| 1. Add includes | 5 min | Minimal | No |
| 2. Replace constants | 30 min | Low | No |
| 3. Add validation | 45 min | Low-Med | Yes (but recommended) |
| 4. Migrate structs | 1 hr | Medium | Yes (but key benefit) |
| 5. Refactor functions | 1 hr | Medium | Yes (but major benefit) |
| 6. Testing & cleanup | 30 min | - | No |
| **Total** | **3-4 hrs** | | |

Plus 24-48 hours of runtime testing.

---

## Next Steps

Would you like me to:

1. **Start Step 1** - Add the includes and compile?
2. **Create a feature branch** for Phase 2 work?
3. **Do Step 2** - Begin replacing constants?
4. **Wait for your review** of this plan?

Let me know which approach you prefer!
