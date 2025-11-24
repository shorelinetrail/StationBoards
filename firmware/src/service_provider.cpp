#include "service_provider.h"
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

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
  // Use GetDepBoardWithDetailsRequest to include calling points (subsequentCallingPoints)
  String soapRequest;
  soapRequest.reserve(512);
  soapRequest = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
  soapRequest += "<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\">";
  soapRequest += "<soap:Header><AccessToken xmlns=\"http://thalesgroup.com/RTTI/2013-11-28/Token/types\">";
  soapRequest += "<TokenValue>" + String(apiToken) + "</TokenValue></AccessToken></soap:Header>";
  soapRequest += "<soap:Body><GetDepBoardWithDetailsRequest xmlns=\"http://thalesgroup.com/RTTI/2016-02-16/ldb/\">";
  soapRequest += "<numRows>8</numRows><crs>" + String(stationCode) + "</crs>";
  soapRequest += "</GetDepBoardWithDetailsRequest></soap:Body></soap:Envelope>";

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

    // Extract fault details - support both SOAP 1.1 and SOAP 1.2 formats
    // SOAP 1.1: <faultcode> and <faultstring>
    // SOAP 1.2: <soap:Code><soap:Value> and <soap:Reason><soap:Text>
    String faultCode = extractTagValue(response, "faultcode", "");
    String faultString = extractTagValue(response, "faultstring", "");

    // Try with soap prefix if no namespace version failed
    if (faultCode.length() == 0) faultCode = extractTagValue(response, "faultcode", "soap");
    if (faultString.length() == 0) faultString = extractTagValue(response, "faultstring", "soap");

    // SOAP 1.2 format
    if (faultCode.length() == 0) faultCode = extractTagValue(response, "Value", "soap");
    if (faultString.length() == 0) faultString = extractTagValue(response, "Text", "soap");

    if (faultCode.length() > 0) {
      Serial.println("   Code: " + faultCode);
    }
    if (faultString.length() > 0) {
      Serial.println("   Message: " + faultString);
    }

    // If we couldn't extract structured fault info, show raw fault section
    if (faultCode.length() == 0 && faultString.length() == 0) {
      int faultStart = response.indexOf("soap:Fault");
      if (faultStart != -1) {
        int faultEnd = response.indexOf("</soap:Fault>", faultStart);
        if (faultEnd != -1) {
          faultEnd += 13; // Include closing tag
          String faultSection = response.substring(faultStart, faultEnd);
          Serial.println("   Raw fault (first 500 chars): " + faultSection.substring(0, min(500, (int)faultSection.length())));
        }
      }
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
    Serial.println("ℹ️ No services tag found - station has no departures");
    serviceCount = 0;
    return true;  // Valid response, just no services available
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

      // Parse calling points for first service if in calling at mode
      if (useCallingAt && serviceCount == 0) {
        Serial.println("📍 Attempting to parse calling points (useCallingAt=true, serviceCount=0)");

        int cpListIdx = block.indexOf("<lt5:subsequentCallingPoints>");
        if (cpListIdx == -1) cpListIdx = block.indexOf("<lt4:subsequentCallingPoints>");

        if (cpListIdx == -1) {
          Serial.println("❌ No <subsequentCallingPoints> tag found");
          Serial.println("📄 Block size: " + String(block.length()) + " bytes");
          Serial.println("📄 First 300 chars: " + block.substring(0, min(300, (int)block.length())));
        } else {
          Serial.println("✅ Found <subsequentCallingPoints> at position " + String(cpListIdx));
        }

        if (cpListIdx != -1) {
          int cpListEndIdx = block.indexOf("</lt5:subsequentCallingPoints>", cpListIdx);
          if (cpListEndIdx == -1) cpListEndIdx = block.indexOf("</lt4:subsequentCallingPoints>", cpListIdx);

          if (cpListEndIdx == -1) {
            Serial.println("❌ No closing </subsequentCallingPoints> tag found");
          } else {
            Serial.println("✅ Found closing tag at position " + String(cpListEndIdx));
          }

          if (cpListEndIdx != -1) {
            String cpSection = block.substring(cpListIdx, cpListEndIdx);
            Serial.println("📄 Calling points section size: " + String(cpSection.length()) + " bytes");

            int cpListStart = cpSection.indexOf("<lt4:callingPointList>");
            if (cpListStart == -1) cpListStart = cpSection.indexOf("<lt5:callingPointList>");

            if (cpListStart == -1) {
              Serial.println("❌ No <callingPointList> tag found");
            } else {
              Serial.println("✅ Found <callingPointList> at position " + String(cpListStart));
            }

            if (cpListStart != -1) {
              int cpListEnd = cpSection.indexOf("</lt4:callingPointList>", cpListStart);
              if (cpListEnd == -1) cpListEnd = cpSection.indexOf("</lt5:callingPointList>", cpListStart);

              if (cpListEnd != -1) {
                String cpList = cpSection.substring(cpListStart, cpListEnd);

                String cpTag = "<lt4:callingPoint>";
                String cpEndTag = "</lt4:callingPoint>";

                if (cpList.indexOf(cpTag) == -1) {
                  cpTag = "<lt5:callingPoint>";
                  cpEndTag = "</lt5:callingPoint>";
                }

                String callingPoints = "";
                int cpPos = 0;

                while ((cpPos = cpList.indexOf(cpTag, cpPos)) != -1) {
                  int cpEnd = cpList.indexOf(cpEndTag, cpPos);
                  if (cpEnd == -1) break;

                  String cpBlock = cpList.substring(cpPos, cpEnd);

                  String cpName = extractTagValue(cpBlock, "locationName", "lt4");
                  if (cpName == "") cpName = extractTagValue(cpBlock, "locationName", "lt5");
                  cpName = decodeHTMLEntities(cpName);

                  String cpTime = extractTagValue(cpBlock, "st", "lt4");
                  if (cpTime == "") cpTime = extractTagValue(cpBlock, "st", "lt5");

                  if (cpName != "") {
                    if (callingPoints != "") callingPoints += ", ";
                    callingPoints += cpName;
                    if (cpTime != "") callingPoints += " (" + cpTime + ")";
                  }

                  cpPos = cpEnd;

                  // Allow other tasks to run during long calling points lists
                  yield();
                }

                if (callingPoints != "" && callingPoints.length() < 500) {
                  callingPoints.toCharArray(services[serviceCount].callingPoints, 500);
                  Serial.println("  ✅ Calling at: " + callingPoints);
                } else if (callingPoints == "") {
                  String fallback = "No further stops available";
                  fallback.toCharArray(services[serviceCount].callingPoints, 500);
                  Serial.println("  ⚠️  Empty calling points list");
                } else {
                  Serial.println("  ⚠️  Calling points too long (" + String(callingPoints.length()) + " chars), truncating");
                  callingPoints = callingPoints.substring(0, 499);
                  callingPoints.toCharArray(services[serviceCount].callingPoints, 500);
                  Serial.println("  ✅ Calling at (truncated): " + callingPoints);
                }
              }
            }
          }
        }
      }

      serviceCount++;
    }

    pos = serviceEnd;
  }

  if (serviceCount == 0) {
    Serial.println("⚠️  No train services found in response");
  }

  return true;  // Return true for valid response, even if 0 services
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

String TflUndergroundProvider::cleanStationName(const String& name) {
  // Remove "Underground Station" suffix from TFL station names
  // E.g., "King's Cross St. Pancras Underground Station" → "King's Cross St. Pancras"
  String cleaned = name;

  // Check for " Underground Station" suffix
  int suffixPos = cleaned.indexOf(" Underground Station");
  if (suffixPos != -1) {
    cleaned = cleaned.substring(0, suffixPos);
  }

  return cleaned;
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
  // TFL accepts various formats:
  // - Hub codes: 6 characters (e.g., HUBSOK for South Kenton)
  // - NaPTAN IDs: 9-12 characters (e.g., 940GZZLUPAC for Paddington)
  if (len < 4 || len > 12) return false;

  return true;
}

bool TflUndergroundProvider::fetchStationName(const char* stationCode) {
  // Quick fetch of station name from TFL API
  WiFiClientSecure client;
  client.setInsecure();  // Skip cert validation for speed

  if (!client.connect(apiHost, 443)) {
    Serial.println("⚠️  Failed to connect for station name fetch");
    return false;
  }

  // Build station info request
  String path = "/StopPoint/" + String(stationCode);
  if (apiKey.length() > 0) {
    path += "?app_key=" + apiKey;
  }

  String req = "GET " + path + " HTTP/1.1\r\n";
  req += "Host: " + String(apiHost) + "\r\n";
  req += "Connection: close\r\n\r\n";

  client.print(req);

  // Wait for response with timeout
  unsigned long start = millis();
  while (!client.available() && millis() - start < 5000) {
    delay(10);
  }

  if (!client.available()) {
    client.stop();
    return false;
  }

  // Read response with timeout (max 10 seconds total read time)
  String response = "";
  unsigned long readStart = millis();
  const unsigned long maxReadTime = 10000;  // 10 second timeout for reading

  while (millis() - readStart < maxReadTime) {
    if (client.available()) {
      char c = client.read();
      response += c;

      // Stop reading once we have the closing brace after commonName
      if (response.indexOf("\"commonName\"") > 0 && c == '}') {
        break;  // We have enough data
      }
    } else {
      delay(10);
    }
  }
  client.stop();

  if (response.length() == 0) {
    Serial.println("⚠️  No response data received");
    return false;
  }

  // Find JSON body
  int jsonStart = response.indexOf('{');
  if (jsonStart == -1) return false;

  String jsonBody = response.substring(jsonStart);

  // Parse for commonName using simple string search
  int nameStart = jsonBody.indexOf("\"commonName\":\"");
  if (nameStart == -1) return false;

  nameStart += 14;  // Length of "commonName":"
  int nameEnd = jsonBody.indexOf("\"", nameStart);
  if (nameEnd == -1) return false;

  // Clean up station name (remove "Underground Station" suffix)
  String rawName = jsonBody.substring(nameStart, nameEnd);
  currentStationName = cleanStationName(rawName);
  Serial.println("📍 Fetched station name: " + currentStationName);

  return true;
}

void TflUndergroundProvider::ensureStationNameCached(const char* stationCode) {
  // Only fetch station name if this is a new station (not on every refresh)
  if (lastFetchedStationCode != String(stationCode)) {
    Serial.println("🆕 New station detected - fetching station name");
    if (fetchStationName(stationCode)) {
      lastFetchedStationCode = String(stationCode);
    } else {
      Serial.println("⚠️  Failed to fetch station name, will use code as fallback");
    }
  } else {
    Serial.println("♻️  Using cached station name: " + currentStationName);
  }
}

bool TflUndergroundProvider::buildRequest(const char* stationCode, String& request) {
  if (!isValidStationCode(stationCode)) {
    Serial.println("❌ Invalid TFL station code: " + String(stationCode));
    return false;
  }

  // Store station code for use as fallback station name
  currentStationCode = String(stationCode);

  // NOTE: Station name is now pre-fetched in main.cpp before opening connection
  // to avoid blocking while the API connection is open

  // Build TFL API request - HYBRID approach for best performance
  // When filter active: Use Line API (small response, reliable parsing)
  // When no filter: Use StopPoint API (need all lines, bigger response)
  String path;

  if (lineFilter.length() > 0) {
    // Use Line API for filtered requests - small response (~5KB)
    path = "/Line/" + lineFilter + "/Arrivals/" + String(stationCode);
    Serial.println("🚇 Line API (filtered): /Line/" + lineFilter + "/Arrivals/" + String(stationCode));
  } else {
    // Use StopPoint API for unfiltered requests - all lines (~40KB)
    path = "/StopPoint/" + String(stationCode) + "/Arrivals";
    Serial.println("🚇 StopPoint API (all lines): /StopPoint/" + String(stationCode) + "/Arrivals");
  }

  // Add API key as query parameter
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

  // Find where HTTP headers end (blank line)
  int headerEnd = response.indexOf("\r\n\r\n");
  if (headerEnd == -1) {
    headerEnd = response.indexOf("\n\n");  // Try \n\n for non-standard responses
  }

  int searchStart = (headerEnd != -1) ? headerEnd + 4 : 0;
  Serial.println("🔍 Searching for JSON from position: " + String(searchStart));

  // Find JSON array start after headers
  int jsonStart = response.indexOf('[', searchStart);
  if (jsonStart == -1) {
    Serial.println("❌ No JSON array found in response");
    Serial.println("First 500 chars: " + response.substring(0, 500));
    return false;
  }

  size_t jsonLength = response.length() - jsonStart;
  Serial.println("📍 JSON starts at position: " + String(jsonStart) + ", length: " + String(jsonLength) + " bytes");

  // MEMORY OPTIMIZATION: Extract JSON substring to free HTTP headers from memory
  // This reduces memory pressure before large DynamicJsonDocument allocation
  Serial.println("🔄 Extracting JSON substring to free headers (" + String(jsonStart) + " bytes)");
  String jsonOnly = response.substring(jsonStart);

  // Note: Can't modify const response, but substring creates new String with just JSON
  // Original response buffer remains but will be cleaned up by caller
  yield();

  Serial.print("💾 Free heap after substring: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");

  // Use zero-copy parsing with pointer
  const char* jsonStart_ptr = jsonOnly.c_str();

  // Create a filter to only parse fields we ACTUALLY USE (dramatically reduces memory usage)
  // TFL JSON has TONS of fields we don't use: currentLocation, vehicleId, bearing, etc.
  // By filtering, we can use much smaller documents and avoid heap fragmentation
  //
  // OPTIMIZED: Removed lineName (not used) and expectedArrival (not used)
  // This reduces filtered data by ~25%
  StaticJsonDocument<150> filter;
  filter[0]["stationName"] = true;    // Station name (first arrival only, for display)
  filter[0]["lineId"] = true;         // e.g., "northern" (for line filtering)
  filter[0]["towards"] = true;        // e.g., "Edgware" (destination display)
  filter[0]["direction"] = true;      // "inbound" or "outbound" (for direction filtering)
  filter[0]["timeToStation"] = true;  // Seconds until arrival (for sorting and ETD calculation)
  filter[0]["platformName"] = true;   // e.g., "Eastbound - Platform 5" (for platform filtering)

  // With filtering, we extract only 6 fields per arrival (down from 8), so the filtered
  // document is much smaller than the source JSON. However, major stations can still
  // return large responses even with line filtering.
  //
  // HEAP FRAGMENTATION STRATEGY: Start with smaller size that can fit in fragmented heap,
  // then progressively retry with larger sizes if IncompleteInput
  // Optimized filtering may allow smaller initial allocation
  size_t docSize = 12288;  // Start with 12KB (reduced from 16KB due to 25% less fields)

  unsigned long freeHeap = ESP.getFreeHeap();
  Serial.print("💾 Free heap: ");
  Serial.print(freeHeap);
  Serial.println(" bytes");
  Serial.println("📦 Allocating " + String(docSize) + " byte JSON document (filtered parsing)");

  if (freeHeap < 80000) {
    Serial.println("⚠️  WARNING: Low heap memory - allocation may fail due to fragmentation");
  }

  // Force garbage collection before large allocation
  yield();
  delay(10);

  DynamicJsonDocument doc(docSize);
  DeserializationError error = deserializeJson(doc, jsonStart_ptr, DeserializationOption::Filter(filter));

  // Retry logic for IncompleteInput or InvalidInput - increase size incrementally
  // Note: NoMemory errors are NOT retried - they indicate heap fragmentation
  // Progressive sizing with optimized filtering: 12KB → 16KB → 20KB → 24KB (4KB increments)
  int retryCount = 0;
  while (error && (error.code() == DeserializationError::IncompleteInput ||
                   error.code() == DeserializationError::InvalidInput) && retryCount < 3) {
    retryCount++;
    size_t newSize = docSize + 4096; // Add 4KB per retry
    if (newSize > 24576) newSize = 24576; // Cap at 24KB (reduced due to better filtering)

    Serial.println("⚠️  " + String(error.c_str()) + " error - retry #" + String(retryCount) + " with " + String(newSize) + " bytes");
    Serial.print("💾 Free heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");

    // Yield to help with garbage collection between attempts
    yield();
    delay(10);

    DynamicJsonDocument retryDoc(newSize);
    error = deserializeJson(retryDoc, jsonStart_ptr, DeserializationOption::Filter(filter));

    if (!error) {
      Serial.println("✅ Retry successful");
      doc = retryDoc;
      docSize = newSize;
      break;
    }
    docSize = newSize;
  }

  if (error) {
    Serial.println("❌ JSON parse error: " + String(error.c_str()) + " (code: " + String((int)error.code()) + ")");
    Serial.println("❌ JSON length: " + String(jsonLength) + " bytes, doc size: " + String(docSize) + " bytes");
    Serial.print("💾 Free heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");

    if (error.code() == DeserializationError::NoMemory) {
      Serial.println("❌ Out of memory - severe heap fragmentation detected");
      Serial.println("💡 Suggestions:");
      Serial.println("   1. Reduce line/platform filters to get smaller responses");
      Serial.println("   2. Device may need restart to defragment heap");
      Serial.println("   3. Try fetching during quieter times (fewer arrivals)");
    }

    // Show first 100 chars without String allocation to avoid memory issues
    char preview[101];
    strncpy(preview, jsonStart_ptr, 100);
    preview[100] = '\0';
    Serial.println("❌ JSON preview: " + String(preview));
    return false;
  }

  Serial.println("✅ JSON parsed successfully");

  JsonArray arrivals = doc.as<JsonArray>();

  // Extract station name - prefer from arrival data, fall back to cached name, then station code
  bool stationNameSet = false;

  if (arrivals.size() > 0) {
    const char* stName = arrivals[0]["stationName"];
    if (stName) {
      // Clean up station name (remove "Underground Station" suffix)
      String cleanedName = cleanStationName(String(stName));
      strncpy(stationName, cleanedName.c_str(), stationNameSize - 1);
      stationName[stationNameSize - 1] = '\0';
      // Cache the cleaned station name for future zero-arrival responses
      currentStationName = cleanedName;
      Serial.println("📍 " + cleanedName + " (from arrival data)");
      stationNameSet = true;
    }
  }

  // If we didn't get station name from arrivals, use cached name from previous fetch
  if (!stationNameSet) {
    if (currentStationName.length() > 0) {
      strncpy(stationName, currentStationName.c_str(), stationNameSize - 1);
      stationName[stationNameSize - 1] = '\0';
      Serial.println("📍 " + currentStationName + " (cached from previous fetch)");
      stationNameSet = true;
    } else if (currentStationCode.length() > 0) {
      strncpy(stationName, currentStationCode.c_str(), stationNameSize - 1);
      stationName[stationNameSize - 1] = '\0';
      Serial.println("📍 " + currentStationCode + " (station code fallback)");
      stationNameSet = true;
    }
  }

  if (arrivals.size() == 0) {
    if (lineFilter.length() > 0) {
      Serial.println("⚠️  No arrivals in TFL response - line '" + lineFilter + "' may not serve this station");
    } else {
      Serial.println("⚠️  No arrivals in TFL response");
    }
  }

  // Sort arrivals by timeToStation (TFL API doesn't guarantee order)
  // Create array of indices sorted by timeToStation
  const int maxArrivals = arrivals.size();
  if (maxArrivals > 100) {
    Serial.println("⚠️  Warning: Too many arrivals (" + String(maxArrivals) + "), limiting to 100");
  }

  const int arrivalLimit = min(maxArrivals, 100);
  int sortedIndices[100];  // Max 100 arrivals to sort

  // Initialize indices
  for (int i = 0; i < arrivalLimit; i++) {
    sortedIndices[i] = i;
  }

  // Bubble sort indices by timeToStation (simple but works for small arrays)
  for (int i = 0; i < arrivalLimit - 1; i++) {
    for (int j = 0; j < arrivalLimit - i - 1; j++) {
      int timeA = arrivals[sortedIndices[j]]["timeToStation"] | 0;
      int timeB = arrivals[sortedIndices[j + 1]]["timeToStation"] | 0;
      if (timeA > timeB) {
        // Swap indices
        int temp = sortedIndices[j];
        sortedIndices[j] = sortedIndices[j + 1];
        sortedIndices[j + 1] = temp;
      }
    }
  }

  // Parse arrivals in sorted order (max 8 services)
  int maxServices = 8;
  int arrivalsIndex = 0;

  while (serviceCount < maxServices && arrivalsIndex < arrivalLimit) {
    JsonObject arrival = arrivals[sortedIndices[arrivalsIndex++]];

    // Only extract fields we actually use (optimized from 8 to 6 fields)
    const char* lineId = arrival["lineId"];
    const char* towards = arrival["towards"];
    const char* direction = arrival["direction"];
    const char* platformName = arrival["platformName"];
    int timeToStation = arrival["timeToStation"] | 0;

    // Skip if essential fields are missing
    if (!towards || !lineId) continue;

    // Filter by line if a line filter is set (client-side filtering)
    if (lineFilter.length() > 0) {
      // Compare against lineId (e.g., "northern", "circle")
      if (lineId && String(lineId) != lineFilter) {
        continue;  // Skip this arrival, doesn't match line filter
      } else if (!lineId) {
        continue;  // No lineId, skip it
      }
    }

    // Filter by direction if a direction filter is set (client-side filtering)
    if (directionFilter.length() > 0) {
      // Compare against direction (e.g., "inbound", "outbound")
      if (direction && String(direction) != directionFilter) {
        continue;  // Skip this arrival, doesn't match filter
      } else if (!direction) {
        continue;  // No direction, skip it
      }
    }

    // Filter by platform if a platform filter is set (client-side filtering)
    if (platformFilter.length() > 0) {
      // Compare against platformName (e.g., "Eastbound - Platform 5")
      if (platformName && String(platformName) != platformFilter) {
        continue;  // Skip this arrival, doesn't match platform filter
      } else if (!platformName) {
        continue;  // No platformName, skip it
      }
    }

    // For TFL: Leave STD field empty (display layer adds "1st", "2nd", "3rd" labels)
    String scheduledTime = "";

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

    // For TFL: Just show destination (no line name or arrow)
    String destination = String(towards);

    // Populate service data
    scheduledTime.toCharArray(services[serviceCount].std, sizeof(services[serviceCount].std));
    etd.toCharArray(services[serviceCount].etd, sizeof(services[serviceCount].etd));
    destination.toCharArray(services[serviceCount].destination, sizeof(services[serviceCount].destination));
    services[serviceCount].callingPoints[0] = '\0';

    Serial.println("🚇 " + String(serviceCount + 1) + ": " + destination + " (" + etd + ")");
    serviceCount++;
  }

  if (serviceCount == 0) {
    if (lineFilter.length() > 0) {
      Serial.println("⚠️  No arrivals for line '" + lineFilter + "' - this line may not serve " + String(stationName));
    } else {
      Serial.println("⚠️  No arrivals found for " + String(stationName));
    }
  }

  return true;  // Return true for valid response, even if 0 services
}
