#include "task_status.h"
#include <assert.h>
#include <stdio.h>
static char json[2048];
static const char *snapshot(const char *server,int sample,int revision,const char *state,const char *percentage) {
    snprintf(json,sizeof(json),"{\"schema\":1,\"server_id\":\"%s\",\"revision\":%d,\"sample\":%d,\"tasks\":[{\"id\":\"job-1\",\"title\":\"Build\",\"stage\":\"Compiler\",\"detail\":\"Actual task\",\"state\":\"%s\",\"progress\":%s,\"elapsed_seconds\":23}]}",server,revision,sample,state,percentage);return json;
}
int main(void) {
    task_model m={0};
    assert(!task_model_accept(&m,snapshot("server-a",1,1,"running","null"),0,1));
    assert(!m.view.connected && m.view.tasks[0].progress==-1 && !m.notifications);
    assert(!task_model_accept(&m,snapshot("server-a",2,1,"running","null"),1,1));
    assert(m.view.connected);
    assert(!task_model_accept(&m,snapshot("server-a",3,2,"succeeded","100"),2,1));
    assert(m.pending_notice && m.notifications==1);
    m.pending_notice=0;
    assert(!task_model_accept(&m,json,3,1));assert(!m.pending_notice && m.notifications==1);
    task_model_age(&m,8);assert(!m.view.connected);
    assert(!task_model_accept(&m,json,9,1));assert(!m.view.connected);
    assert(!task_model_accept(&m,snapshot("server-a",4,2,"succeeded","100"),10,1));
    assert(m.view.connected && m.notifications==1);
    assert(task_model_accept(&m,snapshot("server-a",5,3,"unknown","50"),11,1));
    assert(task_model_accept(&m,snapshot("server-a",5,3,"running","101"),11,1));
    assert(task_model_accept(&m,"{bad json}",11,1));
    assert(m.view.sample==4 && m.view.tasks[0].state==TASK_SUCCEEDED);
    assert(!task_model_accept(&m,snapshot("server-b",1,1,"failed","null"),12,1));
    assert(!m.view.connected && m.notifications==1);
    assert(!task_model_accept(&m,snapshot("server-b",2,1,"failed","null"),13,1));
    assert(m.view.connected && m.notifications==1);
    assert(task_model_accept(&m,snapshot("server-b",1,1,"failed","null"),14,1));
    puts("task model: validation, unknown progress, heartbeat expiry, restart and notification deduplication passed");
}
