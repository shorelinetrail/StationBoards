#ifndef HELPERS_H
#define HELPERS_H

#include <Arduino.h>
#include "types.h"
#include "constants.h"

// ============ Input Validation Helpers ============

/**
 * Validates a UK railway station code (CRS code)
 * Must be exactly 3 uppercase letters A-Z
 */
inline ValidationResult validateStationCode(const String& code) {
  if (code.length() != 3) {
    return ValidationResult::failure(
      ERROR_NONE,
      "Station code must be exactly 3 characters"
    );
  }

  for (int i = 0; i < 3; i++) {
    if (!isalpha(code[i]) || !isupper(code[i])) {
      return ValidationResult::failure(
        ERROR_NONE,
        "Station code must contain only uppercase letters A-Z"
      );
    }
  }

  return ValidationResult::success();
}

/**
 * Validates and sanitizes a station code string
 * Extracts 3-letter code from longer strings, converts to uppercase
 */
inline String sanitizeStationCode(String code) {
  code.trim();
  code.toUpperCase();

  if (code.length() >= 3) {
    code = code.substring(0, 3);
  }

  return code;
}

/**
 * Validates WiFi SSID (1-32 characters)
 */
inline ValidationResult validateSSID(const String& ssid) {
  if (ssid.length() == 0) {
    return ValidationResult::failure(ERROR_NONE, "SSID cannot be empty");
  }

  if (ssid.length() > 32) {
    return ValidationResult::failure(ERROR_NONE, "SSID too long (max 32 characters)");
  }

  return ValidationResult::success();
}

/**
 * Validates WiFi password (8-63 characters for WPA2)
 */
inline ValidationResult validatePassword(const String& password) {
  if (password.length() > 0 && password.length() < 8) {
    return ValidationResult::failure(
      ERROR_NONE,
      "Password must be at least 8 characters"
    );
  }

  if (password.length() > 63) {
    return ValidationResult::failure(
      ERROR_NONE,
      "Password too long (max 63 characters)"
    );
  }

  return ValidationResult::success();
}

/**
 * Validates numeric configuration value is within range
 */
inline ValidationResult validateRange(int value, int min, int max, const char* name) {
  if (value < min || value > max) {
    String msg = String(name) + " must be between " + String(min) + " and " + String(max);
    return ValidationResult::failure(ERROR_NONE, msg);
  }

  return ValidationResult::success();
}

/**
 * Constrains a value to a valid range
 */
inline int constrainToRange(int value, int min, int max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

// ============ String Manipulation Helpers ============

/**
 * Safe string copy with null termination
 */
inline void safeStrCopy(char* dest, const String& src, size_t destSize) {
  if (destSize == 0) return;

  strncpy(dest, src.c_str(), destSize - 1);
  dest[destSize - 1] = '\0';
}

// Note: These functions are also defined in main.cpp
// Only use these definitions if not already defined
#ifndef DECODE_HTML_ENTITIES_DEFINED
/**
 * Decodes HTML entities in XML responses
 */
inline String decodeHTMLEntities(String text) {
  text.replace("&amp;", "&");
  text.replace("&lt;", "<");
  text.replace("&gt;", ">");
  text.replace("&quot;", "\"");
  text.replace("&#39;", "'");
  text.replace("&apos;", "'");
  return text;
}
#define DECODE_HTML_ENTITIES_DEFINED
#endif

#ifndef EXTRACT_TAG_VALUE_DEFINED
/**
 * Extracts value from XML tag with optional namespace
 */
inline String extractTagValue(String xml, String tag, String ns = "") {
  String openTag = "<" + (ns != "" ? ns + ":" : "") + tag + ">";
  String closeTag = "</" + (ns != "" ? ns + ":" : "") + tag + ">";

  int start = xml.indexOf(openTag);
  if (start == -1) return "";

  start += openTag.length();
  int end = xml.indexOf(closeTag, start);
  if (end == -1) return "";

  return xml.substring(start, end);
}
#define EXTRACT_TAG_VALUE_DEFINED
#endif

// ============ Display Helpers ============

#ifndef FORMAT_ETD_DEFINED
/**
 * Formats ETD (Estimated Time of Departure) for display
 * Adds "Exp" prefix if it's a time, leaves text status as-is
 */
inline String formatETD(String etd) {
  if (etd.length() == 5 && etd.indexOf(":") != -1) {
    return "Exp " + etd;
  }
  return etd;
}
#define FORMAT_ETD_DEFINED
#endif

/**
 * Truncates text to fit within maxWidth pixels, adding "." if needed
 * Template version that works with U8G2 display objects
 */
template<typename T>
inline String fitTextToWidth(String text, int maxWidth, T& display) {
  if (display.getUTF8Width(text.c_str()) <= maxWidth) {
    return text;
  }

  int ellipsisWidth = display.getUTF8Width(".");

  while (display.getUTF8Width(text.c_str()) > maxWidth - ellipsisWidth && text.length() > 0) {
    text.remove(text.length() - 1);
  }

  if (text.length() > 0) {
    text += ".";
  }

  return text;
}

// ============ Device ID Generation ============

#ifndef GENERATE_DEVICE_ID_DEFINED
/**
 * Generates unique device ID from ESP32 chip MAC address
 */
inline String generateDeviceId() {
  uint64_t chipid = ESP.getEfuseMac();
  char id[17];
  sprintf(id, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
  return String(id);
}
#define GENERATE_DEVICE_ID_DEFINED
#endif

// ============ Error Handling Helpers ============

/**
 * Logs error with consistent format
 */
inline void logError(ErrorCode code, const char* message) {
  Serial.printf("❌ [ERROR %d] %s\n", code, message);
}

/**
 * Logs warning with consistent format
 */
inline void logWarning(const char* message) {
  Serial.printf("⚠️ [WARNING] %s\n", message);
}

/**
 * Logs info with consistent format
 */
inline void logInfo(const char* message) {
  Serial.printf("ℹ️ [INFO] %s\n", message);
}

/**
 * Logs success with consistent format
 */
inline void logSuccess(const char* message) {
  Serial.printf("✅ [SUCCESS] %s\n", message);
}

// ============ Timing Helpers ============

/**
 * Checks if enough time has elapsed since last event
 */
inline bool hasElapsed(unsigned long lastTime, unsigned long interval) {
  return (millis() - lastTime) >= interval;
}

/**
 * Returns time elapsed since last event in milliseconds
 */
inline unsigned long elapsedTime(unsigned long lastTime) {
  return millis() - lastTime;
}

// ============ Animation Easing Functions ============

/**
 * Ease-out quadratic easing for smoother animations
 * @param t Current time/progress (0.0 to 1.0)
 * @return Eased value (0.0 to 1.0)
 */
inline float easeOutQuad(float t) {
  return t * (2.0f - t);
}

/**
 * Ease-in-out cubic easing for very smooth animations
 * @param t Current time/progress (0.0 to 1.0)
 * @return Eased value (0.0 to 1.0)
 */
inline float easeInOutCubic(float t) {
  if (t < 0.5f) {
    return 4.0f * t * t * t;
  } else {
    float f = (2.0f * t - 2.0f);
    return 0.5f * f * f * f + 1.0f;
  }
}

/**
 * Calculate eased animation offset for smooth scrolling
 * @param current Current offset
 * @param max Maximum offset
 * @return Eased offset value
 */
inline int getEasedOffset(int current, int max) {
  if (max == 0) return 0;
  float progress = (float)current / (float)max;
  float eased = easeOutQuad(progress);
  return (int)(eased * max);
}

#endif
