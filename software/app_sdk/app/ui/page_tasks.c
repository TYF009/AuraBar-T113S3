/* Desktop Link page. Networking never touches these LVGL objects. */
#include "page_conf.h"
#include "task_status.h"
#include "font_conf.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static lv_obj_t *root,*connection,*title,*stage,*detail,*timing,*progress,*notice;
static lv_obj_t *rows[TASK_LIMIT],*row_labels[TASK_LIMIT];
static lv_timer_t *timer;
static char selected[64];
static int follow_latest=1;
static const char *state_name(task_state state) {
    switch(state) {
        case TASK_RUNNING:return "Running";
        case TASK_SUCCEEDED:return "Completed";
        case TASK_FAILED:return "Failed";
        default:return "Cancelled";
    }
}
static lv_color_t state_color(task_state state) {
    return lv_color_hex(state==TASK_SUCCEEDED?0x48d8ad:state==TASK_FAILED?0xff908b:0x6dbdff);
}
static lv_obj_t *label(lv_obj_t *parent,const char *text,int x,int y,int width,int size) {
    lv_obj_t *obj=lv_label_create(parent);
    lv_obj_set_pos(obj,x,y);lv_obj_set_width(obj,width);
    lv_label_set_long_mode(obj,LV_LABEL_LONG_DOT);
    lv_label_set_text(obj,text);
    lv_font_t *font=get_font(FONT_TYPE_CN,size);if(font)lv_obj_set_style_text_font(obj,font,0);
    lv_obj_set_style_text_color(obj,lv_color_hex(0xe6efff),0);return obj;
}
static void deleted(lv_event_t *e) {
    (void)e;if(timer)lv_timer_del(timer);timer=NULL;root=NULL;
}
static void navigate(lv_event_t *e) {
    intptr_t action=(intptr_t)lv_event_get_user_data(e);
    if(action==2) { follow_latest=1;return; }
    lv_obj_clean(lv_scr_act());
    if(action==1)init_page_music();else init_page_main();
}
static void select_row(lv_event_t *e) {
    task_snapshot s;task_status_snapshot(&s);
    int index=(int)(intptr_t)lv_event_get_user_data(e);
    if(index<s.count) { snprintf(selected,sizeof(selected),"%s",s.tasks[index].id);follow_latest=0; }
}
static void button(const char *text,int x,int width,intptr_t action) {
    lv_obj_t *b=lv_btn_create(root);lv_obj_set_pos(b,x,12);lv_obj_set_size(b,width,38);
    lv_obj_set_style_bg_color(b,lv_color_hex(0x183d60),0);
    lv_obj_add_event_cb(b,navigate,LV_EVENT_CLICKED,(void*)action);
    lv_obj_t *t=label(b,text,0,0,width-8,18);lv_obj_center(t);
}
static void refresh(lv_timer_t *unused) {
    (void)unused;if(!root)return;
    task_snapshot s;task_status_snapshot(&s);
    lv_label_set_text(connection,s.message);
    lv_obj_set_style_text_color(connection,lv_color_hex(s.connected?0x48d8ad:0xffce80),0);
    int current=-1;
    for(int i=0;i<s.count;++i)if(!strcmp(selected,s.tasks[i].id))current=i;
    if(follow_latest||current<0) {
        current=s.count?0:-1;
        for(int i=0;i<s.count;++i)if(s.tasks[i].state==TASK_RUNNING) { current=i;break; }
        if(current>=0)snprintf(selected,sizeof(selected),"%s",s.tasks[current].id);
    }
    for(int i=0;i<TASK_LIMIT;++i) {
        if(i>=s.count) { lv_obj_add_flag(rows[i],LV_OBJ_FLAG_HIDDEN);continue; }
        lv_obj_clear_flag(rows[i],LV_OBJ_FLAG_HIDDEN);
        char text[180];snprintf(text,sizeof(text),"%s  |  %s",state_name(s.tasks[i].state),s.tasks[i].title);
        lv_label_set_text(row_labels[i],text);
        lv_obj_set_style_bg_color(rows[i],lv_color_hex(i==current?0x214c73:0x102b45),0);
    }
    lv_label_set_text(notice,s.connected?"Task results update automatically. Music can keep playing.":"Offline: the last received result may be out of date.");
    if(current<0) {
        lv_label_set_text(title,"Ready for your next task");
        lv_label_set_text(stage,"Start a command with the desktop companion");
        lv_label_set_text(detail,"Builds, tests and AI jobs can report their actual state here.");
        lv_label_set_text(timing,"No task received");lv_obj_add_flag(progress,LV_OBJ_FLAG_HIDDEN);return;
    }
    task_item *t=&s.tasks[current];
    lv_label_set_text(title,t->title);lv_label_set_text(stage,t->stage);
    lv_obj_set_style_text_color(stage,state_color(t->state),0);
    lv_label_set_text(detail,t->detail);
    char status[160];
    if(t->progress<0)snprintf(status,sizeof(status),"%s  |  %u:%02u elapsed  |  Percentage unavailable",state_name(t->state),t->elapsed_seconds/60,t->elapsed_seconds%60);
    else snprintf(status,sizeof(status),"%s  |  %u:%02u elapsed  |  %d%%",state_name(t->state),t->elapsed_seconds/60,t->elapsed_seconds%60,t->progress);
    lv_label_set_text(timing,status);
    if(t->progress<0)lv_obj_add_flag(progress,LV_OBJ_FLAG_HIDDEN);
    else { lv_obj_clear_flag(progress,LV_OBJ_FLAG_HIDDEN);lv_bar_set_value(progress,t->progress,LV_ANIM_OFF); }
}
void init_page_tasks(void) {
    selected[0]=0;follow_latest=1;
    root=lv_obj_create(lv_scr_act());lv_obj_set_pos(root,0,0);lv_obj_set_size(root,LV_PCT(100),LV_PCT(100));
    lv_obj_clear_flag(root,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(root,lv_color_hex(0x071626),0);lv_obj_set_style_border_width(root,0,0);
    lv_obj_set_style_radius(root,0,0);lv_obj_set_style_pad_all(root,0,0);
    lv_obj_add_event_cb(root,deleted,LV_EVENT_DELETE,NULL);
    button("Back",16,80,0);button("Music",108,92,1);button("Latest",212,92,2);
    label(root,"Desktop Link",324,15,350,26);
    connection=label(root,"Connecting",750,22,650,16);
    lv_obj_t *list=lv_obj_create(root);lv_obj_set_pos(list,16,65);lv_obj_set_size(list,360,195);
    lv_obj_set_style_pad_all(list,4,0);lv_obj_set_style_bg_color(list,lv_color_hex(0x0d2339),0);
    lv_obj_set_style_border_width(list,0,0);lv_obj_set_flex_flow(list,LV_FLEX_FLOW_COLUMN);
    for(int i=0;i<TASK_LIMIT;++i) {
        rows[i]=lv_btn_create(list);lv_obj_set_size(rows[i],LV_PCT(100),42);
        lv_obj_set_style_pad_all(rows[i],8,0);lv_obj_set_style_radius(rows[i],8,0);
        row_labels[i]=label(rows[i],"",0,0,320,17);
        lv_obj_add_event_cb(rows[i],select_row,LV_EVENT_CLICKED,(void*)(intptr_t)i);
    }
    title=label(root,"",405,66,985,28);
    stage=label(root,"",405,108,985,20);
    detail=label(root,"",405,143,985,18);lv_obj_set_height(detail,52);
    timing=label(root,"",405,204,985,17);
    progress=lv_bar_create(root);lv_obj_set_pos(progress,405,233);lv_obj_set_size(progress,985,5);
    notice=label(root,"",405,245,985,14);
    timer=lv_timer_create(refresh,250,NULL);refresh(NULL);
}
