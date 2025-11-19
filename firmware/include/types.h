#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>
#include "constants.h"

// ============ Service Data Structure ============
struct ServiceData {
  char std[Data::STD_TIME_SIZE];
  char etd[Data::ETD_SIZE];
  char destination[Data::DESTINATION_SIZE];
  char callingPoints[Data::CALLING_POINTS_SIZE];
};

// ============ Fetch State Machine ============
enum FetchState {
  FETCH_IDLE,
  FETCH_START,
  FETCH_WAITING,
  FETCH_READING,
  FETCH_DONE,
  FETCH_FAIL
};

// ============ Error Codes ============
enum ErrorCode {
  ERROR_NONE = 0,
  ERROR_WIFI_NOT_CONNECTED,
  ERROR_API_CONNECTION_FAILED,
  ERROR_API_TIMEOUT,
  ERROR_INVALID_RESPONSE,
  ERROR_PARSE_FAILED,
  ERROR_SPIFFS_FAILED,
  ERROR_CONFIG_LOAD_FAILED,
  ERROR_CONFIG_SAVE_FAILED
};

// ============ Fetch State Management ============
struct FetchStateData {
  FetchState state;
  String buffer;
  unsigned long startTime;
  unsigned long lastAttempt;
  unsigned long lastSuccess;

  FetchStateData() :
    state(FETCH_IDLE),
    buffer(""),
    startTime(0),
    lastAttempt(0),
    lastSuccess(0) {
    buffer.reserve(Net::FETCH_BUFFER_SIZE);
  }

  void reset() {
    state = FETCH_IDLE;
    buffer = "";
    startTime = 0;
  }

  bool isIdle() const { return state == FETCH_IDLE; }
  bool isActive() const { return state != FETCH_IDLE && state != FETCH_FAIL; }
};

// ============ Display State Management ============
struct DisplayState {
  char stationName[Data::STATION_NAME_SIZE];
  ServiceData services[Data::MAX_SERVICES];
  int serviceCount;
  int currentAlternatingService;

  bool isAnimating;
  int animationOffset;
  unsigned long animationStartTime;  // Track when animation started for easing
  int callingAtScrollOffset;
  unsigned long lastCallingAtScroll;
  unsigned long lastRotation;
  unsigned long lastSnapshot;

  bool fetchingNewStation;
  bool dirty;  // Flag to indicate display needs update

  DisplayState() :
    serviceCount(0),
    currentAlternatingService(2),
    isAnimating(false),
    animationOffset(0),
    animationStartTime(0),
    callingAtScrollOffset(0),
    lastCallingAtScroll(0),
    lastRotation(0),
    lastSnapshot(0),
    fetchingNewStation(false),
    dirty(true) {  // Start dirty to force initial draw
    strncpy(stationName, "Station", sizeof(stationName) - 1);
    stationName[sizeof(stationName) - 1] = '\0';
  }

  void clearServices() {
    serviceCount = 0;
    fetchingNewStation = true;
    dirty = true;
  }

  void resetAnimation() {
    isAnimating = false;
    animationOffset = 0;
    callingAtScrollOffset = 0;
    dirty = true;
  }

  void markDirty() {
    dirty = true;
  }

  void clearDirty() {
    dirty = false;
  }
};

// ============ WebSocket Client Management ============
struct WebSocketClients {
  uint8_t clients[WebSocket::MAX_CLIENTS];
  uint8_t count;
  unsigned long lastMetricsBroadcast;

  WebSocketClients() : count(0), lastMetricsBroadcast(0) {
    memset(clients, 0, sizeof(clients));
  }

  void add(uint8_t num) {
    if (count < WebSocket::MAX_CLIENTS) {
      clients[count++] = num;
    }
  }

  void remove(uint8_t num) {
    for (int i = 0; i < count; i++) {
      if (clients[i] == num) {
        for (int j = i; j < count - 1; j++) {
          clients[j] = clients[j + 1];
        }
        count--;
        break;
      }
    }
  }

  bool hasClients() const { return count > 0; }
};

// ============ Monitoring State ============
struct MonitoringState {
  String serverHost;
  int serverPort;
  bool useSSL;
  bool enabled;
  bool connected;
  unsigned long lastHeartbeat;
  unsigned long lastDisconnect;
  unsigned long lastLoop;

  MonitoringState() :
    serverHost("stationboards-production.up.railway.app"),
    serverPort(443),
    useSSL(true),
    enabled(true),
    connected(false),
    lastHeartbeat(0),
    lastDisconnect(0),
    lastLoop(0) {}

  bool shouldSendHeartbeat(unsigned long currentTime) const {
    return connected &&
           (currentTime - lastHeartbeat >= Timing::MONITOR_HEARTBEAT_INTERVAL);
  }

  bool shouldRunLoop(unsigned long currentTime) const {
    bool recentlyDisconnected = (currentTime - lastDisconnect < Timing::MONITOR_DISCONNECT_DELAY);
    return !recentlyDisconnected &&
           (currentTime - lastLoop > Timing::MONITOR_LOOP_THROTTLE);
  }
};

// ============ System State Flags ============
struct SystemFlags {
  bool apMode;
  bool systemError;
  bool firstBoot;

  SystemFlags() : apMode(false), systemError(false), firstBoot(false) {}
};

// ============ Validation Result ============
struct ValidationResult {
  bool valid;
  ErrorCode errorCode;
  String message;

  ValidationResult() : valid(true), errorCode(ERROR_NONE), message("") {}

  ValidationResult(bool v, ErrorCode ec, String msg) :
    valid(v), errorCode(ec), message(msg) {}

  static ValidationResult success() {
    return ValidationResult(true, ERROR_NONE, "");
  }

  static ValidationResult failure(ErrorCode ec, String msg) {
    return ValidationResult(false, ec, msg);
  }
};

#endif
