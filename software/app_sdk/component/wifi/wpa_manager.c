
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "common/wpa_ctrl.h"
#include "osal_thread.h"
#include "wpa_manager.h"
#include "base_utils.h"

//WiFi线程句柄
static osal_thread_t event_thread = NULL;
//wpa句柄
static struct wpa_ctrl *g_pstWpaCtrl = NULL;
static struct wpa_ctrl *event_ctrl = NULL;
static pthread_mutex_t command_lock = PTHREAD_MUTEX_INITIALIZER;
//WiFi状态
static WPA_WIFI_STATUS_E g_wifi_status = WPA_WIFI_CLOSE;
//连接状态
static WPA_WIFI_CONNECT_STATUS_E g_connect_status = WPA_WIFI_INACTIVE;
//回调函数
connect_status_callback_fun connect_status_func = NULL;
wifi_status_callback_fun wifi_status_func = NULL;
static pthread_mutex_t callback_lock = PTHREAD_MUTEX_INITIALIZER;
static int status_pending;
static WPA_WIFI_CONNECT_STATUS_E pending_status;
static void publish_connect_status(void) {
    pthread_mutex_lock(&callback_lock);
    pending_status = g_connect_status;
    status_pending = 1;
    pthread_mutex_unlock(&callback_lock);
}
void wpa_manager_dispatch_pending(void) {
    pthread_mutex_lock(&callback_lock);
    int pending = status_pending;
    WPA_WIFI_CONNECT_STATUS_E status = pending_status;
    status_pending = 0;
    pthread_mutex_unlock(&callback_lock);
    if (pending && connect_status_func) connect_status_func(status);
}


// 发送WiFi指令
static int wifi_send_command(const char *cmd,char *reply,size_t *length) {
    pthread_mutex_lock(&command_lock);
    int ret = g_pstWpaCtrl ? wpa_ctrl_request(g_pstWpaCtrl,cmd,strlen(cmd),reply,length,NULL) : -1;
    pthread_mutex_unlock(&command_lock);
    if(ret<0)fprintf(stderr,"WiFi control request failed\n");
    return ret;
}

//打开WiFi
static void wpa_manager_wifi_on(){
    printf("wpa_manager_wifi_on\n");
    char cmdstr[200];
    if(base_utils_get_process_state("wpa_supplicant",14) == 1)
     return;
    sprintf(cmdstr,"ifconfig %s up", STA_IFNAME);
    system(cmdstr);
    sprintf(cmdstr, "wpa_supplicant -i %s -c %s -B",STA_IFNAME,STA_CONFIG_PATH);
    system(cmdstr);
}

//连接Socket
static int wpa_manager_connect_socket(void) {
    char path[128];
    snprintf(path,sizeof(path),"/etc/wifi/wpa_supplicant/sockets/%s",STA_IFNAME);
    struct wpa_ctrl *command=wpa_ctrl_open(path);
    struct wpa_ctrl *event=wpa_ctrl_open(path);
    if(!command || !event || wpa_ctrl_attach(event)) {
        if(command)wpa_ctrl_close(command);
        if(event)wpa_ctrl_close(event);
        return -1;
    }
    pthread_mutex_lock(&command_lock);
    g_pstWpaCtrl=command;
    pthread_mutex_unlock(&command_lock);
    event_ctrl=event;
    return 0;
}

void wpa_manager_wifi_save_config(void)
{
    printf("wpa_manager_wifi_save_config\n");
    char reply_buf[256] = {0};
    size_t reply_len = sizeof(reply_buf) - 1;
    if (wifi_send_command("SAVE_CONFIG", reply_buf, &reply_len) == 0) {
        printf("SAVE_CONFIG--->status = %s\n",reply_buf);
    }
}

//查询WiFi状态
void wpa_manager_wifi_status(void)
{
    printf("wpa_manager_wifi_status\n");
    char reply_buf[256] = {0};
    size_t reply_len = sizeof(reply_buf) - 1;
    //发送STATUS，查询WiFi状态
    if (wifi_send_command("STATUS", reply_buf, &reply_len) == 0) {
        if (strstr(reply_buf, "COMPLETED") != NULL) {
            g_connect_status = WPA_WIFI_CONNECT;
        } else if (strstr(reply_buf, "DISCONNECTED") != NULL) {
            g_connect_status = WPA_WIFI_DISCONNECT;
        } else if (strstr(reply_buf, "SCANNING") != NULL) {
            g_connect_status = WPA_WIFI_SCANNING;
        } else if (strstr(reply_buf, "INACTIVE") != NULL) {
            g_connect_status = WPA_WIFI_INACTIVE;
        }
        printf("--->status = %d\n",g_connect_status);
        publish_connect_status();
    }
}

//连接WiFi
static int configure_network(const char *command, char *reply, size_t size) {
    size_t length = size - 1;
    int ret = wifi_send_command(command, reply, &length);
    reply[length < size ? length : size - 1] = 0;
    return ret || !strncmp(reply, "FAIL", 4) ? -1 : 0;
}
int wpa_manager_wifi_connect(wpa_ctrl_wifi_info_t *info) {
    if (!info) return -1;
    size_t ssid_len = strnlen(info->ssid, sizeof(info->ssid));
    size_t key_len = strnlen(info->psw, sizeof(info->psw));
    if (!ssid_len || ssid_len > 32 || key_len > 63 || (key_len && key_len < 8)) return -1;
    char reply[256], command[256], ssid[65], key[130];
    static const char hex[] = "0123456789abcdef";
    for (size_t i=0; i<ssid_len; ++i) {
        unsigned char c = (unsigned char)info->ssid[i];
        ssid[i*2]=hex[c>>4]; ssid[i*2+1]=hex[c&15];
    }
    ssid[ssid_len*2]=0;
    size_t used=0;
    for(size_t i=0;i<key_len;++i) {
        if(info->psw[i]=='"' || info->psw[i]=='\\')key[used++]='\\';
        key[used++]=info->psw[i];
    }
    key[used]=0;
    if(configure_network("ADD_NETWORK",reply,sizeof(reply)))return -1;
    char *endptr;
    long id=strtol(reply,&endptr,10);
    if(endptr==reply || id<0 || id>10000)return -1;
    snprintf(command,sizeof(command),"SET_NETWORK %ld ssid %s",id,ssid);
    if(configure_network(command,reply,sizeof(reply)))goto failed;
    if(key_len)snprintf(command,sizeof(command),"SET_NETWORK %ld psk \"%s\"",id,key);
    else snprintf(command,sizeof(command),"SET_NETWORK %ld key_mgmt NONE",id);
    if(configure_network(command,reply,sizeof(reply)))goto failed;
    snprintf(command,sizeof(command),"ENABLE_NETWORK %ld",id);
    if(configure_network(command,reply,sizeof(reply)))goto failed;
    snprintf(command,sizeof(command),"SELECT_NETWORK %ld",id);
    return configure_network(command,reply,sizeof(reply));
failed:
    snprintf(command,sizeof(command),"REMOVE_NETWORK %ld",id);
    configure_network(command,reply,sizeof(reply));
    return -1;
}

//WiFi线程
void *wpa_manager_event_thread(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    //打开WiFi
    wpa_manager_wifi_on();
    //连接wpa_supplicant
    for (int count = 0; count < 10; count++) {
        if (wpa_manager_connect_socket() == 0) break;
        osal_thread_sleep(1000);
        printf("wpa_manager_connect_socket\n");

    }
    //查询WiFi状态
    wpa_manager_wifi_status();
    // 事件处理循环
    while (1) {
        // 检查是否已建立与wpa_supplicant的连接，且有未处理的事件
        if (event_ctrl && wpa_ctrl_pending(event_ctrl) > 0) {
            char buf[512];
            size_t len = sizeof(buf) - 1;
            // 接收wpa_supplicant发送的事件数据
            if (wpa_ctrl_recv(event_ctrl, buf, &len) == 0) {
                buf[len] = '\0';
                // 处理"已连接"事件
                if (strstr(buf, "CTRL-EVENT-CONNECTED")) {
                    char cmd[64];
                    snprintf(cmd, sizeof(cmd), "udhcpc -i %s -t 5 -T 2 -A 5 -q", STA_IFNAME);
                    system(cmd);
                    wpa_manager_wifi_save_config();
                    g_connect_status = WPA_WIFI_CONNECT;
                } else if(strstr(buf, "CTRL-EVENT-DISCONNECTED") != NULL){
                    g_connect_status = WPA_WIFI_DISCONNECT;
                } else if (strstr(buf, "CTRL-EVENT-SSID-TEMP-DISABLED")) {
                    g_connect_status = WPA_WIFI_WRONG_KEY;
                }
                printf("--->status = %d\n",g_connect_status);
                publish_connect_status();
            }
        }
        // 休眠10毫秒，降低CPU占用
        osal_thread_sleep(10);
    }
    return NULL;
}

//注册WiFi状态监听函数
void wpa_manager_add_callback(wifi_status_callback_fun wifi_status_f,
                                connect_status_callback_fun connect_status_f){
    wifi_status_func = wifi_status_f;                       
    connect_status_func = connect_status_f;
}

//初始化WiFi，创建WiFi线程
int wpa_manager_open(){
    if (event_thread) return 0;
    int ret = osal_thread_create(&event_thread,wpa_manager_event_thread, NULL);
    if(ret == -1)
    {
        printf("create thread error\n");
        return ret;
    }
    return ret;
}
