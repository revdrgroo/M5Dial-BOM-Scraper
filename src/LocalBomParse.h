#pragma once
#ifndef _LOCALBOMPARSE_H_
#define _LOCALBOMPARSE_H_

#include <BomParse.h>

class LocalBomParse {   
 private:
  void xmlCallback(uint8_t status, char* tagName, uint16_t tagNameLen, char* text, uint16_t textLen);

 public:
  int periodIndex = -1;
  BomParseResult& result;

  LocalBomParse(BomParseResult& result) : result(result) {};
  void fetchXML(String& region, bool refresh = false);
  void parseXML();
};

#endif