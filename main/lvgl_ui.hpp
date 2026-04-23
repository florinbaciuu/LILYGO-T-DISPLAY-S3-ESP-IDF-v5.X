#pragma once

// Includes
#include "lvgl.h"

// -------------------------------

// ############  GLOBALE UI LVGL ########## //
lv_obj_t* tabview = NULL;

// -------------------------------

/**********************
 *   LVGL Prototypes
 **********************/
void lvgl_ui_function(void);

// -------------------------------
/*********************
 *  rtos variables
 *********************/
TaskHandle_t xHandle_lv_ui_task;

/********************************************** */
/*                   TASK                       */
/********************************************** */
void lv_ui_task(void* arg) {
    (void) arg;
    xHandle_lv_ui_task = xTaskGetCurrentTaskHandle();  // Încoronarea oficială
    s_lvgl_lock(portMAX_DELAY);
    lvgl_ui_function();  // lvgl ui
    vTaskDelay(100);  // Așteaptă puțin pentru a permite stabilizarea UI-ului
    s_lvgl_unlock();
    vTaskDelete(NULL);  // moare după creare
}

// -------------------------------

void lvgl_ui_function(void) {
    // Creăm containerul de taburi
    tabview = lv_tabview_create(lv_screen_active());
    lv_tabview_set_tab_bar_size(tabview, 40);            // Setăm înălțimea tab-urilor
    lv_obj_set_size(tabview, LV_PCT(100), LV_PCT(100));  // Setăm dimensiunea tabview-ului
    lv_obj_set_flex_flow(tabview, LV_FLEX_FLOW_COLUMN);  // Setăm flex flow pentru tabview
    lv_obj_set_flex_grow(tabview, 1);                    // Permitem tabview-ului să ocupe tot spațiul disponibil
    lv_dir_t dir = LV_DIR_TOP;                           // Poziționăm tab-urile în partea de sus
    lv_tabview_set_tab_bar_position(
        tabview, dir);  // Funcția nu există în LVGL, deci comentăm această linie

    /*Adăugăm 3 taburi*/
    lv_obj_t* tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t* tab2 = lv_tabview_add_tab(tabview, "Tab 2");
    lv_obj_t* tab3 = lv_tabview_add_tab(tabview, "Tab 3");

    // TAB 1

    // TAB 2

    // TAB 3
}

// -------------------------------
// -------------------------------