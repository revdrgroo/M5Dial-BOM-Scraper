#ifndef HSPI_HOST
#define HSPI_HOST SPI2_HOST
#endif

#ifndef VSPI_HOST
#define VSPI_HOST SPI3_HOST
#endif
/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#include "main.h"
#include "Credentials.h"

#include <Arduino.h>
#include <SPIFFS.h>

#include "TinyXML.h"
#include "WiFi.h"

///
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
///

#define FORMAT_LITTLEFS_IF_FAILED true
#define TRY_FTP_FETCH false
#define TRY_LOCAL_HTTP_FETCH true

#define DISPLAY_WEATHER_FOR_LINE 40
#define DISPLAY_REGION_LINE 80
#define DISPLAY_PRECIS_LINE 120
#define DISPLAY_PRECIS_LINE2 140
#define DISPLAY_PERIOD_NUM_LINE 200

long oldPosition = -999;
// static BomUI ui;
// static BomParseResult httpBomResult;
static BomParseResult bomResults;
static BomParse bomParse(bomResults);

// #define OUTPUT_BUFFER_SIZE 256
// static char OutputBuffer[OUTPUT_BUFFER_SIZE + 1];

void display_file_system_information() {

  Serial.println("File system:");
  Serial.printf("LittleFS total bytes %d\n", LittleFS.totalBytes());
  Serial.printf("LittleFS used bytes %d\n", LittleFS.usedBytes());
  File file = LittleFS.open(LITTLEFS_FILE_PATH, "r");
  if (file) {
    Serial.printf("Found %s", LITTLEFS_FILE_PATH);
    file.close();
  }
}

bool connect_wifi() {
  int wifi_connect_count = 0;
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wifi hotspot...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("o");
    delay(500);
    wifi_connect_count++;
    if (wifi_connect_count > 20) {
      Serial.println("Failed to connect to Wifi hotspot...");
      return false;
    }
  }

  // Debug trace
  Serial.println("\nConnected");
  //WiFi.disconnect();
  return true;
}

void show_ftp_results_simple() {
  Serial.println("FTP test result:");
  File file = LittleFS.open(LITTLEFS_FILE_PATH, "r");
  if (file) {
    Serial.println("File content:");
    while (file.available()) {
      Serial.write(file.read());
    }
    file.close();
  } else {
    Serial.println("Failed to open file in LittleFS");
  }
}

void fetchXMLFromBomFTP() {
  BomParseResult result;
  BomParse bom(result);

  bom.fetchXML();
}

void fetchXMLFromBomLocal() {
  BomParseResult result;
  LocalBomParse bom(result);

  bom.fetchXML();
}

void parse_ftp_results(BomParseResult& result) {
  BomParse bom(result);
  bom.parseXML();

  Serial.printf("Region: %s\n", result.regionName);
  for (int i = 0; i < result.periodCount; i++) {
    Serial.printf("Period %d: Min=%d, Max=%d, Forecast=%s, Precis=%s, Precip Probability=%s, Precip Range=%s\n", i,
                  result.periods[i].tempMin, result.periods[i].tempMax, result.periods[i].forecast,
                  result.periods[i].precis, result.periods[i].precipProbability, result.periods[i].precipRange);
  }
}

void parse_http_results(BomParseResult& result) {
  LocalBomParse bom(result);
  bom.parseXML();
  Serial.println("HTTP test result:");
  for (int i = 0; i < result.periodCount; i++) {
    Serial.printf("Period %d: Min=%d, Max=%d, Forecast=%s, Precis=%s, Precip Probability=%s, Precip Range=%s\n", i,
                  result.periods[i].tempMin, result.periods[i].tempMax, result.periods[i].forecast,
                  result.periods[i].precis, result.periods[i].precipProbability, result.periods[i].precipRange);
  }
  Serial.println("---\n---\n");
}

void displayPrecis(std::string& precis) {
  int h_center = M5Dial.Display.width() / 2;
  int v_pos = 60;
  int lineLength = 200;
  int lineHeight = 20;
  std::stringstream ss(precis);
  std::vector<std::string> words;
  std::string word;
  std::string line;

  M5Dial.Display.clear();
  M5Dial.Display.setTextColor(GREEN);
  M5Dial.Display.setFont(&fonts::Orbitron_Light_24);
  M5Dial.Display.setTextSize(1);
  M5Dial.Display.drawString(bomResults.regionName, M5Dial.Display.width() / 2, 30);
  M5Dial.Display.setFont(&fonts::DejaVu12);
  M5Dial.Display.setTextSize(1);
  M5Dial.Display.setTextColor(LIGHTGREY);
  while (ss >> word) {
    words.push_back(word);
  }

  int textWidth = 0;
  line.clear();
  int lineCount = 0;
  int lineLimit = 6;
  int spaceWidth = M5Dial.Display.textWidth(" ");
  for (const auto& w : words) {
    int nextWidth = M5Dial.Display.textWidth(w.c_str()) + spaceWidth;
    if (textWidth + nextWidth > lineLength) {
      lineCount++;
      if (lineCount >= lineLimit) {
        line += "...";
      }
      M5Dial.Display.drawString(line.c_str(), h_center, v_pos);
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
      Serial.printf("displayPrecis w=%s line=%s textWidth = %d nextWidth = %d\n", w.c_str(), line.c_str(), textWidth,
                    nextWidth);
    }
  }
  if (lineCount < lineLimit) {
    M5Dial.Display.drawString(line.c_str(), h_center, v_pos);
  }
}

void setup() {
  bool mountWorked = LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED);

  // Initialize configuration
  auto cfg = M5.config();
  // Begin: (config, enableEncoder, enableRFID)
  M5Dial.begin(cfg, true, false);

  if (!mountWorked) {
    M5Dial.Display.clear();
    M5Dial.Display.drawString("FS Fail", 120, 120);
    return;
  }
  Serial.begin(115200);

  // Configure Round Display
  M5Dial.Display.setTextColor(GREEN);
  M5Dial.Display.setTextDatum(middle_center);
  // M5Dial.Display.setFont(&fonts::Orbitron_Light_24);
  M5Dial.Display.setFont(&fonts::Orbitron_Light_24);
  M5Dial.Display.setTextSize(2);
  M5Dial.Display.drawString("Press", M5Dial.Display.width() / 2, M5Dial.Display.height() / 2);
  while (!M5Dial.BtnA.wasPressed()) {
    M5Dial.update();
  }
  M5Dial.Speaker.tone(4000, 20);  // Quick audible click tick
  display_file_system_information();
  if (!connect_wifi()) {
    Serial.println("WiFi not connected, cannot start test");
    return;
  }
  Serial.printf("Local IP: %s\n", WiFi.localIP().toString().c_str());

//  fetchXMLFromBomFTP();
#if TRY_FTP_FETCH
  parse_ftp_results(bomResults);
#endif
#if TRY_LOCAL_HTTP_FETCH
  LocalBomParse localBomParse(bomResults);
  localBomParse.fetchXML();
  localBomParse.parseXML();
  strlcpy(bomResults.regionName, "Bowral", sizeof(bomResults.regionName));
#else
  bomParse.parseXML();
  show_ftp_results_simple();
#endif

  // Draw initial boot graphic
  M5Dial.Display.clear();
  M5Dial.Display.setTextSize(0.6);
  M5Dial.Display.drawString("Weather for", M5Dial.Display.width() / 2, DISPLAY_WEATHER_FOR_LINE);
  M5Dial.Display.setTextSize(1);
  M5Dial.Display.drawString(bomResults.regionName, M5Dial.Display.width() / 2, DISPLAY_REGION_LINE);
  M5Dial.Display.setTextSize(0.6);
  M5Dial.Display.setTextColor(SKYBLUE);
  M5Dial.Display.drawString("Rotate", M5Dial.Display.width() / 2, DISPLAY_REGION_LINE + 25);
  oldPosition = M5Dial.Encoder.read();
}

void loop() {
  // Update state readings for buttons and peripheral ticks
  M5Dial.update();

  // 1. Read and process Rotary Encoder
  long newPosition = M5Dial.Encoder.read();
  if (newPosition != oldPosition) {

    oldPosition = newPosition;
    int selector = (newPosition % bomResults.periodCount + bomResults.periodCount) % bomResults.periodCount;
    Serial.printf("new pos %d, selector %d", newPosition, selector);
    M5Dial.Speaker.tone(4000, 20);  // Quick audible click tick

    std::string precis(bomResults.periods[selector].forecast);
    displayPrecis(precis);

    // M5Dial.Display.setFont(&fonts::Orbitron_Light_24);
    M5Dial.Display.setFont(&fonts::DejaVu12);
    M5Dial.Display.setTextSize(1);
    M5Dial.Display.setTextColor(GREEN);
    int h_center = M5Dial.Display.width() / 2;
    String date = String(bomResults.periods[selector].dayOfWeek) + String(" ") + String(bomResults.periods[selector].dateString);
    Serial.printf("Date string %s", date.c_str());
    M5Dial.Display.drawString(date, h_center, DISPLAY_PERIOD_NUM_LINE);
  }

  // 2. Read Screen/Logo Wake Button Actions
  if (M5Dial.BtnA.wasPressed()) {
    M5Dial.Speaker.tone(2000, 100);
    M5Dial.Encoder.readAndReset();  // Reset encoder count back to 0
  }
}

#if false
void setup() {
  Serial.begin(115200);
  show_ftp_results_simple();
  lv_init();
  hal_setup();
  ui.createUI();
  lv_obj_add_event_cb(ui.refresh, refresh_event_cb, LV_EVENT_ALL, (void*)nullptr); /*Assign a callback to the button*/
  lv_obj_add_event_cb(ui.table, table_event_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui.table, table_event_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(ui.fullForecastDismissButton, forecast_event_cb, LV_EVENT_CLICKED, NULL);

  ui.show_loading_screen();
  lv_refr_now(lv_display_get_default());  // Process LVGL events to update the UI

  display_file_system_information();
  if (!connect_wifi()) {
    Serial.println("WiFi not connected, cannot start test");
    return;
  }
  Serial.printf("Local IP: %s\n", WiFi.localIP().toString().c_str());

  // parse_http_results(ui.bomResults);
  parse_ftp_results(ui.bomResults);
  ui.hide_loading_screen();
  ui.show_results_screen();
  lv_show_bom_results_table(ui.bomResults, ui);
}

void loop() {
  hal_loop(); /* Do not use while loop in this function */
}

void lv_show_bom_results(BomParseResult& result) {
  /*Create a list*/
  lv_obj_t* bomResultsList = lv_list_create(lv_screen_active());
  lv_obj_set_size(bomResultsList, 320, 220);
  lv_obj_center(bomResultsList);
  char label[48];

  lv_obj_t* btn;

  lv_list_add_text(bomResultsList, result.regionName);
  for (int i = 0; i < result.periodCount; i++) {
    snprintf(label, sizeof(label) - 1, "%d:%s: %d->%d :%s of %s", i, result.periods[i].precis,
             result.periods[i].tempMin, result.periods[i].tempMax, result.periods[i].precipProbability,
             result.periods[i].precipRange);
    lv_list_add_text(bomResultsList, label);
  }
}

void refresh_event_cb(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    Serial.println("Refresh button clicked, refreshing data...");
    lv_table_set_row_count(ui.table, 0);
    ui.hide_results_screen();
    ui.show_loading_screen();
    lv_refr_now(lv_display_get_default());  // Process LVGL events to update the UI
    Serial.println("Is loading screen visible?");

    // fetchXMLFromBomLocal();
    // parse_http_results(ui.bomResults);

    fetchXMLFromBomFTP();
    parse_ftp_results(ui.bomResults);
    ui.hide_loading_screen();
    ui.show_results_screen();
    lv_show_bom_results_table(ui.bomResults, ui);
    lv_refr_now(lv_display_get_default());  // Process LVGL events to update the UI
  }
}

void table_event_cb(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);

  if (code == LV_EVENT_PRESSED) {
    ui.cellSelected = true;
    lv_table_get_selected_cell(obj, &ui.selectedRow, &ui.selectedCol);
    Serial.printf("Table pressed %d,%d\n", ui.selectedRow, ui.selectedCol);
  }
  if (code == LV_EVENT_CLICKED) {
    Serial.printf("Table clicked %d,%d\n", ui.selectedRow, ui.selectedCol);
    const char* cell_value = lv_table_get_cell_value(obj, ui.selectedRow, ui.selectedCol);
    Serial.printf("Clicked cell: Row %d, Col %d, Value: %s\n", ui.selectedRow, ui.selectedCol, cell_value);
    Serial.printf("Forecast for selected cell: %s\n", ui.bomResults.periods[ui.selectedRow].forecast);
    lv_label_set_text_fmt(ui.fullForecastLabel, "Period %d:\n%s", ui.selectedRow + 1,
                          ui.bomResults.periods[ui.selectedRow].forecast);
    ui.hide_results_screen();
    ui.show_forecast_screen();
  }
}

void forecast_event_cb(lv_event_t* e) {
  ui.hide_forecast_screen();
  ui.show_results_screen();
}

void lv_show_bom_results_table(BomParseResult& result, BomUI& ui) {
  lv_label_set_text_fmt(ui.regionLabel, "Region: %s", result.regionName);

  lv_table_set_row_count(ui.table, result.periodCount);
  uint32_t i;
  for (i = 0; i < result.periodCount; i++) {
    lv_table_set_cell_value_fmt(ui.table, i, 0, "%d", i + 1);
    lv_table_set_cell_value_fmt(ui.table, i, 1, "%s", result.periods[i].precis);
    lv_table_set_cell_value_fmt(ui.table, i, 2, "%d", result.periods[i].tempMin);
    lv_table_set_cell_value_fmt(ui.table, i, 3, "%d", result.periods[i].tempMax);
    lv_table_set_cell_value_fmt(ui.table, i, 4, "%s", result.periods[i].precipProbability);
    lv_table_set_cell_value_fmt(ui.table, i, 5, "%s", result.periods[i].precipRange);
  }
}
#endif
