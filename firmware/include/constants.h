#ifndef CONSTANTS_H
#define CONSTANTS_H

// ============ Display Constants ============
namespace Display {
  // Display dimensions
  const int WIDTH = 256;
  const int HEIGHT = 64;

  // Text positioning
  const int ETD_RIGHT_X = 251;  // Right-aligned position for ETD times
  const int TEXT_MARGIN = 5;
  const int CLIP_WINDOW_END = 255;

  // Default Y positions (configurable via web interface)
  const int DEFAULT_Y_TOP = 12;    // When station name hidden
  const int DEFAULT_Y_1ST = 26;    // First service line
  const int DEFAULT_Y_2ND = 38;    // Second service line
  const int DEFAULT_Y_ALT = 50;    // Alternating services line

  // Animation
  const int SCROLL_STEP = 2;
  const int TEXT_SPACING = 10;

  // Progress bar
  const int PROGRESS_SEGMENTS = 10;
  const int PROGRESS_SEGMENT_WIDTH = 22;
  const int PROGRESS_SEGMENT_HEIGHT = 16;
  const int PROGRESS_SPACING = 2;
  const int PROGRESS_START_X = 5;
  const int PROGRESS_START_Y = 36;
}

// ============ Network Constants ============
// Note: Named 'Net' to avoid conflict with ESP32's Network object
namespace Net {
  // Timeouts (milliseconds)
  const unsigned long WIFI_CONNECT_TIMEOUT = 30000;     // 30 seconds
  const unsigned long API_CONNECT_TIMEOUT = 8000;       // 8 seconds
  const unsigned long API_RESPONSE_TIMEOUT = 10000;     // 10 seconds (reduced from 15s with optimized reading)
  const unsigned long NTP_SYNC_TIMEOUT = 10000;         // 10 seconds

  // Retry intervals
  const unsigned long MIN_FETCH_RETRY_INTERVAL = 30000; // 30 seconds between attempts

  // Buffer sizes
  const size_t FETCH_BUFFER_SIZE = 16384;  // 16KB for API responses
  const size_t SOAP_REQUEST_SIZE = 512;
  const size_t HTTP_CHUNK_SIZE = 512;

  // WiFi configuration
  inline const char* DEFAULT_AP_SSID = "TrainBoard_AP";
  inline const char* DEFAULT_AP_PASSWORD = "config123";  // TODO: Generate random password

  // Server ports
  const uint16_t HTTP_PORT = 80;
  const uint16_t WEBSOCKET_PORT = 81;
  const uint16_t MONITOR_PORT = 3000;
}

// ============ Data Constants ============
namespace Data {
  // Service limits
  const int MAX_SERVICES = 8;
  const int MAX_EXTRA_SERVICES = 4;

  // String buffer sizes
  const int STD_TIME_SIZE = 6;
  const int ETD_SIZE = 10;
  const int DESTINATION_SIZE = 30;
  const int CALLING_POINTS_SIZE = 500;
  const int STATION_NAME_SIZE = 50;

  // Configuration limits
  const int MIN_REFRESH_INTERVAL = 30;   // seconds
  const int MAX_REFRESH_INTERVAL = 600;  // seconds
  const int MIN_SCROLL_SPEED = 10;       // milliseconds
  const int MAX_SCROLL_SPEED = 200;
  const int MIN_ROTATION_SPEED = 5;      // seconds
  const int MAX_ROTATION_SPEED = 60;

  // API configuration
  inline const char* API_HOST = "lite.realtime.nationalrail.co.uk";
  inline const char* API_PATH = "/OpenLDBWS/ldb9.asmx";
  inline const char* NTP_SERVER = "pool.ntp.org";

  // Response validation
  const size_t MIN_VALID_RESPONSE_SIZE = 100;  // Bytes
  const size_t SHORT_RESPONSE_WARNING = 500;   // Warn if < 500 bytes
}

// ============ Timing Constants ============
namespace Timing {
  // Display refresh rates
  const unsigned long DISPLAY_UPDATE_INTERVAL = 200;      // milliseconds
  const unsigned long METRICS_BROADCAST_INTERVAL = 10000; // 10 seconds
  const unsigned long SNAPSHOT_BROADCAST_INTERVAL = 2000; // 2 seconds
  const unsigned long AP_DISPLAY_UPDATE = 30000;          // 30 seconds
  const unsigned long MONITOR_HEARTBEAT_INTERVAL = 30000; // 30 seconds
  const unsigned long MONITOR_LOOP_THROTTLE = 500;        // milliseconds
  const unsigned long MONITOR_DISCONNECT_DELAY = 2000;    // milliseconds

  // Animation timing
  const unsigned long SERVICE_ANIMATION_DURATION = 500;   // milliseconds for smooth scroll

  // Delays
  const unsigned long SPLASH_SCREEN_DELAY = 3000;
  const unsigned long WELCOME_SCREEN_DELAY = 3000;
  const unsigned long READY_SCREEN_DELAY = 4000;
  const unsigned long ERROR_MESSAGE_DELAY = 3000;
  const unsigned long RESTART_DELAY = 2000;
  const unsigned long LOOP_DELAY = 20;  // Main loop delay
}

// ============ OTA Constants ============
namespace OTA {
  inline const char* HOSTNAME = "trainboard";
  inline const char* PASSWORD = "trainboard2024";
}

// ============ WebSocket Constants ============
namespace WebSocket {
  const int MAX_CLIENTS = 10;
}

// ============ Error Messages ============
namespace ErrorMsg {
  inline const char* WIFI_NOT_CONFIGURED = "WiFi SSID not configured";
  inline const char* WIFI_CONNECT_FAILED = "WiFi Connection Failed";
  inline const char* WIFI_DISCONNECTED = "WiFi disconnected";
  inline const char* API_CONNECT_FAILED = "API connection failed";
  inline const char* TIMEOUT_WAITING = "Timeout WAITING";
  inline const char* TIMEOUT_READING = "Timeout READING";
  inline const char* INVALID_RESPONSE = "Invalid response size";
  inline const char* PARSE_FAILED = "Parse failed";
  inline const char* NO_SERVICES_TAG = "No services tag found";
  inline const char* SOAP_FAULT = "SOAP Fault detected";
  inline const char* SPIFFS_MOUNT_FAILED = "SPIFFS Mount Failed";
  inline const char* SYSTEM_ERROR = "System Error";
}

// ============ Success Messages ============
namespace SuccessMsg {
  inline const char* WIFI_CONNECTED = "WiFi Connected";
  inline const char* TIME_SYNCED = "Time synchronized";
  inline const char* OTA_READY = "OTA Ready";
  inline const char* HTTP_SERVER_STARTED = "HTTP server started";
  inline const char* WEBSOCKET_STARTED = "WebSocket server started";
  inline const char* SETUP_COMPLETE = "Setup complete";
  inline const char* CONFIG_LOADED = "Config loaded";
  inline const char* CONFIG_SAVED = "Config saved";
  inline const char* PARSE_SUCCESSFUL = "Parse successful";
}

#endif
