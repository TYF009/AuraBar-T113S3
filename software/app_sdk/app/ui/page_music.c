/* AuraBar Music & Focus: UI timers and objects have one page owner. */
#include "page_conf.h"
#include "music_player.h"
#include "task_status.h"
#include "device_data.h"
#include "font_conf.h"
#include "color_conf.h"
#include <stdint.h>
#include <stdio.h>

static lv_timer_t *timer;
static lv_obj_t *root, *title, *state, *time_text, *seek_bar, *play_text, *repeat_text, *sleep_text, *favorite_text;
static int seeking;
static lv_obj_t *task_text;
static const char *state_name(music_state s) {
    switch(s) {
        case MUSIC_BUFFERING:return "Loading";
        case MUSIC_PLAYING:return "Playing";
        case MUSIC_PAUSED:return "Paused";
        case MUSIC_ERROR:return "Playback unavailable";
        default:return "Ready";
    }
}
static void font(lv_obj_t *obj,int size) {
    lv_font_t *f=get_font(FONT_TYPE_LETTER,size);
    if(f)lv_obj_set_style_text_font(obj,f,0);
}
static lv_obj_t *label(lv_obj_t *parent,const char *text,int x,int y,int width,int size) {
    lv_obj_t *obj=lv_label_create(parent);
    lv_label_set_text(obj,text); font(obj,size);
    lv_obj_set_width(obj,width); lv_obj_set_pos(obj,x,y);
    lv_obj_set_style_text_color(obj,lv_color_hex(0xe5edff),0);
    lv_label_set_long_mode(obj,LV_LABEL_LONG_DOT);
    return obj;
}
static lv_obj_t *button(lv_obj_t *parent,const char *text,int x,int y,int width,lv_event_cb_t cb,void *data) {
    lv_obj_t *b=lv_btn_create(parent);
    lv_obj_set_pos(b,x,y); lv_obj_set_size(b,width,42);
    lv_obj_set_style_radius(b,12,0);
    lv_obj_set_style_bg_color(b,lv_color_hex(0x16406a),0);
    lv_obj_add_event_cb(b,cb,LV_EVENT_CLICKED,data);
    lv_obj_t *t=label(b,text,0,0,width-12,20);
    lv_obj_center(t);
    return t;
}
static void page_deleted(lv_event_t *e) {
    (void)e;
    if(timer)lv_timer_del(timer);
    timer=NULL; root=NULL; seeking=0;
}
static void back(lv_event_t *e) {
    (void)e;
    lv_obj_clean(lv_scr_act());
    init_page_main();
}
static void tasks(lv_event_t *e) {
    (void)e;lv_obj_clean(lv_scr_act());init_page_tasks();
}
static void select_track(lv_event_t *e) { music_play((int)(intptr_t)lv_event_get_user_data(e)); }
static void transport(lv_event_t *e) {
    intptr_t action=(intptr_t)lv_event_get_user_data(e);
    if(action==-1)music_next(-1);
    else if(action==1)music_next(1);
    else if(action==2)music_stop();
    else music_toggle_pause();
}
static void repeat(lv_event_t *e) {
    (void)e;
    music_snapshot s; music_get_snapshot(&s); music_set_repeat(!s.repeat);
}
static void favorite(lv_event_t *e) { (void)e; music_toggle_favorite(); }
static void sleep_changed(lv_event_t *e) {
    static const unsigned minutes[]={0,15,30,60,90};
    unsigned i=lv_dropdown_get_selected(lv_event_get_target(e));
    if(i<sizeof(minutes)/sizeof(*minutes))music_sleep_after(minutes[i]);
}
static void seek_changed(lv_event_t *e) {
    lv_event_code_t code=lv_event_get_code(e);
    if(code==LV_EVENT_PRESSED)seeking=1;
    if(code==LV_EVENT_RELEASED) {
        music_snapshot s; music_get_snapshot(&s);
        music_seek(s.duration*lv_slider_get_value(seek_bar)/1000.0);
        seeking=0;
    }
    if(code==LV_EVENT_PRESS_LOST)seeking=0;
}
static void volume_changed(lv_event_t *e) {
    int value=lv_slider_get_value(lv_event_get_target(e));
    if(em_set_audio_vol(value)==0) {
        get_device_state()->volume_value=value;
        device_param_write();
    }
}
static void update(lv_timer_t *t) {
    (void)t;
    if(!root)return;
    music_snapshot s; music_get_snapshot(&s);
    task_snapshot jobs;task_status_snapshot(&jobs);
    char task_line[180];
    if(jobs.count)snprintf(task_line,sizeof(task_line),"%s: %s%s",jobs.tasks[0].state==TASK_RUNNING?"Running":jobs.tasks[0].state==TASK_SUCCEEDED?"Completed":jobs.tasks[0].state==TASK_FAILED?"Failed":"Cancelled",jobs.tasks[0].title,jobs.connected?"":" (offline)");
    else snprintf(task_line,sizeof(task_line),"Your music, a quieter workspace.");
    lv_label_set_text(task_text,task_line);
    lv_label_set_text(title,s.title);
    lv_label_set_text(state,s.state==MUSIC_ERROR?s.error:state_name(s.state));
    lv_label_set_text(play_text,s.state==MUSIC_PLAYING||s.state==MUSIC_BUFFERING?"Pause":"Play");
    lv_label_set_text(repeat_text,s.repeat?"Repeat: On":"Repeat: Off");
    lv_label_set_text(favorite_text,s.favorite?"Saved":"Save");
    unsigned pos=(unsigned)s.position, dur=(unsigned)s.duration;
    char times[80];
    if(dur)snprintf(times,sizeof(times),"%02u:%02u / %02u:%02u",pos/60,pos%60,dur/60,dur%60);
    else if(s.state==MUSIC_STOPPED)snprintf(times,sizeof(times),"Select a track, then Play");
    else snprintf(times,sizeof(times),"%02u:%02u  |  Live / continuous",pos/60,pos%60);
    lv_label_set_text(time_text,times);
    if(s.duration>0 && s.state!=MUSIC_STOPPED && s.state!=MUSIC_ERROR)
        lv_obj_clear_state(seek_bar,LV_STATE_DISABLED);
    else lv_obj_add_state(seek_bar,LV_STATE_DISABLED);
    if(!seeking) {
        int value=dur?(int)(s.position*1000/s.duration):0;
        if(value>1000)value=1000;
        lv_slider_set_value(seek_bar,value,LV_ANIM_OFF);
    }
    char text[64];
    if(s.sleep_seconds)snprintf(text,sizeof(text),"Stops in %u:%02u",s.sleep_seconds/60,s.sleep_seconds%60);
    else snprintf(text,sizeof(text),"Sleep timer off");
    lv_label_set_text(sleep_text,text);
}
void init_page_music(void) {
    root=lv_obj_create(lv_scr_act());
    lv_obj_set_size(root,LV_PCT(100),LV_PCT(100)); lv_obj_set_pos(root,0,0);
    lv_obj_clear_flag(root,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(root,lv_color_hex(0x071526),0);
    lv_obj_set_style_border_width(root,0,0); lv_obj_set_style_radius(root,0,0);
    lv_obj_set_style_pad_all(root,0,0);
    lv_obj_add_event_cb(root,page_deleted,LV_EVENT_DELETE,NULL);
    button(root,"Back",16,12,80,back,NULL);
    label(root,"Music & Focus",116,16,420,28);
    task_text=label(root,"",720,20,520,18);
    button(root,"Tasks",1260,12,130,tasks,NULL);

    lv_obj_t *list=lv_list_create(root);
    lv_obj_set_pos(list,16,66); lv_obj_set_size(list,310,194);
    lv_obj_set_style_bg_color(list,lv_color_hex(0x0d233d),0);
    lv_obj_set_style_border_width(list,0,0);
    music_snapshot s; music_get_snapshot(&s);
    for(int i=0;i<s.count;++i) {
        char name[128]; music_track_title(i,name,sizeof(name));
        lv_obj_t *b=lv_list_add_btn(list,NULL,name);
        lv_obj_set_style_bg_color(b,lv_color_hex(0x0d233d),0);
        lv_obj_set_style_text_color(b,lv_color_hex(0xe5edff),0); font(b,18);
        lv_obj_add_event_cb(b,select_track,LV_EVENT_CLICKED,(void*)(intptr_t)i);
    }
    title=label(root,s.title,352,70,680,28);
    state=label(root,"Ready",352,112,680,18);
    seek_bar=lv_slider_create(root);
    lv_obj_set_pos(seek_bar,360,155); lv_obj_set_size(seek_bar,650,8);
    lv_slider_set_range(seek_bar,0,1000);
    lv_obj_add_event_cb(seek_bar,seek_changed,LV_EVENT_ALL,NULL);
    time_text=label(root,"00:00",352,173,400,16);
    button(root,"Prev",352,210,80,transport,(void*)(intptr_t)-1);
    play_text=button(root,"Play",442,210,100,transport,NULL);
    button(root,"Next",552,210,80,transport,(void*)(intptr_t)1);
    button(root,"Stop",642,210,80,transport,(void*)(intptr_t)2);
    repeat_text=button(root,"Repeat: Off",736,210,140,repeat,NULL);

    favorite_text=button(root,"Save",886,210,140,favorite,NULL);

    label(root,"Volume",1070,70,300,20);
    lv_obj_t *volume=lv_slider_create(root);
    lv_obj_set_pos(volume,1080,112); lv_obj_set_size(volume,290,10);
    lv_slider_set_range(volume,0,100);
    lv_slider_set_value(volume,get_device_state()->volume_value,LV_ANIM_OFF);
    lv_obj_add_event_cb(volume,volume_changed,LV_EVENT_RELEASED,NULL);
    label(root,"Sleep timer",1070,150,300,20);
    lv_obj_t *sleep=lv_dropdown_create(root);
    lv_obj_set_pos(sleep,1070,183); lv_obj_set_size(sleep,300,40);
    lv_dropdown_set_options(sleep,"Off\n15 minutes\n30 minutes\n60 minutes\n90 minutes");
    lv_dropdown_set_symbol(sleep,NULL);
    font(sleep,18);
    lv_obj_add_event_cb(sleep,sleep_changed,LV_EVENT_VALUE_CHANGED,NULL);
    sleep_text=label(root,"",1070,232,300,16);
    timer=lv_timer_create(update,200,NULL);
    update(NULL);
}
