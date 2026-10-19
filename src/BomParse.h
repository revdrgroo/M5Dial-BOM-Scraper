#ifndef BOMPARSE_H
#define BOMPARSE_H

#include <SPIFFS.h>
#include "WiFi.h"
#include "GFTPClient.h"
#include "TinyXML.h"


struct BomParsePeriod {
  int tempMin;
  int tempMax;
  char forecast[384];
  char precis[32];
  char precipProbability[8];
  char precipRange[16];
  char dayOfWeek[16];
  char dateString[16];

  BomParsePeriod() : tempMin(0), tempMax(0) {
    forecast[0] = '\0';
    precis[0] = '\0';
    precipProbability[0] = '\0';
    precipRange[0] = '\0';
    dayOfWeek[0] = '\0';
    dateString[0] = '\0';
  }
};

struct BomParseResult {
  char regionName[32];
  BomParsePeriod periods[7];
  int periodCount;
  bool complete;
};

class BomParse {
    private:
    

    void xmlCallback(uint8_t status, char* tagName, uint16_t tagNameLen, char* text, uint16_t textLen);

    public:
    
    bool inAreaTag = false;
    bool inElementTag = false;
    bool inTextTag = false;

    bool inTargetRegion = false;

    bool inTempMax = false;
    bool inTempMin = false;
    bool inPrecipRange = false;
    bool inForecast = false;
    bool inPrecis = false;
    bool inPrecipProbability = false;
    int periodIndex = -1;
    BomParseResult& result;

    BomParse(BomParseResult &result) : result(result) {};
    void fetchXML();
    void parseXML();
};

#endif