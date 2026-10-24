/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 18:56:40 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-26 01:14:26
 */
#include "device_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "file_save.h"
#include "http_manager.h"
#include "em_hal_audio.h"
#include "em_hal_brightness.h"
#include "page_conf.h"
#include "countdown.h"


// #define USE_SYSTEM_TIMER 1
#ifndef USE_SYSTEM_TIMER
    #define USE_LVGL_TIMER 1
#endif

device_state_t device_state;

static time_t timep;
static struct tm time_temp;
static countdown focus_timer, alarm_timer;
static int focus_phase;
static uint64_t monotonic_ms(void) {
    struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t);
    return (uint64_t)t.tv_sec*1000+t.tv_nsec/1000000;
}
void device_focus_start(int rest_phase) {
    focus_phase=rest_phase?2:1;
    unsigned minutes=rest_phase?device_state.tomato_rest_time:device_state.tomato_learn_time;
    countdown_start(&focus_timer,(uint64_t)minutes*60000,monotonic_ms());
    device_state.tomato_time_state=ON;
}
void device_focus_set_running(bool running) {
    countdown_running(&focus_timer,running,monotonic_ms());
}
void device_focus_cancel(void) {
    countdown_running(&focus_timer,false,monotonic_ms());
    focus_phase=0; device_state.tomato_time_state=OFF;
}
void device_alarm_after_minutes(unsigned minutes) {
    if(minutes>1440)minutes=1440;
    device_state.alarm_time1=(int)minutes;
    device_state.is_open_type1=minutes!=0;
    countdown_start(&alarm_timer,(uint64_t)minutes*60000,monotonic_ms());
}
static void timer_count(void) {
    uint64_t now=monotonic_ms();
    if(device_state.is_open_type1) {
        uint64_t remaining=countdown_update(&alarm_timer,now);
        device_state.alarm_time1=(int)((remaining+59999)/60000);
        if(!remaining) {
            device_state.is_open_type1=false;
            init_page_alarm_remind(ALARM_ALARM_CLOCK_TRIGGER);
        }
    }
    if(device_state.is_open_type2) {
        timep=time(NULL); localtime_r(&timep,&time_temp);
        if(time_temp.tm_hour*60+time_temp.tm_min==device_state.alarm_time2) {
            device_state.is_open_type2=false;
            init_page_alarm_remind(ALARM_ALARM_CLOCK_TRIGGER);
        }
    }
    if(focus_phase) {
        uint64_t remaining=countdown_update(&focus_timer,now);
        unsigned minutes=(unsigned)((remaining+59999)/60000);
        if(focus_phase==1)device_state.tomato_learn_remaining_time=minutes;
        else device_state.tomato_rest_remaining_time=minutes;
        if(!remaining && focus_phase==2)device_state.tomato_time_state=OFF;
    }
}

#ifdef USE_LVGL_TIMER
static void lv_timer_cb_func(lv_timer_t * timer)
{
    // printf("lv_timer_cb_func timer run ...\n");
    timer_count();
}
#elif USE_SYSTEM_TIMER
static void timer_handler(int signum) {
    static int count = 0;
    printf("Timer tick %d\n", ++count);
    timer_count();
}
#endif


void device_timer_init(){
    printf("device_timer_init\n");
#ifdef USE_LVGL_TIMER
    lv_timer_t * device_timer = lv_timer_create(lv_timer_cb_func, 1000, NULL);
#elif USE_SYSTEM_TIMER
    struct itimerval system_timer;
    // 注册信号处理函数
    signal(SIGALRM, timer_handler);
    // 初始化定时器
    system_timer.it_value.tv_sec = 1;  // 初始延迟时间，1秒
    system_timer.it_value.tv_usec = 0; // 微秒部分
    system_timer.it_interval.tv_sec = 1;  // 重复间隔时间，1秒
    system_timer.it_interval.tv_usec = 0; // 微秒部分
    // 设置定时器
    if (setitimer(ITIMER_REAL, &system_timer, NULL) == -1) {
        perror("setitimer error");
    }
#endif
}

void device_param_write(){
    int ret = file_param_write("param.cfg",&device_state,sizeof(device_state));
    if(ret == 0)
        printf("device_param_write success\n");
}

void device_param_read(){
    int ret = file_param_read("param.cfg",&device_state,sizeof(device_state));
    if(ret == 0)
        printf("device_param_read success ble_mesh_state = %d \n",device_state.ble_mesh_state);
}

static void weather_callback_func(char *data){
    printf("weather: %s\n", data);
    snprintf(device_state.weather_info, sizeof(device_state.weather_info),
             "%s\n%s", device_state.weather_city, data);
}

static void location_callback_func(char *city, double lat, double lon){
    printf("location: %s (%.4f, %.4f)\n", city, lat, lon);
    strncpy(device_state.weather_city, city, sizeof(device_state.weather_city) - 1);
    device_state.latitude = lat;
    device_state.longitude = lon;
    device_param_write();
    http_get_weather_by_coord(lat, lon);
}

static void connect_status_callback_func(WPA_WIFI_CONNECT_STATUS_E status){
    printf("davicedata----->connect_status_callback_func: %d\n", status);
    device_state.wifi_connect_state = status;
}

static void wifi_status_callback_func(WPA_WIFI_STATUS_E status){
    printf("----->wifi_status_callback_func: %d\n", status);
    device_state.wifi_state = status;
}

void update_wpa_manager_callback_fun(){
    wpa_manager_add_callback(wifi_status_callback_func,connect_status_callback_func);
}

void init_device_state(){

    memset(&device_state,0,sizeof(device_state_t));

    device_state.time_type = TIME_TYPE_1;
    device_state.clock_type = 2;
    device_state.is_disp_orientation = false;

    device_param_read();
    /* Relative countdowns have no trustworthy persisted monotonic origin. */
    device_state.is_open_type1 = false;
    device_state.tomato_time_state = OFF;
    device_state.weather_city[sizeof(device_state.weather_city)-1] = 0;
    device_state.weather_info[sizeof(device_state.weather_info)-1] = 0;

    device_state.led_r = 0;
    device_state.led_g = 0;
    device_state.led_b = 0;

    device_state.led_connect_state = DISCONNECT;
    device_state.switch_connect_state = DISCONNECT;
    device_state.mesh_switch_state = OFF;
    device_state.body_sensor_state = OFF;

    device_state.smart_coaster_connect_state = DISCONNECT;
    device_state.smart_coaster_state = OFF;
    if(device_state.smart_coaster_time == 0){
        device_state.smart_coaster_time = 20;
        device_state.smart_coaster_remaining_time = 20;
    }

    device_state.body_sensor_connect_state = DISCONNECT;
    device_state.body_sensor_state = OFF;
    device_state.body_sensor_notice_type = NOTICE_TYPE_WIN_AND_AUDIO;
    
    device_state.flame_sensor_connect_state = DISCONNECT;
    device_state.flame_sensor_state = OFF;
    device_state.flame_sensor_data = 0;
    if(device_state.flame_sensor_time == 0){
        device_state.flame_sensor_time = 20;
        device_state.flame_sensor_remaining_time = 20;
    }

    device_state.switch_battery_level = 0;
    device_state.body_sensor_battery_level = 0;
    device_state.flame_sensor_battery_level = 0;
    device_state.smart_coaster_battery_level = 0;

    device_state.wifi_connect_state = WPA_WIFI_INACTIVE;
    device_state.ble_mesh_state = DISCONNECT;

    device_state.is_open_type1 = false;
    device_state.is_open_type2 = false;

    device_state.brightness_value = em_hal_brightness_get_value();
    if(device_state.brightness_value == 0){
        //避免黑屏情况
        em_hal_brightness_set_value(10);
    }
    if(device_state.volume_value != 0){
        em_set_audio_vol(device_state.volume_value);
    }else
        device_state.volume_value = em_get_audio_vol();

    if(strlen(device_state.weather_city) == 0)
        strcpy(device_state.weather_city, "Guangzhou");
    if(strlen(device_state.weather_info) == 0)
        strcpy(device_state.weather_info, "--°C");

    

    http_set_weather_callback(weather_callback_func);
    http_set_location_callback(location_callback_func);
    wpa_manager_add_callback(wifi_status_callback_func,connect_status_callback_func);
}

device_state_t* get_device_state(){
    return &device_state;
}