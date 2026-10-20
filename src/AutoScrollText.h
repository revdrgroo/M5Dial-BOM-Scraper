#include <M5Dial.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "DynamicUIElement.h"

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
  void Init() {
    scrollSprite = new LGFX_Sprite(&M5Dial.Display);
    scrollSprite->createSprite(TEXT_SPRITE_WIDTH, TEXT_SPRITE_HEIGHT);
    textYPos = 60;
  }

  void UpdateText(String newText) { text = newText; }

  void StartScroll() {
    scrollStartMillis = millis();
    oldVerticalOffset = -1;
    scrolling = true;
  }

  void StopScroll() {
    scrolling = false;
  }

  void Update() {
    if (!scrolling) {
      return;
    }
    int time = (int)(millis() - scrollStartMillis);
    int scrollTimeMs = (currentMaxScroll / scrollStep * scrollStepTimeMs);
    if (time < preScrollDelayMs) {
      scrollVerticalOffset = 0;
    } else {
      time -= preScrollDelayMs;
      int scrollCandidate = (time / scrollStepTimeMs) * scrollStep;
      scrollVerticalOffset = min(currentMaxScroll, max(0, scrollCandidate));
    }
    if (scrollVerticalOffset != oldVerticalOffset) {
      oldVerticalOffset = scrollVerticalOffset;
      CreateSprite(scrollVerticalOffset);
      DisplaySprite();
    }
    if (time > scrollTimeMs + postScrollDelayMs) {
      StartScroll();
    }
  }

  void CreateSprite(int verticalOffset) {
    std::stringstream ss(text.c_str());
    std::vector<std::string> words;
    std::string word;
    std::string line;

    scrollSprite->setFont(&fonts::DejaVu12);

    while (ss >> word) {
      words.push_back(word);
    }

    line.clear();
    scrollSprite->clear();

    int textWidth = 0;
    int v_pos = -verticalOffset;
    int lineLength = TEXT_SPRITE_WIDTH;
    int lineHeight = 20;
    int lineCount = 0;
    int lineLimit = 10;
    int spaceWidth = M5Dial.Display.textWidth(" ");
    for (const auto& w : words) {
      int nextWidth = M5Dial.Display.textWidth(w.c_str()) + spaceWidth;
      if (textWidth + nextWidth > lineLength) {
        lineCount++;
        scrollSprite->drawString(line.c_str(), 0, v_pos);
        line = w;
        textWidth = nextWidth;
        v_pos += lineHeight;
        if (lineCount >= lineLimit) {
          break;
        }
      } else {
        line += " ";
        line += w;
        textWidth += nextWidth;
      }
    }
    scrollSprite->drawString(line.c_str(), 0, v_pos);
    currentMaxScroll = v_pos + lineHeight;
  }

  void DisplaySprite() {
    int pushX = (M5Dial.Display.width() - TEXT_SPRITE_WIDTH) / 2;
    scrollSprite->pushSprite(pushX, textYPos);
  }
};