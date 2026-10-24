/*
 * AuraBar city / location settings
 * Auto locate (IP) + manual city search (Open-Meteo geocoding)
 */

#include <stdio.h>
#include <string.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"
#include "http_manager.h"

typedef struct {
    char name[32];
    char country[32];
    double lat;
    double lon;
} city_result_t;

static city_result_t s_results[5];
static int s_result_count = 0;
static bool s_search_done = false;

static lv_style_t com_style;
static lv_obj_t *s_result_cont = NULL;
static lv_timer_t *s_result_timer = NULL;

static void com_style_init(){
    lv_style_init(&com_style);
    if(lv_style_is_empty(&com_style) == false)
        lv_style_reset(&com_style);
    lv_style_set_bg_color(&com_style, APP_COLOR_BLACK);
    lv_style_set_radius(&com_style, 0);
    lv_style_set_border_width(&com_style, 0);
    lv_style_set_pad_all(&com_style, 0);
}

static void obj_font_set(lv_obj_t *obj, int type, uint16_t weight){
    lv_font_t* font = get_font(type, weight);
    if(font != NULL)
        lv_obj_set_style_text_font(obj, font, 0);
}

static void back_btn_click_event_cb(lv_event_t * e){
    delete_current_page(&com_style);
    init_page_main();
}

/* net 线程回调：只写全局数组，不操作 LVGL */
static void city_search_callback(char *name, char *country, double lat, double lon, int index, int total){
    if (index < 0) {
        s_result_count = total < 0 ? -1 : 0;
        s_search_done = true;
        return;
    }
    if (index < 5) {
        strncpy(s_results[index].name, name, sizeof(s_results[index].name) - 1);
        strncpy(s_results[index].country, country, sizeof(s_results[index].country) - 1);
        s_results[index].lat = lat;
        s_results[index].lon = lon;
    }
    if (index == total - 1) {
        s_result_count = (total < 5) ? total : 5;
        s_search_done = true;
    }
}

static void apply_city(int idx){
    device_state_t* ds = get_device_state();
    strncpy(ds->weather_city, s_results[idx].name, sizeof(ds->weather_city) - 1);
    ds->latitude = s_results[idx].lat;
    ds->longitude = s_results[idx].lon;
    device_param_write();
    http_get_weather_by_coord(ds->latitude, ds->longitude);
    delete_current_page(&com_style);
    init_page_main();
}

static void result_click_cb(lv_event_t *e){
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    apply_city(idx);
}

static void result_timer_cb(lv_timer_t *t){
    (void)t;
    if (!s_search_done) return;
    s_search_done = false;

    if (s_result_cont == NULL) return;
    lv_obj_clean(s_result_cont);

    if (s_result_count <= 0) {
        lv_obj_t *empty = lv_label_create(s_result_cont);
        obj_font_set(empty, FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
        lv_label_set_text(empty, s_result_count < 0 ? "Search failed. Check network / device time." : "No results");
        lv_obj_set_style_text_color(empty, APP_COLOR_WHITE, 0);
        lv_obj_align(empty, LV_ALIGN_CENTER, 0, 0);
        return;
    }

    for (int i = 0; i < s_result_count; i++) {
        lv_obj_t *btn = lv_btn_create(s_result_cont);
        lv_obj_set_size(btn, 240, 44);
        lv_obj_set_style_bg_color(btn, APP_COLOR_BUTTON_UNSELECT, 0);
        lv_obj_set_style_radius(btn, 10, 0);
        lv_obj_clear_state(btn, LV_STATE_FOCUS_KEY);
        lv_obj_add_event_cb(btn, result_click_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);

        lv_obj_t *label = lv_label_create(btn);
        obj_font_set(label, FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
        char buf[64];
        snprintf(buf, sizeof(buf), "%s, %s", s_results[i].name, s_results[i].country);
        lv_label_set_text(label, buf);
        lv_obj_set_style_text_color(label, APP_COLOR_WHITE, 0);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 10, 0);

        if (i > 0) {
            lv_obj_align_to(btn, lv_obj_get_child(s_result_cont, i - 1), LV_ALIGN_OUT_BOTTOM_MID, 0, 6);
        } else {
            lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 0);
        }
    }
}

static void search_btn_click_cb(lv_event_t *e){
    lv_obj_t *ta = (lv_obj_t*)lv_event_get_user_data(e);
    const char *name = lv_textarea_get_text(ta);
    if (name == NULL || strlen(name) == 0) return;
    printf("search city: %s\n", name);
    s_search_done = false;
    s_result_count = 0;
    lv_obj_clean(s_result_cont);
    lv_obj_t *status = lv_label_create(s_result_cont);
    lv_label_set_text(status, "Searching...");
    http_search_city((char*)name);
}

static void auto_locate_click_cb(lv_event_t *e){
    (void)e;
    printf("auto locate by ip\n");
    http_get_location_by_ip();
    delete_current_page(&com_style);
    init_page_main();
}

static lv_obj_t * init_title_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_align(cont, LV_ALIGN_TOP_MID);

    lv_obj_t *back_img = lv_img_create(cont);
    lv_img_set_src(back_img, GET_IMAGE_PATH("icon_back.png"));
    lv_obj_set_align(back_img, LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_pad_left(back_img, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_top(back_img, 20, LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title, FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title, "City Settings");
    lv_obj_set_style_text_color(title, APP_COLOR_WHITE, 0);
    lv_obj_align_to(title, back_img, LV_ALIGN_OUT_RIGHT_MID, 20, 3);

    lv_obj_add_event_cb(cont, back_btn_click_event_cb, LV_EVENT_CLICKED, NULL);
    return cont;
}

static lv_obj_t * make_btn(lv_obj_t *parent, const char *text, lv_event_cb_t cb, void *user_data){
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 240, 50);
    lv_obj_set_style_bg_color(btn, APP_COLOR_BUTTON_DEFALUT, 0);
    lv_obj_set_style_radius(btn, 25, 0);
    lv_obj_clear_state(btn, LV_STATE_FOCUS_KEY);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user_data);

    lv_obj_t *label = lv_label_create(btn);
    obj_font_set(label, FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, APP_COLOR_WHITE, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    return btn;
}

static void city_page_deleted(lv_event_t *e)
{
    (void)e;
    if (s_result_timer) lv_timer_del(s_result_timer);
    s_result_timer = NULL;
    s_result_cont = NULL;
    s_search_done = false;
    http_cancel_city_search();
}

void init_page_city_setting()
{
    http_cancel_city_search();
    s_result_count = 0;
    s_search_done = false;
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_add_event_cb(cont, city_page_deleted, LV_EVENT_DELETE, NULL);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);

    lv_obj_t *title_view = init_title_view(cont);

    /* 自动定位按钮 */
    lv_obj_t *auto_btn = make_btn(cont, "Auto Locate", auto_locate_click_cb, NULL);
    lv_obj_align_to(auto_btn, title_view, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    /* 城市名输入框 */
    lv_obj_t *ta = lv_textarea_create(cont);
    lv_obj_set_size(ta, 240, 46);
    lv_obj_set_style_text_color(ta, APP_COLOR_WHITE, 0);
    lv_obj_set_style_bg_color(ta, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(ta, 10, 0);
    lv_textarea_set_placeholder_text(ta, "City name");
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_max_length(ta, 40);
    lv_obj_align_to(ta, auto_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);

    lv_obj_t *kb = lv_keyboard_create(cont);
    lv_keyboard_set_textarea(kb, ta);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);

    /* 搜索按钮 */
    lv_obj_t *search_btn = make_btn(cont, "Search", search_btn_click_cb, ta);
    lv_obj_align_to(search_btn, ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);

    /* 搜索结果容器 */
    s_result_cont = lv_obj_create(cont);
    lv_obj_set_size(s_result_cont, 250, 240);
    lv_obj_add_style(s_result_cont, &com_style, 0);
    lv_obj_set_style_bg_opa(s_result_cont, LV_OPA_0, 0);
    lv_obj_clear_flag(s_result_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align_to(s_result_cont, search_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);

    s_result_timer = lv_timer_create(result_timer_cb, 200, NULL);

    http_set_search_callback(city_search_callback);

    lv_obj_t *bg_img = lv_img_create(cont);
    lv_img_set_src(bg_img, GET_IMAGE_PATH("bg_weather.png"));
    lv_obj_align(bg_img, LV_ALIGN_RIGHT_MID, -2, 0);
    lv_obj_move_background(bg_img);
}
