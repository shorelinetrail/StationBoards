/**
 * EXAMPLE: Refactored Configuration Handling
 *
 * This file demonstrates how the web server configuration handling
 * would look after applying the code quality improvements.
 *
 * BEFORE: 80+ lines with scattered validation and error handling
 * AFTER: Clean, validated, well-structured code
 */

#include <Arduino.h>
#include <WebServer.h>
#include "../include/constants.h"
#include "../include/types.h"
#include "../include/helpers.h"
#include "../include/config.h"

// External dependencies
extern WebServer server;
extern Config config;
extern DisplayState displayState;
extern FetchStateData fetchState;
extern WiFiClientSecure fetchClient;

// ============ BEFORE: Original Code ============
/*
void handleApplyConfigOld() {
  String oldStation = String(config.stationCode);
  bool oldCallingAt = config.useCallingAt;

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

  if (server.hasArg("interval")) config.refreshInterval = server.arg("interval").toInt();
  if (server.hasArg("mode")) config.useCallingAt = (server.arg("mode") == "1");
  // ... 50+ more lines of similar code
}
*/

// ============ AFTER: Refactored Code ============

/**
 * Validates and applies WiFi configuration
 * Returns ValidationResult indicating success or failure
 */
ValidationResult applyWiFiConfig() {
  if (!server.hasArg("ssid")) {
    return ValidationResult::success();  // No change requested
  }

  String ssid = server.arg("ssid");
  ValidationResult ssidResult = validateSSID(ssid);
  if (!ssidResult.valid) {
    return ssidResult;
  }

  // Validate password if provided
  if (server.hasArg("password") && !server.arg("password").isEmpty()) {
    String password = server.arg("password");
    ValidationResult pwResult = validatePassword(password);
    if (!pwResult.valid) {
      return pwResult;
    }

    safeStrCopy(config.wifiPassword, password, sizeof(config.wifiPassword));
  }

  safeStrCopy(config.wifiSSID, ssid, sizeof(config.wifiSSID));
  logInfo("WiFi configuration updated");

  return ValidationResult::success();
}

/**
 * Validates and applies station configuration
 * Returns ValidationResult and sets stationChanged flag
 */
ValidationResult applyStationConfig(bool& stationChanged) {
  stationChanged = false;

  if (!server.hasArg("station")) {
    return ValidationResult::success();  // No change requested
  }

  String oldStation = String(config.stationCode);
  String station = sanitizeStationCode(server.arg("station"));

  ValidationResult result = validateStationCode(station);
  if (!result.valid) {
    return result;
  }

  safeStrCopy(config.stationCode, station, sizeof(config.stationCode));

  stationChanged = (oldStation != station);
  if (stationChanged) {
    logInfo(("Station changed to " + station).c_str());
  }

  return ValidationResult::success();
}

/**
 * Validates and applies display configuration
 * Returns ValidationResult and sets displayModeChanged flag
 */
ValidationResult applyDisplayConfig(bool& displayModeChanged, bool& switchedToCallingAt) {
  displayModeChanged = false;
  switchedToCallingAt = false;

  bool oldCallingAt = config.useCallingAt;
  int oldExtraServices = config.extraServices;

  // Refresh interval
  if (server.hasArg("interval")) {
    int interval = server.arg("interval").toInt();
    ValidationResult result = validateRange(
      interval,
      Data::MIN_REFRESH_INTERVAL,
      Data::MAX_REFRESH_INTERVAL,
      "Refresh interval"
    );
    if (!result.valid) return result;

    config.refreshInterval = interval;
  }

  // Display mode
  if (server.hasArg("mode")) {
    config.useCallingAt = (server.arg("mode") == "1");
  }

  // Station name visibility
  if (server.hasArg("showstation")) {
    config.showStationName = (server.arg("showstation") == "1");
  }

  // Extra services
  if (server.hasArg("extra")) {
    int extra = server.arg("extra").toInt();
    config.extraServices = constrainToRange(extra, 0, Data::MAX_EXTRA_SERVICES);
  }

  // Scroll speed
  if (server.hasArg("scrollspeed")) {
    int speed = server.arg("scrollspeed").toInt();
    config.scrollSpeed = constrainToRange(
      speed,
      Data::MIN_SCROLL_SPEED,
      Data::MAX_SCROLL_SPEED
    );
  }

  // Rotation speed
  if (server.hasArg("rotationspeed")) {
    int speed = server.arg("rotationspeed").toInt();
    config.rotationSpeed = constrainToRange(
      speed,
      Data::MIN_ROTATION_SPEED,
      Data::MAX_ROTATION_SPEED
    );
  }

  // Y positions (with range validation)
  if (server.hasArg("ytop")) {
    int y = server.arg("ytop").toInt();
    config.yPosTop = constrainToRange(y, 0, Display::HEIGHT);
  }
  if (server.hasArg("y1")) {
    int y = server.arg("y1").toInt();
    config.yPos1st = constrainToRange(y, 0, Display::HEIGHT);
  }
  if (server.hasArg("y2")) {
    int y = server.arg("y2").toInt();
    config.yPos2nd = constrainToRange(y, 0, Display::HEIGHT);
  }
  if (server.hasArg("y3")) {
    int y = server.arg("y3").toInt();
    config.yPosAlt = constrainToRange(y, 0, Display::HEIGHT);
  }

  // Detect mode changes
  displayModeChanged = (oldCallingAt != config.useCallingAt) ||
                       (oldExtraServices != config.extraServices);
  switchedToCallingAt = (!oldCallingAt && config.useCallingAt);

  if (displayModeChanged) {
    logInfo("Display mode changed");
  }

  return ValidationResult::success();
}

/**
 * Updates the alternating service index based on current configuration
 */
void updateAlternatingServiceIndex() {
  int offset = getServiceOffset(config.showStationName);

  if (config.useCallingAt) {
    displayState.currentAlternatingService = 1 + offset;
  } else {
    displayState.currentAlternatingService = 2 + offset;
  }

  displayState.resetAnimation();
  logInfo("Alternating service index updated");
}

/**
 * Triggers immediate data fetch with proper state cleanup
 */
void triggerImmediateFetch() {
  displayState.clearServices();

  if (fetchState.isActive()) {
    fetchClient.stop();
    fetchState.reset();
  }

  logInfo("Immediate fetch triggered");
}

/**
 * Main handler for /apply endpoint
 * Applies configuration changes without restarting device
 */
void handleApplyConfig() {
  logInfo("Processing configuration update...");

  // Track what changed
  bool stationChanged = false;
  bool displayModeChanged = false;
  bool switchedToCallingAt = false;

  // Apply WiFi settings (validation included)
  ValidationResult wifiResult = applyWiFiConfig();
  if (!wifiResult.valid) {
    server.send(400, "text/plain", wifiResult.message);
    logError(ERROR_CONFIG_SAVE_FAILED, wifiResult.message.c_str());
    return;
  }

  // Apply station settings
  ValidationResult stationResult = applyStationConfig(stationChanged);
  if (!stationResult.valid) {
    server.send(400, "text/plain", stationResult.message);
    logError(ERROR_CONFIG_SAVE_FAILED, stationResult.message.c_str());
    return;
  }

  // Apply display settings
  ValidationResult displayResult = applyDisplayConfig(displayModeChanged, switchedToCallingAt);
  if (!displayResult.valid) {
    server.send(400, "text/plain", displayResult.message);
    logError(ERROR_CONFIG_SAVE_FAILED, displayResult.message.c_str());
    return;
  }

  // Update service rotation index if needed
  if (displayModeChanged) {
    updateAlternatingServiceIndex();
  }

  // Clear calling points when switching to calling at mode
  if (switchedToCallingAt) {
    for (int i = 0; i < Data::MAX_SERVICES; i++) {
      displayState.services[i].callingPoints[0] = '\0';
    }
    logInfo("Cleared calling points for fresh fetch");
  }

  // Save configuration to SPIFFS
  if (!config.save()) {
    server.send(500, "text/plain", "Failed to save configuration");
    logError(ERROR_CONFIG_SAVE_FAILED, "SPIFFS write failed");
    return;
  }

  // Notify WebSocket clients
  broadcastStatus("Settings applied successfully", "success");

  if (displayModeChanged) {
    broadcastDisplaySnapshot();
    broadcastTrainUpdate();
  }

  // Send success response
  String html = FPSTR(APPLY_SUCCESS_PAGE);
  server.send(200, "text/html", html);

  // Trigger immediate fetch if station changed or switched to calling at
  if (stationChanged || switchedToCallingAt) {
    triggerImmediateFetch();

    if (stationChanged) {
      logInfo(("Station changed - fetching data for " + String(config.stationCode)).c_str());
      broadcastStatus("Station changed - fetching new data...", "info");
    } else {
      logInfo("Fetching detailed calling points data");
      broadcastStatus("Fetching calling points...", "info");
    }
  }

  logSuccess("Configuration updated successfully");
}

// ============ Benefits of Refactored Version ============
/*
 * ✅ Validation: All inputs validated before applying
 * ✅ Error Handling: Consistent error responses with proper HTTP codes
 * ✅ Logging: Structured logging with logInfo/logError helpers
 * ✅ Safety: Safe string copying with size checks
 * ✅ Clarity: Each function has single responsibility
 * ✅ Testability: Small functions easy to unit test
 * ✅ Maintainability: Easy to add new config options
 * ✅ Constants: No magic numbers, all from constants.h
 * ✅ Structs: Uses DisplayState and FetchStateData
 * ✅ Documentation: Clear comments explain what each function does
 *
 * Lines of code:
 * - Before: ~80 lines of tangled logic
 * - After: ~190 lines BUT with validation, error handling, logging
 * - Effective complexity: Much lower
 * - Bugs prevented: Many (buffer overflows, invalid values, etc.)
 */
