
// Includes
#include "lvgl_config.h"
#include "lvgl_port.h"
#include "display_port.h"
#include "product_pins.h"
#include "esp_log.h"
#include "lvgl.h"
#include <lv_conf.h>

// ========================================== //

/**********************
 *   LVGL FUNCTIONS
 **********************/

/* Display flushing function callback */
void lv_disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) { /* #if LVGL_BENCH_TEST */
    esp_lcd_panel_draw_bitmap(
        panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, (const void*) px_map);
#ifdef flush_ready_in_disp_flush
    lv_disp_flush_ready(disp);
#endif /* #ifdef (flush_ready_in_disp_flush) */
}

// -------------------------------

#if LV_TICK_SOURCE == LV_TICK_SOURCE_CALLBACK
uint32_t lv_get_rtos_tick_count_callback(void) {
    return xTaskGetTickCount();
}  // Callback pentru a obține numărul de tick-uri RTOS
#endif /* #if LV_TICK_SOURCE == LV_TICK_SOURCE_CALLBACK */

// -------------------------------

uint32_t    bufSize;           // Dimensiunea buffer-ului
lv_color_t* disp_draw_buf;     // Buffer LVGL
lv_color_t* disp_draw_buf_II;  // Buffer LVGL secundar

void s_lvgl_set_buffers_config() {
#if (BUFFER_MODE == BUFFER_FULL)
    bufSize = ((LCD_WIDTH * LCD_HEIGHT) * lv_color_format_get_size(lv_display_get_color_format(disp)));
#elif (BUFFER_MODE == BUFFER_60LINES)
    bufSize = ((LCD_WIDTH * 60) * lv_color_format_get_size(lv_display_get_color_format(disp)));
#elif (BUFFER_MODE == BUFFER_40LINES)
    bufSize = ((LCD_WIDTH * 40) * lv_color_format_get_size(lv_display_get_color_format(disp)));
#elif (BUFFER_MODE == BUFFER_20LINES)
    bufSize = ((LCD_WIDTH * 20) * lv_color_format_get_size(lv_display_get_color_format(disp)));
#elif (BUFFER_MODE == BUFFER_DEVIDED4)
    bufSize = ((LCD_WIDTH * LCD_HEIGHT) * lv_color_format_get_size(lv_display_get_color_format(disp)) / 4);
#endif
#if (BUFFER_MEM == BUFFER_SPIRAM)
#    if (DOUBLE_BUFFER_MODE == 1)
    disp_draw_buf    = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM);
    disp_draw_buf_II = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM);
    ESP_LOGI("LVGL", "LVGL buffers created in SPIRAM");
#    else
    disp_draw_buf = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM);
#    endif
#elif (BUFFER_MEM == BUFFER_INTERNAL)
#    if (DMA_ON == 1)
#        if (DOUBLE_BUFFER_MODE == 1)
    disp_draw_buf    = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    disp_draw_buf_II = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    ESP_LOGI("LVGL", "LVGL buffers created in SPIRAM");
#        else
    disp_draw_buf = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
#        endif
#    else
#        if (DOUBLE_BUFFER_MODE == 1)
    disp_draw_buf    = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL);
    disp_draw_buf_II = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL);
    ESP_LOGI("LVGL", "LVGL buffers created in SPIRAM");
#        else
    disp_draw_buf = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL);
#        endif
#    endif
#endif /* #if (BUFFER_MEM == BUFFER_SPIRAM) */

    // --- memset pentru curățare frame-uri ---
    // Asta îți garantează că primul frame e complet „negru”

    if (disp_draw_buf) {
        memset(disp_draw_buf, 0, bufSize);
    }
    if (disp_draw_buf_II) {
        memset(disp_draw_buf_II, 0, bufSize);
    }

    if (!disp_draw_buf) {  // VERIFICA DACA PRIMUL BUFFER ESTE CREAT
        ESP_LOGE("LVGL", "LVGL disp_draw_buf allocate failed!");
    }
#if (DOUBLE_BUFFER_MODE == 1)
    if (!disp_draw_buf_II) {  // VERIFICA DACA AL DOILEA BUFFER ESTE CREAT
        ESP_LOGE("LVGL", "LVGL disp_draw_buf_II allocate failed!");
    }
#endif

#if (DOUBLE_BUFFER_MODE == 1)
    lv_display_set_buffers(
        disp, disp_draw_buf, disp_draw_buf_II, bufSize, (lv_display_render_mode_t) RENDER_MODE);
    ESP_LOGI("LVGL", "LVGL buffers set");
#else
    lv_display_set_buffers(disp, disp_draw_buf, NULL, bufSize, (lv_display_render_mode_t) RENDER_MODE);
#endif
    vTaskDelay(1);
}

// -------------------------------

void s_lvgl_display_panel_setup_config_properties() {
    // Configurare panou LVGL
    lv_display_set_resolution(disp, LCD_WIDTH, LCD_HEIGHT);           // Seteaza rezolutia software
    lv_display_set_physical_resolution(disp, LCD_WIDTH, LCD_HEIGHT);  // Actualizeaza rezolutia reala
    lv_display_set_rotation(disp, (lv_display_rotation_t) 0);         // Seteaza rotatia lvgl
    lv_display_set_render_mode(disp,
        (lv_display_render_mode_t) RENDER_MODE);  // Seteaza (lv_display_render_mode_t)
    lv_display_set_antialiasing(disp, true);      // Antialiasing DA sau NU
    ESP_LOGI("LVGL", "LVGL display settings done");
    vTaskDelay(1);
}

// -------------------------------

// -------------------------------

void s_lvgl_port_init(void) {
    lv_init();
    ESP_LOGI("LVGL", "LVGL initialized...");
#if LV_TICK_SOURCE == LV_TICK_SOURCE_CALLBACK
    // Next function comment because create problems with lvgl timers and esp32 timers
    lv_tick_set_cb(lv_get_rtos_tick_count_callback);
#endif /* #if LV_TICK_SOURCE == LV_TICK_SOURCE_CALLBACK */
    disp = lv_display_create(
        (int32_t) LCD_WIDTH,
        (int32_t) LCD_HEIGHT);

    s_lvgl_port_init_locking_mutex();
    s_lvgl_set_buffers_config();                     // configure buffers based on CONFIG settings
    s_lvgl_display_panel_setup_config_properties();  // configure display properties based on CONFIG settings

    lv_display_set_flush_cb(disp, lv_disp_flush);  // Set the flush callback which will be called to
                                                   // copy the rendered image to the display.
    ESP_LOGI("LVGL", "LVGL display flush callback set");
}

// -------------------------------
// -------------------------------

/*
███████ ██████  ███████ ███████ ██████ ████████  ██████  ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██ ██      
█████   ██████  █████   █████   ██████    ██    ██    ██ ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██      ██ 
██      ██   ██ ███████ ███████ ██   ██   ██     ██████  ███████ 
*/

static SemaphoreHandle_t s_lvgl_mutex = NULL;

// ========================================== //

// -------------------------------

bool s_lvgl_port_init_locking_mutex(void) {
    if (!s_lvgl_mutex) {
        s_lvgl_mutex = xSemaphoreCreateRecursiveMutex();
        ESP_LOGI("LVGL", "LVGL mutex created...");
    }
    return (s_lvgl_mutex != NULL);
}

// -------------------------------

// -------------------------------

bool s_lvgl_lock(TickType_t timeout_ms) {
    if (!s_lvgl_mutex) {
        return false;
    }
    return (xSemaphoreTakeRecursive(s_lvgl_mutex, timeout_ms) == pdTRUE);
}

// -------------------------------

void s_lvgl_unlock(void) {
    if (s_lvgl_mutex)
        xSemaphoreGiveRecursive(s_lvgl_mutex);
}

// -------------------------------

/*********************
 *  rtos variables
 *********************/
TaskHandle_t xHandle_lv_main_task;
TaskHandle_t xHandle_lv_main_tick_task;

/************************************************** */

#if LV_TICK_SOURCE == LV_TICK_SOURCE_TIMER

static void lv_tick_timer_callback_func(void* arg) {
    lv_tick_inc((TICK_INCREMENTATION));  // 5 ms ticklv_timer_handler();
}

static void lv_tick_start_timer(void) {
    const esp_timer_create_args_t timer_args = {
        .callback              = &lv_tick_timer_callback_func,
        .dispatch_method       = ESP_TIMER_TASK,
        .name                  = "lv_tick",
        .skip_unhandled_events = true};
    esp_timer_handle_t tick_timer;

    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, LV_TIMER_INCREMENT));  // 1000 us = 1 ms
    ESP_LOGI("Timer", "LVGL tick timer started with period %d ms", LV_TIMER_INCREMENT / 1000);
    vTaskDelay(1);
}

/********************************************** */
/*                   TASK                       */
/********************************************** */
void lv_main_task(void* parameter) {
    xHandle_lv_main_task   = xTaskGetCurrentTaskHandle();
    static TickType_t tick = 0;
    tick                   = xTaskGetTickCount();
    while (true) {
        if (s_lvgl_lock(portMAX_DELAY)) {
            lv_timer_handler();
            s_lvgl_unlock();  // <— ADD
        }
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(LV_DELAY));
        // vTaskDelay(LV_DELAY);
    }
}

#elif LV_TICK_SOURCE == LV_TICK_SOURCE_TASK

/********************************************** */
/*                   TASK                       */
/********************************************** */
void lv_main_tick_task(void* parameter) {
    static TickType_t tick = 0;
    tick                   = xTaskGetTickCount();
    while (true) {
        lv_tick_inc(TICK_INCREMENTATION);                 // Incrementeaza tick-urile LVGL
        xTaskDelayUntil(&tick, pdMS_TO_TICKS(LV_DELAY));  // Delay precis mult mai rapid asa
    }
}

/********************************************** */
/*                   TASK                       */
/********************************************** */
void lv_main_task(void* parameter) {
    xHandle_lv_main_task   = xTaskGetCurrentTaskHandle();
    static TickType_t tick = 0;
    tick                   = xTaskGetTickCount();  // Inițializare corectă
    while (true) {
        if (s_lvgl_lock(portMAX_DELAY)) {  // <— ADD
            lv_timer_handler();
            s_lvgl_unlock();  // <— ADD
        }
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(LV_DELAY));
    }
}

#elif LV_TICK_SOURCE == LV_TICK_SOURCE_CALLBACK
/********************************************** */
/*                   TASK                       */
/********************************************** */
void lv_main_task(void* parameter) {
    xHandle_lv_main_task   = xTaskGetCurrentTaskHandle();
    static TickType_t tick = 0;
    tick                   = xTaskGetTickCount();  // Inițializare corectă
    while (true) {
        if (s_lvgl_lock(portMAX_DELAY)) {  // <— ADD
            lv_timer_handler();
            s_lvgl_unlock();  // <— ADD
        }
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(LV_DELAY));
    }
}

#endif /* #if LV_TICK_SOURCE == LV_TICK_SOURCE_TIMER */

/************************************************** */

void create_and_start_lvgl_tasks(void) {
    ESP_LOGI("FreeRTOS", "LVGL RTOS Kernel starting...");
    static bool started = false;
    if (started)
        return;
    started = true;
    xTaskCreatePinnedToCore(lv_main_task,  // Functia task-ului
        (const char*) "LVGL Main Task",    // Numele task-ului
        (uint32_t) (8192),                 // Dimensiunea stack-ului
        (NULL),                            // Parametri (daca exista)
        (UBaseType_t) 6,                   // Prioritatea task-ului // 6
        &xHandle_lv_main_task,             // Handle-ul task-ului
        ((1))                              // Nucleul pe care ruleaza task-ul
    );
    ESP_LOGI("FreeRTOS", "LVGL Main task created...");
#if LV_TICK_SOURCE == LV_TICK_SOURCE_TIMER
    lv_tick_start_timer();
    ESP_LOGI("Timer", "LVGL incremental tick timer created...");
#elif LV_TICK_SOURCE == LV_TICK_SOURCE_TASK
    xTaskCreatePinnedToCore(lv_main_tick_task,  // Functia care ruleaza task-ul
        (const char*) "LVGL Tick Task",         // Numele task-ului
        (uint32_t) (3072),                      // Dimensiunea stack-ului
        (NULL),                                 // Parametri
        (UBaseType_t) 6,                        // Prioritatea task-ului // 6
        &xHandle_lv_main_tick_task,             // Handle-ul task-ului
        ((1))                                   // Nucleul pe care ruleaza (ESP32 e dual-core)
    );
    ESP_LOGI("FreeRTOS", "LVGL incremental tick task created...");
#endif /* #if LV_TICK_SOURCE == LV_TICK_SOURCE_TIMER */
}

/************************************************** */

/************************************************** */

/************************************************** */

/************************************************** */