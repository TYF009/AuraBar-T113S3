#ifndef AURABAR_TASK_STATUS_H
#define AURABAR_TASK_STATUS_H
#include <stdint.h>
#define TASK_LIMIT 8
typedef enum { TASK_RUNNING, TASK_SUCCEEDED, TASK_FAILED, TASK_CANCELLED } task_state;
typedef struct {
    char id[64], title[128], stage[128], detail[512];
    char evidence[512], next_step[512]; /* Optional schema-1 review card fields. */
    task_state state;
    int progress; /* -1 means unavailable, never fabricate percentages. */
    unsigned elapsed_seconds;
} task_item;
typedef struct {
    int enabled, connected, count;
    uint64_t revision, sample;
    double last_received;
    char server_id[64], message[96];
    task_item tasks[TASK_LIMIT];
} task_snapshot;
typedef struct {
    task_snapshot view;
    int have_baseline, pending_notice;
    unsigned notifications;
} task_model;
int task_model_accept(task_model *model, const char *json, double now, int heartbeat_required);
void task_model_age(task_model *model, double now);
int task_status_init(void);
void task_status_shutdown(void);
void task_status_snapshot(task_snapshot *out);
int task_status_take_notice(void); /* Main thread queues notification audio. */
#endif
