#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lvgl.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"
#include "utils.h"
#include "http_manager.h"
#include "audio_player_async.h"
#include "music_player.h"
#include "task_status.h"

extern void lv_port_disp_init(bool is_disp_orientation);
extern void lv_port_indev_init(void);
static uint32_t time_till_next = 1;

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        puts("AuraBar Coding Companion 0.4.0-alpha.1");
        return 0;
    }
    if (argc > 2 || (argc > 1 && strcmp(argv[1], "--music") != 0 && strcmp(argv[1], "--tasks") != 0)) {
        fprintf(stderr, "Usage: %s [--music|--tasks|--version]\n", argv[0]);
        return 2;
    }
    system_signal_init();
    init_device_state();
    device_state_t *state = get_device_state();
    lv_init();
    lv_port_disp_init(state->is_disp_orientation);
    lv_port_indev_init();
    FONT_INIT();
    device_timer_init();
    if (music_init() != 0) fprintf(stderr, "Music service failed to initialize\n");
    init_async_audio_player();
#ifdef SIMULATOR_LINUX
    // Linux模拟器无ble mesh
    printf("Running in Linux simulator mode.\n");
#else
    printf("Running in T113 mode.111\n");
    wpa_manager_open();
#endif
    http_request_create();
    if(task_status_init())fprintf(stderr,"Task link configuration is unavailable\n");
    if (argc > 1 && !strcmp(argv[1],"--tasks")) init_page_tasks();
    else if (argc > 1) init_page_music();
    else init_page_main();
    while (!system_should_exit()) {
        http_dispatch_pending();
        wpa_manager_dispatch_pending();
        if(task_status_take_notice())start_play_audio_async(GET_MUSIC_PATH("audio_msg.wav"));
        time_till_next = lv_task_handler();
        if (time_till_next > 20) time_till_next = 20;
        /**
         * 延时，保证cpu占有率不会过高
         */
        usleep(time_till_next*1000);
    }
    task_status_shutdown();
    music_stop();
    deinit_async_audio_player();
    music_shutdown();
    return 0;
}
