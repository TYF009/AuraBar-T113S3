#include "task_status.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
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
    assert(!m.view.tasks[0].evidence[0] && !m.view.tasks[0].next_step[0]);
    const char *card="{\"schema\":1,\"server_id\":\"server-b\",\"revision\":2,\"sample\":3,\"tasks\":[{\"id\":\"card\",\"title\":\"Codex\",\"stage\":\"Review\",\"detail\":\"AI summary\",\"evidence\":\"1 command exit 0\",\"next_step\":\"Human review\",\"state\":\"succeeded\",\"progress\":null,\"elapsed_seconds\":1}]}";
    assert(!task_model_accept(&m,card,15,1));
    assert(!strcmp(m.view.tasks[0].evidence,"1 command exit 0"));
    assert(!strcmp(m.view.tasks[0].next_step,"Human review"));
    snprintf(json,sizeof(json),"%s",card);
    char *bad=strstr(json,"\"evidence\":\"1 command exit 0\"");
    assert(bad);
    char *value=bad+strlen("\"evidence\":");
    size_t old_length=strlen("\"1 command exit 0\"");
    memmove(value+4,value+old_length,strlen(value+old_length)+1);memcpy(value,"null",4);
    assert(task_model_accept(&m,json,16,1));
    puts("task model: validation, unknown progress, heartbeat expiry, restart and notification deduplication passed");
}
