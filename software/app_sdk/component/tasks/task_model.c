#include "task_status.h"
#include "cJSON/cJSON.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static int text(cJSON *obj,const char *name,char *out,size_t size,int id) {
    cJSON *v=cJSON_GetObjectItemCaseSensitive(obj,name);
    if(!cJSON_IsString(v) || strlen(v->valuestring)>=size)return -1;
    if(id && !v->valuestring[0])return -1;
    for(const unsigned char *p=(const unsigned char*)v->valuestring;*p;++p) {
        if(*p<32 || *p==127)return -1;
        if(id && !((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')||(*p>='0'&&*p<='9')||*p=='-'||*p=='_'))return -1;
    }
    snprintf(out,size,"%s",v->valuestring);return 0;
}
static int integer(cJSON *obj,const char *key,double max,double *out) {
    cJSON *v=cJSON_GetObjectItemCaseSensitive(obj,key);
    if(!cJSON_IsNumber(v)||!isfinite(v->valuedouble)||v->valuedouble<0||v->valuedouble>max||floor(v->valuedouble)!=v->valuedouble)return -1;
    *out=v->valuedouble;return 0;
}
static int optional_text(cJSON *obj,const char *key,char *out,size_t size) {
    if(!cJSON_GetObjectItemCaseSensitive(obj,key)) { out[0]=0;return 0; }
    return text(obj,key,out,size,0);
}
void task_model_age(task_model *m,double now) {
    if(m->view.connected && now-m->view.last_received>5) {
        m->view.connected=0;
        snprintf(m->view.message,sizeof(m->view.message),"Disconnected - showing last received state");
    }
}
int task_model_accept(task_model *m,const char *json,double now,int heartbeat_required) {
    task_snapshot next={.enabled=1};
    cJSON *root=cJSON_ParseWithOpts(json,NULL,1);
    double value;
    int ret=-1;
    if(!cJSON_IsObject(root)||integer(root,"schema",1,&value)||value!=1||
       text(root,"server_id",next.server_id,sizeof(next.server_id),1))goto done;
    if(integer(root,"revision",9007199254740991.0,&value))goto done;
    next.revision=(uint64_t)value;
    if(integer(root,"sample",9007199254740991.0,&value))goto done;
    next.sample=(uint64_t)value;
    cJSON *tasks=cJSON_GetObjectItemCaseSensitive(root,"tasks");
    if(!cJSON_IsArray(tasks)||(next.count=cJSON_GetArraySize(tasks))>TASK_LIMIT)goto done;
    for(int i=0;i<next.count;++i) {
        cJSON *item=cJSON_GetArrayItem(tasks,i);
        task_item *t=&next.tasks[i];
        char state[24];
        if(!cJSON_IsObject(item)||text(item,"id",t->id,sizeof(t->id),1)||
           text(item,"title",t->title,sizeof(t->title),0)||text(item,"stage",t->stage,sizeof(t->stage),0)||
           text(item,"detail",t->detail,sizeof(t->detail),0)||text(item,"state",state,sizeof(state),0))goto done;
        if(optional_text(item,"evidence",t->evidence,sizeof(t->evidence))||
           optional_text(item,"next_step",t->next_step,sizeof(t->next_step)))goto done;
        for(int j=0;j<i;++j)if(!strcmp(t->id,next.tasks[j].id))goto done;
        if(!strcmp(state,"running"))t->state=TASK_RUNNING;
        else if(!strcmp(state,"succeeded"))t->state=TASK_SUCCEEDED;
        else if(!strcmp(state,"failed"))t->state=TASK_FAILED;
        else if(!strcmp(state,"cancelled"))t->state=TASK_CANCELLED;
        else goto done;
        cJSON *p=cJSON_GetObjectItemCaseSensitive(item,"progress");
        t->progress=-1;
        if(!cJSON_IsNull(p)) {
            if(!cJSON_IsNumber(p)||!isfinite(p->valuedouble)||p->valuedouble<0||p->valuedouble>100)goto done;
            t->progress=(int)p->valuedouble;
        }
        if(integer(item,"elapsed_seconds",4294967295.0,&value))goto done;
        t->elapsed_seconds=(unsigned)value;
    }
    int same_server=m->have_baseline&&!strcmp(m->view.server_id,next.server_id);
    if(same_server && (next.sample<m->view.sample || next.revision<m->view.revision))goto done;
    if(heartbeat_required && same_server && next.sample==m->view.sample) {
        task_model_age(m,now);ret=0;goto done;
    }
    if(same_server) {
        for(int i=0;i<next.count;++i) {
            if(next.tasks[i].state==TASK_RUNNING)continue;
            int was_terminal=0;
            for(int j=0;j<m->view.count;++j)
                if(!strcmp(next.tasks[i].id,m->view.tasks[j].id) && m->view.tasks[j].state!=TASK_RUNNING)was_terminal=1;
            if(!was_terminal) { m->pending_notice=1;++m->notifications; }
        }
    }
    next.connected=!heartbeat_required||same_server;
    next.last_received=now;
    snprintf(next.message,sizeof(next.message),"%s",next.connected?"Connected":"Waiting for fresh USB heartbeat");
    m->view=next;m->have_baseline=1;ret=0;
done:
    cJSON_Delete(root);return ret;
}
