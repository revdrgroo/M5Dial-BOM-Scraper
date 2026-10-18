#ifndef BOM_UI_H
#define BOM_UI_H

#include <M5Dial.h>

#include "BomParse.h"

struct BomUI {
#ifndef M5STACK
  // Widgets
  lv_obj_t* loadingLabel;
  lv_obj_t* table;
  lv_obj_t* regionLabel;
  lv_obj_t* refresh;
  lv_obj_t* refreshLabel;
  lv_obj_t* fullForecastLabel;
  lv_obj_t* fullForecastDismissButton;
  lv_obj_t* fullForecastDismissLabel;

  BomParseResult bomResults;

  // State
  bool cellSelected = false;
  uint32_t selectedRow = 0;
  uint32_t selectedCol = 0;

  void createUI() {
    create_loading_screen();
    create_results_screen();
    create_forecast_screen();
    hide_loading_screen();
    hide_results_screen();
    hide_forecast_screen();
  }

  void create_loading_screen() {
    lv_obj_t* label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Loading...");
    lv_obj_center(label);
    this->loadingLabel = label;
  }

  void create_results_screen() {
    this->table = lv_table_create(lv_screen_active());
    this->regionLabel = lv_label_create(lv_screen_active());
    this->refresh = lv_button_create(lv_screen_active());

    lv_obj_align(this->regionLabel, LV_ALIGN_TOP_LEFT, 3, 10);
    lv_obj_align(this->refresh, LV_ALIGN_TOP_RIGHT, -3, 3);
    lv_obj_set_size(this->refresh, 80, 27);

    this->refreshLabel = lv_label_create(this->refresh); /*Add a label to the button*/
    lv_label_set_text(this->refreshLabel, "Refresh");    /*Set the labels text*/
    lv_obj_center(this->refreshLabel);

    // Configure the table column sizes
    lv_table_set_column_count(this->table, 6);

    lv_table_set_column_width(this->table, 0, 20);
    lv_table_set_column_width(this->table, 1, 160);
    lv_table_set_column_width(this->table, 2, 30);
    lv_table_set_column_width(this->table, 3, 30);
    lv_table_set_column_width(this->table, 4, 40);
    lv_table_set_column_width(this->table, 5, 90);

    /*Set a smaller height to the table. It'll make it scrollable*/
    lv_obj_set_size(this->table, 310, 200);
    lv_obj_align(this->table, LV_ALIGN_CENTER, 0, 15);
    lv_obj_set_style_pad_ver(this->table, 1, LV_PART_ITEMS);
    lv_obj_set_style_pad_hor(this->table, 3, LV_PART_ITEMS);
    /*Don't make the cell pressed, we will draw something different in the event*/
    //lv_obj_remove_style(this->table, NULL, LV_PART_ITEMS | LV_STATE_PRESSED);
  }

  void hide_loading_screen() { lv_obj_add_flag(this->loadingLabel, LV_OBJ_FLAG_HIDDEN); }

  void show_loading_screen() { lv_obj_clear_flag(this->loadingLabel, LV_OBJ_FLAG_HIDDEN); }

  void hide_results_screen() {
    lv_obj_add_flag(this->table, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(this->regionLabel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(this->refresh, LV_OBJ_FLAG_HIDDEN);
  }

  void show_results_screen() {
    lv_obj_clear_flag(this->table, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(this->regionLabel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(this->refresh, LV_OBJ_FLAG_HIDDEN);
  }

  void create_forecast_screen() {
    this->fullForecastLabel = lv_label_create(lv_screen_active());
    lv_obj_align(this->fullForecastLabel, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_size(this->fullForecastLabel, 300, 160);
    lv_label_set_long_mode(this->fullForecastLabel, LV_LABEL_LONG_WRAP);
    lv_label_set_text(this->fullForecastLabel, "Full forecast");

    this->fullForecastDismissButton = lv_button_create(lv_screen_active());
    lv_obj_align(this->fullForecastDismissButton, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_size(this->fullForecastDismissButton, 80, 30);
    this->fullForecastDismissLabel = lv_label_create(this->fullForecastDismissButton);
    lv_label_set_text(this->fullForecastDismissLabel, "Close");
    lv_obj_align(this->fullForecastDismissLabel, LV_ALIGN_CENTER, 0, 0);
  }

  void hide_forecast_screen() {
    lv_obj_add_flag(this->fullForecastLabel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(this->fullForecastDismissButton, LV_OBJ_FLAG_HIDDEN);
  }

  void show_forecast_screen() {
    lv_obj_clear_flag(this->fullForecastLabel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(this->fullForecastDismissButton, LV_OBJ_FLAG_HIDDEN);
  }
#endif
};
#endif