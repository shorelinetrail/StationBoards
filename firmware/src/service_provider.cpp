#include "service_provider.h"
#include <ArduinoJson.h>

// ============ National Rail Provider Implementation ============

String NationalRailProvider::extractTagValue(const String& xml, const String& tag, const String& ns) {
  String openTag = "<" + ns + ":" + tag + ">";
  String closeTag = "</" + ns + ":" + tag + ">";

  int startIdx = xml.indexOf(openTag);
  if (startIdx == -1) return "";

  startIdx += openTag.length();
  int endIdx = xml.indexOf(closeTag, startIdx);
  if (endIdx == -1) return "";

  return xml.substring(startIdx, endIdx);
}

String NationalRailProvider::decodeHTMLEntities(const String& text) {
  String result = text;
  result.replace("&amp;", "&");
  result.replace("&lt;", "<");
  result.replace("&gt;", ">");
  result.replace("&quot;", "\"");
  result.replace("&#39;", "'");
  return result;
}

bool NationalRailProvider::isValidStationCode(const char* code) {
  if (!code || strlen(code) != 3) return false;

  // Must be exactly 3 uppercase letters
  for (int i = 0; i < 3; i++) {
    if (!isalpha(code[i])) return false;
  }
  return true;
}

bool NationalRailProvider::buildRequest(const char* stationCode, String& request, bool useCallingAt) {
  if (!isValidStationCode(stationCode)) {
    Serial.println("❌ Invalid National Rail station code: " + String(stationCode));
    return false;
  }

  // Build SOAP request - use detailed API for calling points
  String soapRequest;
  soapRequest.reserve(512);
  soapRequest = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
  soapRequest += "<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\">";
  soapRequest += "<soap:Header><AccessToken xmlns=\"http://thalesgroup.com/RTTI/2013-11-28/Token/types\">";
  soapRequest += "<TokenValue>" + String(apiToken) + "</TokenValue></AccessToken></soap:Header>";

  if (useCallingAt) {
    // Use GetDepBoardWithDetails to get calling points data
    soapRequest += "<soap:Body><GetDepBoardWithDetailsRequest xmlns=\"http://thalesgroup.com/RTTI/2016-02-16/ldb/\">";
    soapRequest += "<numRows>8</numRows><crs>" + String(stationCode) + "</crs>";
    soapRequest += "</GetDepBoardWithDetailsRequest></soap:Body></soap:Envelope>";
  } else {
    // Use basic GetDepartureBoard when calling points not needed
    soapRequest += "<soap:Body><GetDepartureBoardRequest xmlns=\"http://thalesgroup.com/RTTI/2016-02-16/ldb/\">";
    soapRequest += "<numRows>8</numRows><crs>" + String(stationCode) + "</crs>";
    soapRequest += "</GetDepartureBoardRequest></soap:Body></soap:Envelope>";
  }

  // Build HTTP request
  request = "POST " + String(apiPath) + " HTTP/1.1\r\n";
  request += "Host: " + String(apiHost) + "\r\n";
  request += "Content-Type: text/xml\r\n";
  request += "Content-Length: " + String(soapRequest.length()) + "\r\n";
  request += "Connection: close\r\n\r\n";
  request += soapRequest;

  return true;
}

bool NationalRailProvider::parseResponse(const String& response,
                                         ServiceData* services,
                                         int& serviceCount,
                                         char* stationName,
                                         size_t stationNameSize,
                                         bool useCallingAt) {
  serviceCount = 0;

  Serial.println("📊 Processing National Rail response (" + String(response.length()) + " bytes)");

  // Check for SOAP fault
  if (response.indexOf("soap:Fault") != -1) {
    Serial.println("❌ SOAP Fault detected");
    String faultMsg = extractTagValue(response, "faultstring", "");
    if (faultMsg.length() > 0) {
      Serial.println("   Message: " + faultMsg);
    }
    return false;
  }

  // Extract station name
  String station = extractTagValue(response, "locationName", "lt4");
  if (station == "") station = extractTagValue(response, "locationName", "lt5");

  if (station.length() > 0) {
    strncpy(stationName, station.c_str(), stationNameSize - 1);
    stationName[stationNameSize - 1] = '\0';
    Serial.println("📍 " + station);
  }

  // Find train services section
  int servicesStart = response.indexOf("<lt5:trainServices>");
  if (servicesStart == -1) servicesStart = response.indexOf("<lt4:trainServices>");

  if (servicesStart == -1) {
    Serial.println("❌ No services tag found");
    return false;
  }

  int servicesEnd = response.indexOf("</lt5:trainServices>", servicesStart);
  if (servicesEnd == -1) servicesEnd = response.indexOf("</lt4:trainServices>", servicesStart);
  if (servicesEnd == -1) servicesEnd = response.length();

  String trainServices = response.substring(servicesStart, servicesEnd);

  String serviceTag = "<lt5:service>";
  String serviceEndTag = "</lt5:service>";

  int pos = 0;
  int maxServices = 8;

  // Parse each service
  while (serviceCount < maxServices) {
    int serviceStart = trainServices.indexOf(serviceTag, pos);
    if (serviceStart == -1) break;

    int serviceEnd = trainServices.indexOf(serviceEndTag, serviceStart);
    if (serviceEnd == -1) {
      int nextServiceStart = trainServices.indexOf(serviceTag, serviceStart + 1);
      if (nextServiceStart != -1) {
        serviceEnd = nextServiceStart;
      } else {
        serviceEnd = trainServices.length();
      }
    } else {
      serviceEnd += serviceEndTag.length();
    }

    String block = trainServices.substring(serviceStart, serviceEnd);

    String std = extractTagValue(block, "std", "lt4");
    if (std == "") std = extractTagValue(block, "std", "lt5");

    String etd = extractTagValue(block, "etd", "lt4");
    if (etd == "") etd = extractTagValue(block, "etd", "lt5");

    String destBlock = extractTagValue(block, "destination", "lt5");
    if (destBlock == "") destBlock = extractTagValue(block, "destination", "lt4");

    String destination = extractTagValue(destBlock, "locationName", "lt4");
    if (destination == "") destination = extractTagValue(destBlock, "locationName", "lt5");
    destination = decodeHTMLEntities(destination);

    if (std != "" && destination != "") {
      std.toCharArray(services[serviceCount].std, sizeof(services[serviceCount].std));
      etd.toCharArray(services[serviceCount].etd, sizeof(services[serviceCount].etd));
      destination.toCharArray(services[serviceCount].destination, sizeof(services[serviceCount].destination));

      // Parse calling points for first service only if useCallingAt is enabled
      if (useCallingAt && serviceCount == 0) {
        // Find subsequentCallingPoints section
        int cpListIdx = block.indexOf("<lt5:subsequentCallingPoints>");
        if (cpListIdx == -1) cpListIdx = block.indexOf("<lt4:subsequentCallingPoints>");

        if (cpListIdx != -1) {
          int cpListEndIdx = block.indexOf("</lt5:subsequentCallingPoints>", cpListIdx);
          if (cpListEndIdx == -1) cpListEndIdx = block.indexOf("</lt4:subsequentCallingPoints>", cpListIdx);

          if (cpListEndIdx != -1) {
            String cpSection = block.substring(cpListIdx, cpListEndIdx);

            // Find callingPointList inside subsequentCallingPoints
            int cpListStart = cpSection.indexOf("<lt4:callingPointList>");
            if (cpListStart == -1) cpListStart = cpSection.indexOf("<lt5:callingPointList>");

            if (cpListStart != -1) {
              int cpListEnd = cpSection.indexOf("</lt4:callingPointList>", cpListStart);
              if (cpListEnd == -1) cpListEnd = cpSection.indexOf("</lt5:callingPointList>", cpListStart);

              if (cpListEnd != -1) {
                String cpList = cpSection.substring(cpListStart, cpListEnd);

                // Determine which namespace to use
                String cpTag = "<lt4:callingPoint>";
                String cpEndTag = "</lt4:callingPoint>";
                if (cpList.indexOf(cpTag) == -1) {
                  cpTag = "<lt5:callingPoint>";
                  cpEndTag = "</lt5:callingPoint>";
                }

                String callingPoints = "";
                int cpPos = 0;

                // Extract each calling point
                while ((cpPos = cpList.indexOf(cpTag, cpPos)) != -1) {
                  int cpEnd = cpList.indexOf(cpEndTag, cpPos);
                  if (cpEnd == -1) break;

                  String cpBlock = cpList.substring(cpPos, cpEnd);

                  String cpName = extractTagValue(cpBlock, "locationName", "lt4");
                  if (cpName == "") cpName = extractTagValue(cpBlock, "locationName", "lt5");
                  cpName = decodeHTMLEntities(cpName);

                  String cpTime = extractTagValue(cpBlock, "st", "lt4");
                  if (cpTime == "") cpTime = extractTagValue(cpBlock, "st", "lt5");

                  if (cpName.length() > 0) {
                    if (callingPoints.length() > 0) callingPoints += ", ";
                    callingPoints += cpName;
                    if (cpTime.length() > 0) callingPoints += " (" + cpTime + ")";
                  }

                  cpPos = cpEnd;
                  yield();  // Allow other tasks during long lists
                }

                if (callingPoints.length() > 0) {
                  callingPoints.toCharArray(services[serviceCount].callingPoints, sizeof(services[serviceCount].callingPoints));
                } else {
                  services[serviceCount].callingPoints[0] = '\0';
                }
              } else {
                services[serviceCount].callingPoints[0] = '\0';
              }
            } else {
              services[serviceCount].callingPoints[0] = '\0';
            }
          } else {
            services[serviceCount].callingPoints[0] = '\0';
          }
        } else {
          services[serviceCount].callingPoints[0] = '\0';
        }
      } else {
        services[serviceCount].callingPoints[0] = '\0';
      }

      Serial.println("🚂 " + String(serviceCount + 1) + ": " + std + " → " + destination);

      // Log calling points if present for first service
      if (useCallingAt && serviceCount == 0 && strlen(services[serviceCount].callingPoints) > 0) {
        Serial.println("   Calling at: " + String(services[serviceCount].callingPoints));
      }

      serviceCount++;
    }

    pos = serviceEnd;
  }

  return serviceCount > 0;
}

// ============ TFL Underground Provider Implementation ============

TflUndergroundProvider::TflUndergroundProvider() {
  apiKey = "";
}

String TflUndergroundProvider::formatTime(const String& isoTimestamp) {
  // Parse ISO timestamp: "2025-01-15T10:45:00Z"
  // Extract time portion and format as HH:MM
  int tPos = isoTimestamp.indexOf('T');
  if (tPos == -1) return "";

  String timePart = isoTimestamp.substring(tPos + 1);
  int colonPos = timePart.indexOf(':');
  if (colonPos == -1) return "";

  String hours = timePart.substring(0, colonPos);
  String minutes = timePart.substring(colonPos + 1, colonPos + 3);

  return hours + ":" + minutes;
}

String TflUndergroundProvider::extractJsonValue(const String& json, const String& key) {
  // Simple JSON value extractor for strings
  String searchKey = "\"" + key + "\":\"";
  int startIdx = json.indexOf(searchKey);
  if (startIdx == -1) return "";

  startIdx += searchKey.length();
  int endIdx = json.indexOf("\"", startIdx);
  if (endIdx == -1) return "";

  return json.substring(startIdx, endIdx);
}

bool TflUndergroundProvider::isValidStationCode(const char* code) {
  if (!code) return false;

  int len = strlen(code);
  // TFL NaPTAN IDs are typically 9-12 characters
  // Format: 940GZZLU + 3-4 letter code (e.g., 940GZZLUPAC for Paddington)
  if (len < 9 || len > 12) return false;

  return true;
}

bool TflUndergroundProvider::buildRequest(const char* stationCode, String& request, bool useCallingAt) {
  if (!isValidStationCode(stationCode)) {
    Serial.println("❌ Invalid TFL station code: " + String(stationCode));
    return false;
  }

  // Build TFL API request
  // If line filter is set, use Line-specific endpoint for better filtering
  // Otherwise fall back to all arrivals at station
  String path;
  if (lineId.length() > 0) {
    // GET /Line/{lineId}/Arrivals/{stationCode}
    path = "/Line/" + lineId + "/Arrivals/" + String(stationCode);
    Serial.println("  🚇 Filtering by line: " + lineId);
  } else {
    // GET /StopPoint/{stationCode}/Arrivals
    path = "/StopPoint/" + String(stationCode) + "/Arrivals";
  }

  // Add API key if configured
  if (apiKey.length() > 0) {
    path += "?app_key=" + apiKey;
  }

  // Note: Direction filtering is done in parseResponse, not via API parameter
  // The TFL Arrivals API doesn't support a 'direction' query parameter
  if (direction.length() > 0) {
    Serial.println("  ➡️  Will filter by direction: " + direction);
  }

  request = "GET " + path + " HTTP/1.1\r\n";
  request += "Host: " + String(apiHost) + "\r\n";
  request += "Accept: application/json\r\n";
  request += "Connection: close\r\n\r\n";

  Serial.println("  🌐 API Path: " + path);
  return true;
}

bool TflUndergroundProvider::parseResponse(const String& response,
                                           ServiceData* services,
                                           int& serviceCount,
                                           char* stationName,
                                           size_t stationNameSize,
                                           bool useCallingAt) {
  serviceCount = 0;

  Serial.println("📊 Processing TFL Underground response (" + String(response.length()) + " bytes)");

  // Find JSON array start
  int jsonStart = response.indexOf('[');
  if (jsonStart == -1) {
    Serial.println("❌ No JSON array found in response");
    // Check if it's an error response
    int errorStart = response.indexOf('{');
    if (errorStart != -1) {
      String preview = response.substring(errorStart, min(errorStart + 200, (int)response.length()));
      Serial.println("📄 Response preview: " + preview);
    }
    return false;
  }

  String jsonBody = response.substring(jsonStart);
  Serial.println("📏 JSON body size: " + String(jsonBody.length()) + " bytes");

  // Use ArduinoJson for parsing
  // TFL responses can be large (30KB+), increase to 40KB buffer
  DynamicJsonDocument doc(40960);  // 40KB for JSON parsing
  DeserializationError error = deserializeJson(doc, jsonBody);

  if (error) {
    Serial.println("❌ JSON parse error: " + String(error.c_str()));
    Serial.println("📄 First 500 chars of JSON: " + jsonBody.substring(0, min(500, (int)jsonBody.length())));
    return false;
  }

  JsonArray arrivals = doc.as<JsonArray>();
  if (arrivals.size() == 0) {
    Serial.println("❌ No arrivals found");
    return false;
  }

  // Extract station name from first arrival
  if (arrivals.size() > 0) {
    const char* stName = arrivals[0]["stationName"];
    if (stName) {
      strncpy(stationName, stName, stationNameSize - 1);
      stationName[stationNameSize - 1] = '\0';
      Serial.println("📍 " + String(stName));
    }
  }

  // Parse arrivals (max 8 services), applying direction filter if set
  int processedCount = 0;

  for (int i = 0; i < arrivals.size() && serviceCount < 8; i++) {
    JsonObject arrival = arrivals[i];

    const char* lineName = arrival["lineName"];
    const char* towards = arrival["towards"];
    const char* expectedArrival = arrival["expectedArrival"];
    const char* arrivalDirection = arrival["direction"];
    int timeToStation = arrival["timeToStation"] | 0;

    if (!lineName || !towards || !expectedArrival) continue;

    // Apply direction filter if set
    if (direction.length() > 0 && arrivalDirection) {
      // TFL API uses "inbound" and "outbound" in lowercase
      if (direction != String(arrivalDirection)) {
        continue;  // Skip arrivals that don't match the direction filter
      }
    }

    // Format scheduled time from expectedArrival
    String scheduledTime = formatTime(String(expectedArrival));

    // Format ETD (estimated time in minutes)
    String etd;
    int minutes = timeToStation / 60;
    if (minutes == 0) {
      etd = "Due";
    } else if (minutes == 1) {
      etd = "1 min";
    } else {
      etd = String(minutes) + " min";
    }

    // Format destination (just the "towards" direction, no line name)
    String destination = String(towards);

    // Populate service data
    scheduledTime.toCharArray(services[serviceCount].std, sizeof(services[serviceCount].std));
    etd.toCharArray(services[serviceCount].etd, sizeof(services[serviceCount].etd));
    destination.toCharArray(services[serviceCount].destination, sizeof(services[serviceCount].destination));
    services[serviceCount].callingPoints[0] = '\0';

    Serial.println("🚇 " + String(serviceCount + 1) + ": " + scheduledTime + " " + destination + " (" + etd + ")");
    serviceCount++;
    processedCount++;
  }

  if (direction.length() > 0) {
    Serial.println("✅ Filtered " + String(serviceCount) + " arrivals (direction: " + direction + ") from " + String(arrivals.size()) + " total");
  } else {
    Serial.println("✅ Processed " + String(serviceCount) + " arrivals");
  }

  return serviceCount > 0;
}
