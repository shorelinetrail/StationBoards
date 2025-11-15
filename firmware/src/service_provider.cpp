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

bool NationalRailProvider::buildRequest(const char* stationCode, String& request) {
  if (!isValidStationCode(stationCode)) {
    Serial.println("❌ Invalid National Rail station code: " + String(stationCode));
    return false;
  }

  // Build SOAP request
  String soapRequest;
  soapRequest.reserve(512);
  soapRequest = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
  soapRequest += "<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\">";
  soapRequest += "<soap:Header><AccessToken xmlns=\"http://thalesgroup.com/RTTI/2013-11-28/Token/types\">";
  soapRequest += "<TokenValue>" + String(apiToken) + "</TokenValue></AccessToken></soap:Header>";
  soapRequest += "<soap:Body><GetDepartureBoardRequest xmlns=\"http://thalesgroup.com/RTTI/2016-02-16/ldb/\">";
  soapRequest += "<numRows>8</numRows><crs>" + String(stationCode) + "</crs>";
  soapRequest += "</GetDepartureBoardRequest></soap:Body></soap:Envelope>";

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
      services[serviceCount].callingPoints[0] = '\0';

      Serial.println("🚂 " + String(serviceCount + 1) + ": " + std + " → " + destination);
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

bool TflUndergroundProvider::buildRequest(const char* stationCode, String& request) {
  if (!isValidStationCode(stationCode)) {
    Serial.println("❌ Invalid TFL station code: " + String(stationCode));
    return false;
  }

  // Build TFL API request
  // GET /StopPoint/{stationCode}/Arrivals
  String path = "/StopPoint/" + String(stationCode) + "/Arrivals";

  // Add API key if configured
  if (apiKey.length() > 0) {
    path += "?app_key=" + apiKey;
  }

  request = "GET " + path + " HTTP/1.1\r\n";
  request += "Host: " + String(apiHost) + "\r\n";
  request += "Accept: application/json\r\n";
  request += "Connection: close\r\n\r\n";

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
    return false;
  }

  String jsonBody = response.substring(jsonStart);

  // Use ArduinoJson for parsing
  DynamicJsonDocument doc(16384);  // 16KB for JSON parsing
  DeserializationError error = deserializeJson(doc, jsonBody);

  if (error) {
    Serial.println("❌ JSON parse error: " + String(error.c_str()));
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
      String cleanName = String(stName);
      // Remove redundant suffix from TFL station names
      cleanName.replace(" Underground Station", "");
      cleanName.replace(" Rail Station", "");

      strncpy(stationName, cleanName.c_str(), stationNameSize - 1);
      stationName[stationNameSize - 1] = '\0';
      Serial.println("📍 " + cleanName);
    }
  }

  // Parse arrivals (max 8 services)
  int maxServices = min(8, (int)arrivals.size());

  for (int i = 0; i < maxServices && serviceCount < 8; i++) {
    JsonObject arrival = arrivals[i];

    const char* lineName = arrival["lineName"];
    const char* lineId = arrival["lineId"];
    const char* towards = arrival["towards"];
    const char* expectedArrival = arrival["expectedArrival"];
    int timeToStation = arrival["timeToStation"] | 0;

    if (!lineName || !towards || !expectedArrival) continue;

    // Filter by line if lineFilter is set
    if (lineFilter.length() > 0 && lineId) {
      String currentLineId = String(lineId);
      currentLineId.toLowerCase();
      String filterLower = lineFilter;
      filterLower.toLowerCase();

      if (currentLineId != filterLower) {
        continue;  // Skip this arrival, doesn't match filter
      }
    }

    // Filter by direction if directionFilter is set
    if (directionFilter.length() > 0 && towards) {
      String towardsStr = String(towards);
      towardsStr.toLowerCase();
      String dirFilterLower = directionFilter;
      dirFilterLower.toLowerCase();

      if (towardsStr.indexOf(dirFilterLower) == -1) {
        continue;  // Skip this arrival, doesn't match direction filter
      }
    }

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

    // Format destination as "Towards (Line)" - cleaner than arrow format
    String destination = String(towards) + " (" + String(lineName) + ")";

    // For TFL: STD is empty (no scheduled time), ETD shows minutes on the right
    // This matches National Rail format where time info is on the right
    services[serviceCount].std[0] = '\0';  // Empty STD for TFL
    etd.toCharArray(services[serviceCount].etd, sizeof(services[serviceCount].etd));
    destination.toCharArray(services[serviceCount].destination, sizeof(services[serviceCount].destination));
    services[serviceCount].callingPoints[0] = '\0';

    Serial.println("🚇 " + String(serviceCount + 1) + ": " + destination + " - " + etd);
    serviceCount++;
  }

  return serviceCount > 0;
}
