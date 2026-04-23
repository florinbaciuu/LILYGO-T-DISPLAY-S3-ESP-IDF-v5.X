
//---------

/*********************
 *   C  INCLUDES 
 *********************/
extern "C" {
#include "product_pins.h"
#include "lvgl_config.h"
#include "display_port.h"
#include "lvgl_port.h"
#include "lvgl_ui.hpp"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "filesystem.h"
#include "cli.h"
}

/*********************
 *   CPP INCLUDES 
 *********************/
//#include <fmt/base.h>

/**********************
 *   GLOBAL VARIABLES
 **********************/

//---------

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
//---------
void power_latch_init() {
    gpio_reset_pin((gpio_num_t) PWR_ON_PIN);
    gpio_config_t io_conf = {.pin_bit_mask = 1ULL << PWR_ON_PIN,
        .mode                              = GPIO_MODE_OUTPUT,
        .pull_up_en                        = GPIO_PULLUP_DISABLE,
        .pull_down_en                      = GPIO_PULLDOWN_DISABLE,
        .intr_type                         = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t) PWR_ON_PIN, 1);  // ⚡ ține placa aprinsă
}
//---------
void gfx_set_backlight(uint32_t mode) {
    gpio_reset_pin((gpio_num_t) BOARD_TFT_BL);
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BOARD_TFT_BL,
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t) BOARD_TFT_BL, mode);
}
//---------
//---------

/*
███████ ██████  ███████ ███████ ██████ ████████  ██████  ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██ ██      
█████   ██████  █████   █████   ██████    ██    ██    ██ ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██      ██ 
██      ██   ██ ███████ ███████ ██   ██   ██     ██████  ███████ 
*/

/*********************
 *  rtos variables
 *********************/
TaskHandle_t xHandle_chechButton0State;

// -------------------------------
// -------------------------------

/********************************************** */
/*                   TASK                       */
/********************************************** */
static void IRAM_ATTR chechButton0State_isr_handler(void* arg) {
    // NOTĂ: NU face log sau delay aici
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR((TaskHandle_t) arg, 0x01, eSetBits, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}
//---------
void chechButton0State(void* parameter) {
    (void) parameter;
    static uint8_t current_tab = 0;
    xHandle_chechButton0State = xTaskGetCurrentTaskHandle();  // Încoronarea oficială
    uint32_t      notificationValue;
    gpio_reset_pin((gpio_num_t) BOARD_BUTTON_BOOT);
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BOARD_BUTTON_BOOT,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,  // activăm pull-up intern
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_NEGEDGE,  // întrerupere pe orice schimbare de stare
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);  // doar o dată în tot proiectul
    gpio_isr_handler_add((gpio_num_t) BOARD_BUTTON_BOOT, chechButton0State_isr_handler, (void*) xHandle_chechButton0State);
    while (true) {
        xTaskNotifyWait(0x00, 0xFFFFFFFF, &notificationValue, portMAX_DELAY);
        if (notificationValue & 0x01) {
            ESP_LOGW("BUTTON", "Button ACTIVAT pe GPIO0");
           current_tab++;
            if (current_tab > 2)
                current_tab = 0;
            // schimbă tab-ul LVGL
            if (s_lvgl_lock(40)) {  // protejează LVGL cu mutex-ul tău
                lv_tabview_set_act(tabview, current_tab, LV_ANIM_ON);
                s_lvgl_unlock();
            }
        }
        vTaskDelay(200); // debounce și pentru a evita multiple notificări rapide
    }
}
/****************************/
void printStartupApplicationMessage() {
    printf("\n\n");
    printf("========================================\n");
    printf("   🚀 Starting Application 🚀\n");
    ESP_LOGI("BIOS", "   🚀 Starting Application 🚀\n");
    printf("========================================\n");
    
}
//--------------------------------------
/*
███    ███  █████  ██ ███    ██ 
████  ████ ██   ██ ██ ████   ██ s
██ ████ ██ ███████ ██ ██ ██  ██ 
██  ██  ██ ██   ██ ██ ██  ██ ██ 
██      ██ ██   ██ ██ ██   ████ 
  * This is the main entry point of the application.
  * It initializes the hardware, sets up the display, and starts the LVGL tasks.
  * The application will run indefinitely until the device is powered off or reset.
*/
extern "C" void app_main(void) {
    //vTaskDelay(1000);  // Așteaptă puțin pentru a permite stabilizarea sistemului după boot
    printStartupApplicationMessage();
    vTaskDelay(1000);  // Așteaptă puțin pentru a permite stabilizarea sistemului după boot

    esp_log_level_set("*", ESP_LOG_INFO);  // Setează nivelul de logare pentru toate modulele la INFO
    ESP_LOGI("SYSTEM LOG", "SYSTEM WIDE LOG LEVEL SET TO ESP_LOG_INFO");

    power_latch_init();                    // Inițializare latch pentru alimentare
    ESP_LOGI("Power", "Board Power initialized, device should stay ON");
    gfx_set_backlight(1);                  // Aprinde backlight-ul
    ESP_LOGI("Backlight", "Backlight ON");
    gpio_reset_pin((gpio_num_t) BOARD_TFT_RD);                        // Reset pin pentru display, necesar pentru unele modele de display-uri
    gpio_set_direction((gpio_num_t) BOARD_TFT_RD, GPIO_MODE_OUTPUT);  // Configurează pinul ca output
    gpio_set_level((gpio_num_t) BOARD_TFT_RD, 1);                     // Setează pinul la nivel înalt pentru a evita problemele de comunicare cu display-ul
    ESP_LOGI("Display Power", "Power ON on PIN %d", BOARD_TFT_RD);


    s_lvgl_port_init();  // Inițializează portul LVGL (driverul de display, input, etc.)

    display_bus_config();     // Configurează bus-ul de comunicare cu display-ul
    display_io_i80_config();  // Configurează interfața i80 pentru display
    display_panel_config();   // Configurează panoul de display (rezoluție, orientare, etc.)


    create_and_start_lvgl_tasks();  // freetos tasks for lvgl
    vTaskDelay(50);


    xTaskCreatePinnedToCore(lv_ui_task,      // Functia care ruleaza task-ul
        (const char*) "ui_task",             // Numele task-ului
        (uint32_t) (8192),                   // Dimensiunea stack-ului
        (NULL),                              // Parametri
        (UBaseType_t) tskIDLE_PRIORITY + 5,  // Prioritatea task-ului
        &xHandle_lv_ui_task,                 // Handle-ul task-ului
        ((1))                                // Nucleul pe care ruleaza (ESP32 e dual-core)
    );

    xTaskCreatePinnedToCore(chechButton0State,  // Functia care ruleaza task-ul
        (const char*) "v_check_0_pin_state",    // Numele task-ului
        (uint32_t) (4096),                      // Dimensiunea stack-ului
        (NULL),                                 // Parametri
        (UBaseType_t) tskIDLE_PRIORITY + 3,     // Prioritatea task-ului // 6
        &xHandle_chechButton0State,             // Handle-ul task-ului
        ((1))                                   // Nucleul pe care ruleaza (ESP32 e dual-core)
    );

    init_filesystem_sys();  // Inițializează sistemul de fișiere (LITTLEFS, SPIFFS, sau FFAT, în funcție de configurație)
    start_TDisplayS3_CLI();  // Pornește interfața de linie de comandă (CLI) pentru a permite interacțiunea cu utilizatorul prin terminal
    vTaskDelay(200);  // Așteaptă puțin pentru a permite stabilizarea sistemului înainte de a începe interacțiunea cu utilizatorul

    //fmt::print("Aici aplicatia ar trebui sa returneze.Meh\n");
    // Aplicația va continua să ruleze în fundal, gestionând interacțiunile cu display-ul și CLI-ul, până când dispozitivul este oprit sau resetat.
}
// END MAIN

// ############################################################## //
//---------