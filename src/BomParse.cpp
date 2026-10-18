#include "BomParse.h"
#include <LittleFS.h>

#define FTP_SERVER_ADDRESS "ftp.bom.gov.au"
#define FTP_SERVER_PORT 21
#define FTP_USER_NAME "anonymous"
#define FTP_USER_PASSWORD ""
// #define FTP_FILE_PATH "anon/gen/fwo/IDN11060.xml"
#define FTP_FILE_PATH "anon/gen/fwo/IDN11116.xml"
#define LITTLEFS_PATH "/test.xml"

void BomParse::fetchXML() {
  Serial.printf("Local IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("WIFI Gateway: %s\n", WiFi.gatewayIP().toString().c_str());

  Serial.printf("Fetching BOM XML by FTP to LittleFS: %s", LITTLEFS_PATH);
  GFTPClient ftp_client(5);;
  bool res = ftp_client.open(FTP_SERVER_ADDRESS, FTP_SERVER_PORT, FTP_USER_NAME, FTP_USER_PASSWORD);
  Serial.printf("Connect result: %s\n", res ? "success" : "failed");

  if (res) {
    Serial.print("Read file to LittleFS: ");
    File saveFile = LittleFS.open(LITTLEFS_PATH, "w", /* create */ true);
    if (ftp_client.read_file(FTP_FILE_PATH, saveFile)) {
      Serial.println("success");
    }
    else {
      Serial.println("failure");
    }
    saveFile.close();
    ftp_client.close();
  }
}

#if 0
#define XML_PARSE_LIMIT 4096
#define SELECTED_REGION "Sydney"
#else
#define XML_PARSE_LIMIT (1 << 20)
#define SELECTED_REGION "Bowral"
#endif

#define AREA_TAG "/product/forecast/area"
#define AREA_FORECAST_PERIOD_TAG "/product/forecast/area/forecast-period"
#define AREA_FORECAST_TEXT_TAG "/product/forecast/area/forecast-period/text"
#define AREA_FORECAST_ELEMENT_TAG "/product/forecast/area/forecast-period/element"
#define DESCRIPTION_ATTR "description"
#define TYPE_ATTR "type"
#define PRECIPITATION_RANGE_ATTR "precipitation_range"
#define TEMPERATURE_MIN_ATTR "air_temperature_minimum"
#define TEMPERATURE_MAX_ATTR "air_temperature_maximum"
#define PRECIS_ATTR "precis"
#define FORECAST_ATTR "forecast"
#define PRECIP_PROBABILITY_ATTR "probability_of_precipitation"

#define COMPARE(field, value) (((field##Len == (sizeof(value) - 1)) && (strncasecmp(field, value, field##Len) == 0)))

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

void callback(uint8_t status, char* tagName, uint16_t tagNameLen, char* text, uint16_t textLen, void* userData) {
  BomParse* bp = (BomParse*)userData;
  if (!bp->inAreaTag) {
    if (status == STATUS_START_TAG) {
      if (COMPARE(tagName, AREA_TAG)) {
        // Serial.println("In area tag");
        bp->inAreaTag = true;
        bp->inTargetRegion = false;
        bp->periodIndex = -1;
      }
    }
  }
  if (status == STATUS_END_TAG) {
    if (COMPARE(tagName, AREA_TAG)) {
      //   Serial.println("Exiting area tag");
      if (bp->inTargetRegion) {
        bp->result.complete = true;
      }
      bp->inAreaTag = false;
      bp->inTargetRegion = false;
      return;
    }
  }

  if (!bp->inAreaTag) {
    return;
  }
  // From here on we are in an area tag.
  if ((status == STATUS_ATTR_TEXT)) {
    if (COMPARE(tagName, DESCRIPTION_ATTR)) {
      if (COMPARE(text, SELECTED_REGION)) {
        bp->inTargetRegion = true;
        Serial.printf("Found %s area!\n", SELECTED_REGION);
        bp->result.periodCount = -1;
        strncpy(bp->result.regionName, SELECTED_REGION, sizeof(bp->result.regionName) - 1);
      }
    }
  }
  if (!bp->inTargetRegion) {
    // Serial.printf("x");
    return;
  }

  // From here on we are in the target region.
  //Serial.printf("XML Callback - Status: %s, Tag: >%.*s< Text: >%.*s<\n", statusPrintable(status), tagNameLen, tagName, textLen, text);

  if (status == STATUS_START_TAG) {
    bp->inElementTag = false;
    bp->inTextTag = false;
    bp->inPrecipRange = false;
    bp->inTempMin = false;
    bp->inTempMax = false;
    bp->inPrecis = false;
    bp->inForecast = false;
    bp->inPrecipProbability = false;

    if (COMPARE(tagName, AREA_FORECAST_PERIOD_TAG)) {
      bp->periodIndex = min(bp->periodIndex + 1, (int)(sizeof(bp->result.periods) / sizeof(bp->result.periods[0]) - 1));
      bp->result.periodCount = bp->periodIndex + 1;
    }
    if (COMPARE(tagName, AREA_FORECAST_TEXT_TAG)) {
      //   Serial.println("In text tag");
      bp->inTextTag = true;
    }
    if (COMPARE(tagName, AREA_FORECAST_ELEMENT_TAG)) {
      //   Serial.println("In element tag");
      bp->inElementTag = true;
    }
  }

  if (status == STATUS_ATTR_TEXT && COMPARE(tagName, TYPE_ATTR)) {
    // Serial.println("In type attr");
    if (bp->inElementTag) {
      if (COMPARE(text, PRECIPITATION_RANGE_ATTR)) {
        // Serial.println("In precip range attr");
        bp->inPrecipRange = true;
      }
      if (COMPARE(text, TEMPERATURE_MIN_ATTR)) {
        // Serial.println("In temp min attr");
        bp->inTempMin = true;
      }
      if (COMPARE(text, TEMPERATURE_MAX_ATTR)) {
        // Serial.println("In temp max attr");
        bp->inTempMax = true;
      }
    }
    if (bp->inTextTag) {
      if (COMPARE(text, FORECAST_ATTR)) {
        // Serial.println("In forecast attr");
        bp->inForecast = true;
      }
      if (COMPARE(text, PRECIS_ATTR)) {
        // Serial.println("In precis attr");
        bp->inPrecis = true;
      }
      if (COMPARE(text, PRECIP_PROBABILITY_ATTR)) {
        // Serial.println("In precip probability attr");
        bp->inPrecipProbability = true;
      }
    }
  }

  if ((status == STATUS_TAG_TEXT) && COMPARE(tagName, AREA_FORECAST_ELEMENT_TAG)) {
    if (bp->inPrecipRange) {
      Serial.printf("Precipitation range: %.*s\n", textLen, text);
      strncpy(bp->result.periods[bp->periodIndex].precipRange, text, sizeof(bp->result.periods[bp->periodIndex].precipRange) - 1);
    }
    if (bp->inTempMin) {
      Serial.printf("Min temperature: %.*s\n", textLen, text);
      bp->result.periods[bp->periodIndex].tempMin = atoi(text);
    }
    if (bp->inTempMax) {
      Serial.printf("Max temperature: %.*s\n", textLen, text);
      bp->result.periods[bp->periodIndex].tempMax = atoi(text);
    }
  }
  if ((status == STATUS_TAG_TEXT) && COMPARE(tagName, AREA_FORECAST_TEXT_TAG)) {
    if (bp->inForecast) {
      Serial.printf("Forecast: %.*s\n", textLen, text);
      strncpy(bp->result.periods[bp->periodIndex].forecast, text, sizeof(bp->result.periods[bp->periodIndex].forecast) - 1);
    }
    if (bp->inPrecis) {
      Serial.printf("Precis: %.*s\n", textLen, text);
      strncpy(bp->result.periods[bp->periodIndex].precis, text, sizeof(bp->result.periods[bp->periodIndex].precis) - 1);}
    if (bp->inPrecipProbability) {
      Serial.printf("Probability of precipitation: %.*s\n", textLen, text);
      strncpy(bp->result.periods[bp->periodIndex].precipProbability, text, sizeof(bp->result.periods[bp->periodIndex].precipProbability) - 1);
    }
  }
}

void BomParse::parseXML() {
  uint8_t dataBuffer[384];
  size_t maxDataLen = sizeof(dataBuffer);
  TinyXML xml_parser;

  this->result.complete = false;
  xml_parser.init(dataBuffer, maxDataLen, callback, this);
  Serial.println("FTP test result:");
  File file = LittleFS.open(LITTLEFS_PATH, "r");
  if (file) {
    Serial.println("File content:");
    int count = 0;
    int limit = XML_PARSE_LIMIT;
    this->inAreaTag = false;
    this->inTargetRegion = false;

    while (!this->result.complete && count < limit && file.available()) {
      xml_parser.processChar(file.read());
      count++;
    }
    file.close();
    Serial.println("End of file reached.");
  } else {
    Serial.println("Failed to open file in LittleFS");
  }
}