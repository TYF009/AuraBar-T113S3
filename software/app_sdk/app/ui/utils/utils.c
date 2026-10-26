/*
 * @Author: xiaozhi 
 * @Date: 2024-09-25 00:07:46 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-26 02:56:32
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "lvgl.h"
#include "utils.h"

static char time[32];

char *get_time_str(int time_min) {
    if(time_min<0)time_min=0;
    snprintf(time,sizeof(time),"%02d:%02d",time_min/60,time_min%60);
    return time;
}

char *get_time_str_nosymbol(int time_min) {
    if(time_min<0)time_min=0;
    snprintf(time,sizeof(time),"%02d#000000 :#%02d",time_min/60,time_min%60);
    return time;
}

static volatile sig_atomic_t exit_requested;
static void signal_callback_func(int sig_no) { (void)sig_no; exit_requested = 1; }
int system_should_exit(void) { return exit_requested != 0; }
void system_signal_init(void) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = signal_callback_func;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    /* Leave fault signals and job control to their normal OS behavior. */
}

// typedef enum {
//     WIFI_DISCONNECTED=0,
//     WIFI_CONNECTED,
//     INVAILD_STATUS,
// }WIFI_STATUS_E;

// #define READ_WIFI_STATUS_CMD  "cat /sys/class/net/%s/carrier"
// #define WIFI_INTERFACE  "wlan0"

// WIFI_STATUS_E  get_wifi_status(void)
// {
//     int status = 0;
//     int bytes = 0;
//     char cmd[100];
//     char buf[500];
//     char *p =NULL;
//     memset(cmd,0,sizeof(cmd));
//     memset(buf,0,sizeof(buf));
//     sprintf(cmd,READ_WIFI_STATUS_CMD,WIFI_INTERFACE);
//     if(bytes > 0){
//         status = atoi(buf);
//         if(status == 1){
//             return WIFI_CONNECTED;
//         }
//         else if(status == 0){
//              return WIFI_DISCONNECTED;
//         }
//         else
//             return INVAILD_STATUS;
//     }
// }