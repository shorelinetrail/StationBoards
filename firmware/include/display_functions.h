#ifndef DISPLAY_FUNCTIONS_H
#define DISPLAY_FUNCTIONS_H

#include <U8g2lib.h>
#include "types.h"
#include "constants.h"
#include "helpers.h"

// Forward declaration for external display object
extern U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2;
extern Config config;

// ============ Display Component Functions ============

/**
 * Displays the station name centered at the top of the screen
 * Only called if config.showStationName is true
 */
inline void displayStationName(const char* stationName) {
  String displayName = stationName;
  int nameWidth = u8g2.getUTF8Width(displayName.c_str());

  // Truncate if too wide (allow some margin)
  if (nameWidth > Display::WIDTH - 6) {
    while (u8g2.getUTF8Width(displayName.c_str()) > Display::WIDTH - 6 && displayName.length() > 3) {
      displayName.remove(displayName.length() - 1);
    }
    displayName += "...";
    nameWidth = u8g2.getUTF8Width(displayName.c_str());
  }

  u8g2.setCursor((Display::WIDTH - nameWidth) / 2, 12);
  u8g2.print(displayName);
}

/**
 * Displays a single service line with time, destination, and ETD
 */
inline void displayServiceLine(const ServiceData& service, const char* label,
                                int yPos, U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI& display) {
  // Build left side differently for TFL vs National Rail
  String leftSide;

  if (service.std[0] == '\0') {
    // TFL: Extract just the number from "1st ", "2nd ", etc. and use plain number
    // "1st " -> "1 ", "2nd " -> "2 ", "3rd " -> "3 "
    String labelStr = String(label);
    int numStart = 0;
    int numEnd = 0;
    for (int i = 0; i < labelStr.length(); i++) {
      if (isdigit(labelStr[i])) {
        if (numEnd == 0) numStart = i;
        numEnd = i + 1;
      }
    }
    leftSide = labelStr.substring(numStart, numEnd) + " ";
  } else {
    // National Rail: Use ordinal label + time
    leftSide = String(label) + String(service.std) + " ";
  }

  String rightSide = formatETD(String(service.etd));

  int leftWidth = display.getUTF8Width(leftSide.c_str());
  int rightWidth = display.getUTF8Width(rightSide.c_str());
  int availableWidth = Display::WIDTH - leftWidth - rightWidth - Display::TEXT_SPACING;

  String destination = fitTextToWidth(String(service.destination), availableWidth, display);

  display.setCursor(1, yPos);
  display.print(leftSide + destination);
  display.setCursor(Display::ETD_RIGHT_X - rightWidth, yPos);
  display.print(rightSide);
}

/**
 * Displays the "No trains scheduled" or "Loading..." message
 */
inline void displayNoServicesMessage(bool fetchingNewStation) {
  u8g2.setFont(u8g2_font_helvB10_tr);
  String msg = fetchingNewStation ? "Loading station data..." : "No trains scheduled";
  int msgWidth = u8g2.getUTF8Width(msg.c_str());
  u8g2.setCursor((Display::WIDTH - msgWidth) / 2, 35);
  u8g2.print(msg);
}

/**
 * Displays calling points with horizontal scrolling
 * Returns true if scrolling is active
 */
inline bool displayCallingPoints(const char* callingPoints, int yPos,
                                   int& scrollOffset, unsigned long& lastScroll) {
  if (strlen(callingPoints) == 0) {
    u8g2.setCursor(1, yPos);
    u8g2.print("Calling at: Loading stops...");
    scrollOffset = 0;
    return false;
  }

  String label = "Calling at: ";
  int labelWidth = u8g2.getUTF8Width(label.c_str());

  String callingText = String(callingPoints);
  String loopingText = callingText + " * " + callingText;

  int fullTextWidth = u8g2.getUTF8Width(callingText.c_str());
  int availableSpace = Display::WIDTH - labelWidth - 5;

  bool needsScroll = fullTextWidth > availableSpace;

  if (needsScroll) {
    unsigned long currentTime = millis();
    if (currentTime - lastScroll > config.scrollSpeed) {
      scrollOffset++;
      lastScroll = currentTime;
      // Note: Scrolling marks display dirty automatically via the loop
    }

    if (scrollOffset > fullTextWidth + 15) {
      scrollOffset = 0;
    }

    u8g2.setCursor(1, yPos);
    u8g2.print(label);

    u8g2.setClipWindow(labelWidth + 2, 0, Display::CLIP_WINDOW_END, Display::HEIGHT - 1);
    u8g2.setCursor(labelWidth + 2 - scrollOffset, yPos);
    u8g2.print(loopingText);
    u8g2.setMaxClipWindow();

    return true;
  } else {
    u8g2.setCursor(1, yPos);
    u8g2.print(label);
    u8g2.print(callingText);
    scrollOffset = 0;
    return false;
  }
}

/**
 * Displays two services with animation between them (for alternating line)
 */
inline void displayAlternatingServices(const ServiceData& serviceA, const ServiceData& serviceB,
                                        const char* labelA, const char* labelB,
                                        int baselineY, int animOffset, bool isAnimating) {
  // Calculate text metrics
  int ascent = u8g2.getAscent();
  int descent = u8g2.getDescent();
  int textHeight = ascent - descent;

  // Build service A
  String leftA = String(labelA) + String(serviceA.std) + " ";
  String rightA = formatETD(String(serviceA.etd));
  int leftAWidth = u8g2.getUTF8Width(leftA.c_str());
  int rightAWidth = u8g2.getUTF8Width(rightA.c_str());
  int availA = Display::WIDTH - leftAWidth - rightAWidth - Display::TEXT_SPACING;
  String destA = fitTextToWidth(String(serviceA.destination), availA, u8g2);

  // Set clip window for this line
  u8g2.setClipWindow(0, baselineY - ascent, Display::CLIP_WINDOW_END, baselineY - descent);

  if (isAnimating) {
    // Draw current service scrolling up
    u8g2.setCursor(1, baselineY - animOffset);
    u8g2.print(leftA + destA);
    u8g2.setCursor(Display::ETD_RIGHT_X - rightAWidth, baselineY - animOffset);
    u8g2.print(rightA);

    // Draw next service scrolling up from below
    String leftB = String(labelB) + String(serviceB.std) + " ";
    String rightB = formatETD(String(serviceB.etd));
    int leftBWidth = u8g2.getUTF8Width(leftB.c_str());
    int rightBWidth = u8g2.getUTF8Width(rightB.c_str());
    int availB = Display::WIDTH - leftBWidth - rightBWidth - Display::TEXT_SPACING;
    String destB = fitTextToWidth(String(serviceB.destination), availB, u8g2);

    u8g2.setCursor(1, baselineY + textHeight - animOffset);
    u8g2.print(leftB + destB);
    u8g2.setCursor(Display::ETD_RIGHT_X - rightBWidth, baselineY + textHeight - animOffset);
    u8g2.print(rightB);
  } else {
    // Draw static service
    u8g2.setCursor(1, baselineY);
    u8g2.print(leftA + destA);
    u8g2.setCursor(Display::ETD_RIGHT_X - rightAWidth, baselineY);
    u8g2.print(rightA);
  }

  u8g2.setMaxClipWindow();
}

/**
 * Displays the current time at the bottom center of the screen
 */
inline void displayClock() {
  time_t now = time(nullptr);
  if (now > 100000) {  // Check if time is synced
    struct tm* timeInfo = localtime(&now);
    char timeString[9];
    strftime(timeString, sizeof(timeString), "%H:%M:%S", timeInfo);

    u8g2.setFont(u8g2_font_t0_11_tf);
    int width = u8g2.getUTF8Width(timeString);
    u8g2.setCursor((Display::WIDTH - width) / 2, Display::HEIGHT);
    u8g2.print(timeString);
  }
}

/**
 * Gets the service label based on index and offset
 * e.g., getServiceLabel(2, 0) = "3rd ", getServiceLabel(2, 1) = "4th "
 */
inline String getServiceLabel(int index, int offset) {
  int displayNum = index + 1;

  if (offset == 1) displayNum++;  // Adjust for hidden station name

  switch (displayNum) {
    case 1: return "1st ";
    case 2: return "2nd ";
    case 3: return "3rd ";
    case 4: return "4th ";
    case 5: return "5th ";
    case 6: return "6th ";
    case 7: return "7th ";
    case 8: return "8th ";
    default: return String(displayNum) + "th ";
  }
}

/**
 * Calculates service offset based on whether station name is shown
 * When station name is hidden, we show an extra service at top, shifting indices
 */
inline int getServiceOffset(bool showStationName) {
  return showStationName ? 0 : 1;
}

/**
 * Calculates the starting index for alternating services based on display mode
 */
inline int getAlternatingStartIndex(bool useCallingAt, bool showStationName) {
  int offset = getServiceOffset(showStationName);

  if (useCallingAt) {
    return 1 + offset;  // Calling at mode: after first service + calling points
  } else {
    return 2 + offset;  // Standard mode: after first two services
  }
}

/**
 * Calculates the maximum index for alternating services
 */
inline int getAlternatingMaxIndex(bool useCallingAt, int extraServices, bool showStationName) {
  int offset = getServiceOffset(showStationName);

  if (useCallingAt) {
    return extraServices + offset;
  } else {
    return extraServices + 1 + offset;
  }
}

/**
 * Calculates minimum services needed to enable alternating display
 */
inline int getMinServicesForAlternating(bool useCallingAt, int extraServices, bool showStationName) {
  int offset = getServiceOffset(showStationName);

  if (useCallingAt) {
    // Need at least 2 services on bottom line to rotate
    return extraServices + 2 + offset;
  } else {
    // Standard mode: need at least 2 services on bottom line
    return extraServices + 3 + offset;
  }
}

#endif
