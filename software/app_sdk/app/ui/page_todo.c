/*
 * AuraBar To-Do list page
 * Checkbox list with per-item delete button, persistent storage
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"
#include "file_save.h"

#define MAX_TODO 20
#define TODO_TEXT_LEN 64

typedef struct {
    char text[TODO_TEXT_LEN];
    bool done;
} todo_item_t;

typedef struct {
    int count;
    todo_item_t items[MAX_TODO];
} todo_data_t;

static todo_data_t s_todo;
static lv_style_t com_style;
static lv_obj_t *s_list_cont = NULL;
static lv_obj_t *s_kb = NULL;

static void rebuild_list(void);

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

static void todo_load(){
    memset(&s_todo, 0, sizeof(s_todo));
    file_param_read("todo.cfg", &s_todo, sizeof(s_todo));
    if (s_todo.count < 0 || s_todo.count > MAX_TODO)
        s_todo.count = 0;
    for (int i = 0; i < s_todo.count; ++i)
        s_todo.items[i].text[TODO_TEXT_LEN - 1] = 0;
}

static void todo_save(){
    file_param_write("todo.cfg", &s_todo, sizeof(s_todo));
}

static void back_btn_click_event_cb(lv_event_t * e){
    delete_current_page(&com_style);
    init_page_main();
}

static void checkbox_change_cb(lv_event_t *e){
    lv_obj_t *cb = lv_event_get_target(e);
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    if (idx >= 0 && idx < s_todo.count) {
        s_todo.items[idx].done = lv_obj_has_state(cb, LV_STATE_CHECKED);
        todo_save();
    }
}

static void del_btn_cb(lv_event_t *e){
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    if (idx < 0 || idx >= s_todo.count) return;
    for (int i = idx; i < s_todo.count - 1; i++)
        s_todo.items[i] = s_todo.items[i + 1];
    s_todo.count--;
    todo_save();
    rebuild_list();
}

static lv_obj_t * create_task_item(int idx){
    lv_obj_t *row = lv_obj_create(s_list_cont);
    lv_obj_set_size(row, 700, 46);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(row, 12, 0);

    lv_obj_t *cb = lv_checkbox_create(row);
    lv_obj_set_size(cb, 36, 36);
    lv_checkbox_set_text(cb, "");
    if (s_todo.items[idx].done)
        lv_obj_add_state(cb, LV_STATE_CHECKED);
    lv_obj_add_event_cb(cb, checkbox_change_cb, LV_EVENT_VALUE_CHANGED, (void*)(intptr_t)idx);

    lv_obj_t *label = lv_label_create(row);
    obj_font_set(label, FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
    lv_label_set_text(label, s_todo.items[idx].text);
    lv_obj_set_style_text_color(label, APP_COLOR_WHITE, 0);
    lv_obj_set_width(label, 560);

    lv_obj_t *del = lv_btn_create(row);
    lv_obj_set_size(del, 36, 36);
    lv_obj_set_style_bg_color(del, lv_color_hex(0xCC4444), 0);
    lv_obj_set_style_radius(del, 18, 0);
    lv_obj_clear_state(del, LV_STATE_FOCUS_KEY);
    lv_obj_add_event_cb(del, del_btn_cb, LV_EVENT_CLICKED, (void*)(intptr_t)idx);
    lv_obj_t *del_label = lv_label_create(del);
    obj_font_set(del_label, FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
    lv_label_set_text(del_label, "X");
    lv_obj_set_style_text_color(del_label, APP_COLOR_WHITE, 0);
    lv_obj_align(del_label, LV_ALIGN_CENTER, 0, 0);

    return row;
}

static void rebuild_list(){
    if (s_list_cont == NULL)
        return;
    lv_obj_clean(s_list_cont);
    for (int i = 0; i < s_todo.count; i++) {
        lv_obj_t *row = create_task_item(i);
        if (i > 0) {
            lv_obj_align_to(row, lv_obj_get_child(s_list_cont, i - 1), LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
        } else {
            lv_obj_align(row, LV_ALIGN_TOP_MID, 0, 0);
        }
    }
}

static void clear_done_cb(lv_event_t *e){
    (void)e;
    int w = 0;
    for (int i = 0; i < s_todo.count; i++) {
        if (!s_todo.items[i].done) {
            s_todo.items[w++] = s_todo.items[i];
        }
    }
    s_todo.count = w;
    todo_save();
    rebuild_list();
}

static void ta_event_cb(lv_event_t *e){
    if (lv_event_get_code(e) == LV_EVENT_FOCUSED && s_kb != NULL) {
        lv_keyboard_set_textarea(s_kb, lv_event_get_target(e));
    }
}

static void add_btn_cb(lv_event_t *e){
    lv_obj_t *ta = (lv_obj_t*)lv_event_get_user_data(e);
    const char *text = lv_textarea_get_text(ta);
    if (text == NULL || strlen(text) == 0)
        return;
    if (s_todo.count >= MAX_TODO)
        return;
    strncpy(s_todo.items[s_todo.count].text, text, TODO_TEXT_LEN - 1);
    s_todo.items[s_todo.count].text[TODO_TEXT_LEN - 1] = 0;
    s_todo.items[s_todo.count].done = false;
    s_todo.count++;
    todo_save();
    lv_textarea_set_text(ta, "");
    rebuild_list();
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
    lv_label_set_text(title, "To-Do");
    lv_obj_set_style_text_color(title, APP_COLOR_WHITE, 0);
    lv_obj_align_to(title, back_img, LV_ALIGN_OUT_RIGHT_MID, 20, 3);

    lv_obj_t *clear_btn = lv_btn_create(cont);
    lv_obj_set_size(clear_btn, 130, 40);
    lv_obj_set_style_bg_color(clear_btn, APP_COLOR_BUTTON_DEFALUT, 0);
    lv_obj_set_style_radius(clear_btn, 20, 0);
    lv_obj_clear_state(clear_btn, LV_STATE_FOCUS_KEY);
    lv_obj_add_event_cb(clear_btn, clear_done_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_align(clear_btn, LV_ALIGN_TOP_RIGHT, -20, 20);

    lv_obj_t *clear_label = lv_label_create(clear_btn);
    obj_font_set(clear_label, FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
    lv_label_set_text(clear_label, "Clear Done");
    lv_obj_set_style_text_color(clear_label, APP_COLOR_WHITE, 0);
    lv_obj_align(clear_label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(cont, back_btn_click_event_cb, LV_EVENT_CLICKED, NULL);
    return cont;
}

void init_page_todo()
{
    com_style_init();
    todo_load();

    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);

    lv_obj_t *title_view = init_title_view(cont);

    /* 输入区 */
    lv_obj_t *ta = lv_textarea_create(cont);
    lv_obj_set_size(ta, 520, 44);
    lv_obj_set_style_text_color(ta, APP_COLOR_WHITE, 0);
    lv_obj_set_style_bg_color(ta, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(ta, 10, 0);
    lv_textarea_set_placeholder_text(ta, "New task...");
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_max_length(ta, TODO_TEXT_LEN - 1);
    lv_obj_align_to(ta, title_view, LV_ALIGN_OUT_BOTTOM_LEFT, 40, 14);
    lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_FOCUSED, NULL);

    lv_obj_t *add_btn = lv_btn_create(cont);
    lv_obj_set_size(add_btn, 100, 44);
    lv_obj_set_style_bg_color(add_btn, APP_COLOR_BUTTON_DEFALUT, 0);
    lv_obj_set_style_radius(add_btn, 22, 0);
    lv_obj_clear_state(add_btn, LV_STATE_FOCUS_KEY);
    lv_obj_add_event_cb(add_btn, add_btn_cb, LV_EVENT_CLICKED, ta);
    lv_obj_align_to(add_btn, ta, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_obj_t *add_label = lv_label_create(add_btn);
    obj_font_set(add_label, FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
    lv_label_set_text(add_label, "Add");
    lv_obj_set_style_text_color(add_label, APP_COLOR_WHITE, 0);
    lv_obj_align(add_label, LV_ALIGN_CENTER, 0, 0);

    /* 键盘：点击输入框时弹出，放在右侧 */
    s_kb = lv_keyboard_create(cont);
    lv_keyboard_set_textarea(s_kb, ta);
    lv_keyboard_set_mode(s_kb, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_obj_set_size(s_kb, 700, 200);
    lv_obj_align(s_kb, LV_ALIGN_RIGHT_MID, -10, 0);

    /* 列表容器（可滚动） */
    s_list_cont = lv_obj_create(cont);
    lv_obj_set_size(s_list_cont, 760, 150);
    lv_obj_add_style(s_list_cont, &com_style, 0);
    lv_obj_set_style_bg_opa(s_list_cont, LV_OPA_TRANSP, 0);
    lv_obj_align_to(s_list_cont, ta, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 14);

    rebuild_list();
}
