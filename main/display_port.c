
// Includes
#include "lvgl_config.h"
#include "product_pins.h"
#include "display_port.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"  // Sau driverul real folosit de tines

/**********************
 *   GLOBAL VARIABLES
 **********************/
esp_lcd_panel_io_handle_t     lcd_io_handle   = NULL;
esp_lcd_i80_bus_handle_t      i80_bus         = NULL;
esp_lcd_panel_handle_t        panel_handle    = NULL;

lv_display_t* disp;  // Display LVGL
/**********************
 *   DISPLAY FUNCTIONS
 **********************/

 void display_bus_config(){
    esp_lcd_i80_bus_config_t lcd_bus_config = {.dc_gpio_num = BOARD_TFT_DC,
        .wr_gpio_num                                        = BOARD_TFT_WR,
        .clk_src                                            = LCD_CLK_SRC_DEFAULT,
        .data_gpio_nums                                     = {
            BOARD_TFT_DATA0,
            BOARD_TFT_DATA1,
            BOARD_TFT_DATA2,
            BOARD_TFT_DATA3,
            BOARD_TFT_DATA4,
            BOARD_TFT_DATA5,
            BOARD_TFT_DATA6,
            BOARD_TFT_DATA7,
        },
        .bus_width          = 8,
        .max_transfer_bytes = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
        .psram_trans_align  = 64,
        .sram_trans_align   = 4};
    // esp_lcd_new_i80_bus(&lcd_bus_config, &i80_bus);
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&lcd_bus_config, &i80_bus));
 }

 //========================================


 void display_io_i80_config(){
    esp_lcd_panel_io_i80_config_t lcd_io_config = {
        .cs_gpio_num         = BOARD_TFT_CS,
        .pclk_hz             = 10000000,
        .trans_queue_depth   = 10,
        .on_color_trans_done = panel_io_trans_done_callback,
        //.user_ctx            = NULL,
        .user_ctx       = disp,
        .lcd_cmd_bits   = 8,
        .lcd_param_bits = 8,
        .dc_levels      = {
                 .dc_idle_level  = 0,
                 .dc_cmd_level   = 0,
                 .dc_dummy_level = 0,
                 .dc_data_level  = 1,
        },
        .flags = {
            .cs_active_high     = 0,
            .reverse_color_bits = 0,
            .swap_color_bytes   = 0,
            .pclk_active_neg    = 0,
            .pclk_idle_low      = 0,
        },
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &lcd_io_config, &lcd_io_handle));
 }

 //========================================


 void display_panel_config(){
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BOARD_TFT_RST,
        .rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_RGB,
        .data_endian    = LCD_RGB_DATA_ENDIAN_LITTLE,  // .swap_color_bytes   = 1,
        .bits_per_pixel = 16,
        .flags          = {
                     .reset_active_high = 0,
        },
        .vendor_config = NULL,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(lcd_io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 0, 35));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, true));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
 }

 //========================================



bool panel_io_trans_done_callback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t* edata, void* user_ctx) {
#ifdef flush_ready_in_io_trans_done
    lv_display_t* d = (lv_display_t*) user_ctx;
    if (d)
        lv_disp_flush_ready(d);
#endif /* #ifdef (flush_ready_in_io_trans_done) */
    return false;
}

//========================================

