#ifndef SERVICE_PROVIDER_H
#define SERVICE_PROVIDER_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include "types.h"
#include "config.h"

// ============ Service Provider Interface ============
// Abstract base class for transport service providers
class ServiceProvider {
public:
  virtual ~ServiceProvider() {}

  // Get provider name for display/logging
  virtual const char* getProviderName() = 0;

  // Get API host
  virtual const char* getApiHost() = 0;

  // Get API port (usually 443 for HTTPS)
  virtual int getApiPort() = 0;

  // Build HTTP request for fetching services
  // Returns true if request was built successfully
  virtual bool buildRequest(const char* stationCode, String& request) = 0;

  // Parse API response and populate ServiceData array
  // Returns true if parsing was successful
  virtual bool parseResponse(const String& response,
                            ServiceData* services,
                            int& serviceCount,
                            char* stationName,
                            size_t stationNameSize,
                            bool useCallingAt) = 0;

  // Validate station code format
  virtual bool isValidStationCode(const char* code) = 0;

  // Get station code description (e.g., "3-letter CRS code" or "TFL Station ID")
  virtual const char* getStationCodeDescription() = 0;
};

// ============ National Rail Provider ============
class NationalRailProvider : public ServiceProvider {
private:
  const char* apiHost = "lite.realtime.nationalrail.co.uk";
  const char* apiPath = "/OpenLDBWS/ldb9.asmx";
  const char* apiToken = "73ee3834-af35-4f22-9b8b-480b70571c39";

  // Helper function to extract tag values from XML
  String extractTagValue(const String& xml, const String& tag, const String& ns);

  // Decode HTML entities in station names
  String decodeHTMLEntities(const String& text);

public:
  const char* getProviderName() override { return "National Rail"; }
  const char* getApiHost() override { return apiHost; }
  int getApiPort() override { return 443; }
  const char* getStationCodeDescription() override { return "3-letter CRS code (e.g., PAD, BHM)"; }

  bool buildRequest(const char* stationCode, String& request) override;
  bool parseResponse(const String& response,
                    ServiceData* services,
                    int& serviceCount,
                    char* stationName,
                    size_t stationNameSize,
                    bool useCallingAt) override;
  bool isValidStationCode(const char* code) override;
};

// ============ TFL Underground Provider ============
class TflUndergroundProvider : public ServiceProvider {
private:
  const char* apiHost = "api.tfl.gov.uk";
  String apiKey;  // Will be loaded from config
  String lineFilter;  // Line filter (e.g., "northern", "elizabeth", "" for all)
  String directionFilter;  // Direction filter ("inbound", "outbound", "" for all)
  String currentStationCode;  // Store current station code for fallback station name
  String currentStationName;  // Store fetched station name
  String lastFetchedStationCode;  // Track which station code we last fetched name for

  // Helper to format ISO timestamp into time string
  String formatTime(const String& isoTimestamp);

  // Fetch station name from TFL API
  bool fetchStationName(const char* stationCode);

  // Extract JSON value (simple parser to avoid ArduinoJson overhead)
  String extractJsonValue(const String& json, const String& key);

public:
  TflUndergroundProvider();
  void setApiKey(const String& key) { apiKey = key; }
  void setLineFilter(const String& filter) { lineFilter = filter; }
  void setDirectionFilter(const String& direction) { directionFilter = direction; }
  void ensureStationNameCached(const char* stationCode);  // Pre-fetch station name if not cached

  const char* getProviderName() override { return "TFL Underground"; }
  const char* getApiHost() override { return apiHost; }
  int getApiPort() override { return 443; }
  const char* getStationCodeDescription() override { return "TFL Station ID (e.g., HUBSOK or 940GZZLUPAC)"; }

  bool buildRequest(const char* stationCode, String& request) override;
  bool parseResponse(const String& response,
                    ServiceData* services,
                    int& serviceCount,
                    char* stationName,
                    size_t stationNameSize,
                    bool useCallingAt) override;
  bool isValidStationCode(const char* code) override;
};

#endif
