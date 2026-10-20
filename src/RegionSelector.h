#pragma once
#ifndef _REGION_SELECTOR_H_
#define _REGION_SELECTOR_H_
#include <M5Dial.h>
#include <Vector>

class RegionSelector {
  static const int DisplayRegionLine = 80;
  static const int DisplayWeatherForLine = 40;
  std::vector<String> Regions;
  int previousWheelPosition;
  int selectedRegionIndex;
  int displayedRegionIndex;

 public:
  
  void Init() {
    Regions.push_back("Bowral");
    Regions.push_back("Sydney");
    Regions.push_back("Wollongong");
    Regions.push_back("Nowra");
    Start();
  }

  void Start() {
    selectedRegionIndex = -1;
    int dummy = M5Dial.Encoder.readAndReset();
    M5Dial.update();
    previousWheelPosition = M5Dial.Encoder.read();
    UpdateDisplay();
  }

  void UpdateDisplay() {
    displayedRegionIndex = previousWheelPosition % Regions.size();
    String regionName = Regions[displayedRegionIndex];

    M5Dial.Display.clear();
    M5Dial.Display.setFont(&fonts::Orbitron_Light_24);
    M5Dial.Display.setTextColor(GREEN);
    M5Dial.Display.setTextSize(0.6);
    M5Dial.Display.drawString("Weather for", M5Dial.Display.width() / 2, DisplayWeatherForLine);
    M5Dial.Display.setTextSize(1);
    M5Dial.Display.drawString(regionName, M5Dial.Display.width() / 2, DisplayRegionLine);
    M5Dial.Display.setTextSize(0.6);
    M5Dial.Display.setTextColor(SKYBLUE);
    M5Dial.Display.drawString("Rotate", M5Dial.Display.width() / 2, DisplayRegionLine + 25);
  }

  void Update() {
    M5Dial.update();
    if (SelectionMade()) {
      return;
    }
    int wheelPosition = M5Dial.Encoder.read();
    if (wheelPosition != previousWheelPosition) {
      M5Dial.Speaker.tone(2000, 100);
      previousWheelPosition = wheelPosition;
      UpdateDisplay();
    }
    if (M5Dial.BtnA.wasPressed()) {
      M5Dial.Speaker.tone(2000, 100);
      M5Dial.Encoder.readAndReset();  // Reset encoder count back to 0
      selectedRegionIndex = displayedRegionIndex;
    }
  }

  bool SelectionMade() {
    return selectedRegionIndex >= 0;
  }

  const String& SelectedRegion() {
    if (SelectionMade()) {
      return Regions[selectedRegionIndex];
    } else {
      return Regions[0];
    }
  }
};
#endif
