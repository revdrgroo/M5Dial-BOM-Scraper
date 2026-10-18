#ifndef MAIN_H
#define MAIN_H

// #include "lvgl.h"
#include "BomParse.h"
#include "LocalBomParse.h"
#include "BomUI.h"

#define SPIFFS_FILE_PATH "/test.xml"

bool connect_wifi();
#ifndef M5Stack
lv_obj_t* lv_show_loading();
void lv_show_bom_results(BomParseResult& result);
void lv_show_bom_results_table(BomParseResult& result, BomUI& ui);
void refresh_event_cb(lv_event_t* e);
void table_event_cb(lv_event_t* e);
void forecast_event_cb(lv_event_t* e);
#endif
#endif