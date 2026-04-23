#pragma once
#ifndef DISPLAY_PORT_H
#define DISPLAY_PORT_H

// Includes
#include "lvgl.h"
#include "esp_lcd_panel_ops.h"

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

extern lv_display_t* disp;
extern esp_lcd_panel_handle_t    panel_handle;


/**********************
 *   DISPLAY FUNCTIONS
 **********************/
void display_bus_config();
void display_io_i80_config();
void display_panel_config();
bool panel_io_trans_done_callback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t* edata, void* user_ctx);


#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */
#endif /* #ifndef DISPLAY_PORT_H */