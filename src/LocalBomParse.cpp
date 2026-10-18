#include "LocalBomParse.h"
#include <HTTPClient.h>

#define HTTP_SERVER_ADDRESS "10.1.1.150"
#define FTP_SERVER_PORT 8000
#define LITTLEFS_PATH "/local.xml"
#define XML_PARSE_LIMIT 50000

#define ROOT_TAG "/week_forecast"
#define TIMECODE_TAG ROOT_TAG"/timecode"
#define FORECAST_PERIOD_TAG ROOT_TAG"/forecast_period"
#define FORECAST_ICON_TAG FORECAST_PERIOD_TAG"/forecast_icon_code"
#define PRECIPITATION_RANGE_TAG FORECAST_PERIOD_TAG"/precipitation_range"
#define TEMPERATURE_MIN_TAG FORECAST_PERIOD_TAG"/air_temperature_minimum"
#define TEMPERATURE_MAX_TAG FORECAST_PERIOD_TAG"/air_temperature_maximum"
#define LONGFORECAST_TAG FORECAST_PERIOD_TAG"/forecast"
#define PRECIS_TAG FORECAST_PERIOD_TAG"/precis"
#define PRECIP_PROBABILITY_TAG FORECAST_PERIOD_TAG"/probability_of_precipitation"
#define PRECIPITATION_RANGE_ATTR FORECAST_PERIOD_TAG"precipitation_range"
#define UV_ALERT_TAG FORECAST_PERIOD_TAG"/uv_alert"
#define FIRE_DANGER_TAG FORECAST_PERIOD_TAG"/fire_danger"

#define COMPARE(field, value) (((field##Len == (sizeof(value) - 1)) && (strncasecmp(field, value, field##Len) == 0)))

void LocalBomParse::fetchXML() {
  Serial.printf("Local IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("WIFI Gateway: %s\n", WiFi.gatewayIP().toString().c_str());

  String serverPath = String("http://") + HTTP_SERVER_ADDRESS + ":" + String(FTP_SERVER_PORT);
  HTTPClient http;

  http.begin(serverPath.c_str());

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);

    // TODO: Save to SPIFFS
    File file = SPIFFS.open(LITTLEFS_PATH, "w");
    if (file) {
      file.print(payload);
      file.close();
      Serial.println("File written to SPIFFS");
    } else {
      Serial.println("Failed to write file to SPIFFS");
    }
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}

static const char* statusPrintable(uint8_t status) {
  switch (status) {
    case STATUS_START_TAG:
      return "START_TAG";
    case STATUS_END_TAG:
      return "END_TAG";
    case STATUS_TAG_TEXT:
      return "TAG_TEXT";
    case STATUS_ATTR_TEXT:
      return "ATTR_TEXT";
    case STATUS_ERROR:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}

static void LocalBomParse_XML_callback(uint8_t status, char* tagName, uint16_t tagNameLen, char* text, uint16_t textLen,
                                       void* userData) {
  //Serial.printf("HTTP XML callback: status=%s, tagName=%.*s, text=%.*s\n", statusPrintable(status), tagNameLen, tagName, textLen, text);                                
  LocalBomParse* bp = (LocalBomParse*)userData;

  if (status == STATUS_START_TAG) {
    if (COMPARE(tagName, FORECAST_PERIOD_TAG)) {
      bp->periodIndex = min(bp->periodIndex + 1, (int)(sizeof(bp->result.periods) / sizeof(bp->result.periods[0]) - 1));
      bp->result.periodCount = bp->periodIndex + 1;
    }
  }

  if ((status == STATUS_TAG_TEXT)) {
    if (COMPARE(tagName, TIMECODE_TAG)) {
      Serial.printf("Timecode: %.*s\n", textLen, text);
    }
    if (COMPARE(tagName, LONGFORECAST_TAG)) {
      Serial.printf("Forecast: %.*s\n", textLen, text);
      strncpy(bp->result.periods[bp->periodIndex].forecast, text,
              sizeof(bp->result.periods[bp->periodIndex].forecast) - 1);
    }
    if (COMPARE(tagName, PRECIS_TAG)) {
      Serial.printf("Precis: %.*s\n", textLen, text);
      strncpy(bp->result.periods[bp->periodIndex].precis, text, sizeof(bp->result.periods[bp->periodIndex].precis) - 1);
    }
    if (COMPARE(tagName, PRECIP_PROBABILITY_TAG)) {
      Serial.printf("Probability of precipitation: %.*s\n", textLen, text);
      strncpy(bp->result.periods[bp->periodIndex].precipProbability, text,
              sizeof(bp->result.periods[bp->periodIndex].precipProbability) - 1);
    }
    if (COMPARE(tagName, PRECIPITATION_RANGE_TAG)) {
      Serial.printf("Precipitation range: %.*s\n", textLen, text);
      strncpy(bp->result.periods[bp->periodIndex].precipRange, text,
              sizeof(bp->result.periods[bp->periodIndex].precipRange) - 1);
    }
    if (COMPARE(tagName, TEMPERATURE_MIN_TAG)) {
      Serial.printf("Min temperature: %.*s\n", textLen, text);
      bp->result.periods[bp->periodIndex].tempMin = atoi(text);
    }
    if (COMPARE(tagName, TEMPERATURE_MAX_TAG)) {
      Serial.printf("Max temperature: %.*s\n", textLen, text);
      bp->result.periods[bp->periodIndex].tempMax = atoi(text);
    }
  }
}

void LocalBomParse::parseXML() {
  uint8_t dataBuffer[384];
  size_t maxDataLen = sizeof(dataBuffer);
  TinyXML xml_parser;

  this->result.complete = false;
  xml_parser.init(dataBuffer, maxDataLen, LocalBomParse_XML_callback, this);
  Serial.println("HTTP test result:");
  fs::File spiffs_file = SPIFFS.open(LITTLEFS_PATH, "r");
  if (spiffs_file) {
    Serial.println("File content:");
    int count = 0;
    int limit = XML_PARSE_LIMIT;

    while (!this->result.complete && count < limit && spiffs_file.available()) {
      xml_parser.processChar(spiffs_file.read());
      count++;
    }
    spiffs_file.close();
    Serial.println("End of file reached.");
  } else {
    Serial.println("Failed to open file in SPIFFS");
  }
}