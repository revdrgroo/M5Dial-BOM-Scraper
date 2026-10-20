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

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "AutoScrollText.h"
#include "RegionSelector.h"
#include "WeatherDisplay.h"

#define ROTATE_DISPLAY_180 true
#define FORMAT_LITTLEFS_IF_FAILED true
#define TRY_FTP_FETCH false
#define TRY_LOCAL_HTTP_FETCH true

#define DISPLAY_WEATHER_FOR_LINE 40

#define FORECAST_SPRITE_WIDTH 200
#define FORECAST_SPRITE_HEIGHT 100

long oldPosition = -999;
// static BomUI ui;
// static BomParseResult httpBomResult;
static BomParseResult bomResults;
static BomParse bomParse(bomResults);
// static AutoScrollText forecastScroll;

// #define OUTPUT_BUFFER_SIZE 256
// static char OutputBuffer[OUTPUT_BUFFER_SIZE + 1];

LGFX_Sprite* scrollSprite;

enum class AppState { REGION_SELECTION, WEATHER_DISPLAY };
static AppState currentState = AppState::REGION_SELECTION;
static String selectedRegion = String("");
static RegionSelector regionSelector;
static WeatherDisplay weatherDisplay(bomResults);

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

  String region("bowral");
  bom.fetchXML(region);
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

void showStartScreen() {
  M5Dial.Display.setTextColor(GREEN);
  M5Dial.Display.setTextDatum(middle_center);
  int dw = M5Dial.Display.width();
  int dh = M5Dial.Display.height();
  M5Dial.Display.setFont(&fonts::Orbitron_Light_24);
  M5Dial.Display.setTextSize(2);
  M5Dial.Display.drawString("Press", dw / 2, dh / 2);
  int triSize = 30;
#if ROTATE_DISPLAY_180
  M5Dial.Display.fillTriangle(dw / 2 - triSize, 10 + triSize, dw / 2 + triSize, 10 + triSize, dw / 2, 10, GREEN);
#else
  M5Dial.Display.fillTriangle(dw / 2 - triSize, 220 - triSize, dw / 2 + triSize, 220 - triSize, dw / 2, 220, GREEN);
#endif
  while (!M5Dial.BtnA.wasPressed()) {
    M5Dial.update();
  }
  M5Dial.Speaker.tone(4000, 20);  // Quick audible click tick
}

void setup() {
  bool mountWorked = LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED);

  // Initialize configuration
  auto cfg = M5.config();
  // Begin: (config, enableEncoder, enableRFID)
  M5Dial.begin(cfg, true, false);
#if ROTATE_DISPLAY_180
  M5Dial.Display.setRotation(2);
#endif

  if (!mountWorked) {
    M5Dial.Display.clear();
    M5Dial.Display.drawString("FS Fail", 120, 120);
    return;
  }
  Serial.begin(115200);

  showStartScreen();
  display_file_system_information();
  if (!connect_wifi()) {
    Serial.println("WiFi not connected, cannot start test");
    return;
  }
  Serial.printf("Local IP: %s\n", WiFi.localIP().toString().c_str());

#if false
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
#endif

  regionSelector.Init();
  weatherDisplay.Init();
}

void loop() {
  switch (currentState) {
    case AppState::REGION_SELECTION:
      regionSelector.Update();
      if (regionSelector.SelectionMade()) {
        selectedRegion = String(regionSelector.SelectedRegion());
        currentState = AppState::WEATHER_DISPLAY;
        LocalBomParse localBomParse(bomResults);
        localBomParse.fetchXML(selectedRegion);
        localBomParse.parseXML();
        strlcpy(bomResults.regionName, selectedRegion.c_str(), sizeof(bomResults.regionName));
        weatherDisplay.Start();
      }
      break;
    case AppState::WEATHER_DISPLAY:
      weatherDisplay.Update();
      if (weatherDisplay.WasCancelled()) {
        currentState = AppState::REGION_SELECTION;
        regionSelector.Start();
      }
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
