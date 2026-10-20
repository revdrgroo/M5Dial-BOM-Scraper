#pragma once
#ifndef _AUTO_SCROLL_TEXT_H_
#define _AUTO_SCROLL_TEXT_H_
#include <M5Dial.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define TEXT_SPRITE_WIDTH 200
#define TEXT_SPRITE_HEIGHT 100

class AutoScrollText {
  LGFX_Sprite* scrollSprite;
  unsigned long scrollStartMillis;
  int oldVerticalOffset;
  int scrollVerticalOffset;
  String text;
  int textYPos;
  int preScrollDelayMs = 2000;
  int postScrollDelayMs = 2000;
  int maxScroll = 120;
  int currentMaxScroll = maxScroll;
  int scrollStep = 2;
  int scrollStepTimeMs = 150;
  bool scrolling = false;

 public:
  void Init();
  void UpdateText(String newText);
  void StartScroll();
  void StopScroll();
  void Update();
  void CreateSprite(int verticalOffset);
  void DisplaySprite();
};
#endif