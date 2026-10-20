#pragma once
#ifndef _WEATHERDISPLAY_H_
#define _WEATHERDISPLAY_H_
#include <M5Dial.h>
#include "AutoScrollText.h"
#include "BomParse.h"

class WeatherDisplay {
  static const int DisplayDateLine = 200;
  static const int DisplayRegionLine = 50;
  BomParseResult& bomResults;
  AutoScrollText forecastScroll;
  int selectedPeriodIndex;
  int oldPosition;
  bool cancelled;

 public:
  WeatherDisplay(BomParseResult& bomResults) : bomResults(bomResults) {}

  void Init() {
    selectedPeriodIndex = 0;
    forecastScroll.Init();
    cancelled = false;
  }

  void Start() {
    cancelled = false;
    M5Dial.update();
    long oldPosition = M5Dial.Encoder.read();
    StartWeatherDisplay(selectedPeriodIndex);
  }

  void StartWeatherDisplay(int selector) {
    M5Dial.Display.clear();
    M5Dial.Display.setTextColor(GREEN);
    M5Dial.Display.setFont(&fonts::FreeSans18pt7b);
    M5Dial.Display.setTextSize(0.8);
    M5Dial.Display.drawString(bomResults.regionName, M5Dial.Display.width() / 2, DisplayRegionLine);
    M5Dial.Display.setFont(&fonts::DejaVu12);
    M5Dial.Display.setTextSize(1);
    M5Dial.Display.setTextColor(GREEN);
    int h_center = M5Dial.Display.width() / 2;
    String date =
        String(bomResults.periods[selector].dayOfWeek) + String(" ") + String(bomResults.periods[selector].dateString);
    M5Dial.Display.drawString(date, h_center, DisplayDateLine);
    forecastScroll.UpdateText(String(bomResults.periods[selector].forecast));
    forecastScroll.StartScroll();
  }

  void Update() {
    M5Dial.update();
    if (M5Dial.BtnA.wasPressed()) {
      cancelled = true;
      return;
    }
    long newPosition = M5Dial.Encoder.read();
    int selector = (newPosition % bomResults.periodCount + bomResults.periodCount) % bomResults.periodCount;

    if (newPosition != oldPosition) {
      oldPosition = newPosition;
      StartWeatherDisplay(selector);
      M5Dial.Speaker.tone(2000, 100);
    }
    forecastScroll.Update();
  }

  bool WasCancelled() { return cancelled; }
};
#endif