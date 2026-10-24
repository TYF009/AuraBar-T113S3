#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <locale.h>

#include "em_hal_time.h"
#include <pthread.h>
#include <spawn.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
extern char **environ;
static pthread_mutex_t ntp_lock = PTHREAD_MUTEX_INITIALIZER;
static int ntp_running;

static void *ntp_worker(void *arg) {
    (void)arg;
    char *argv[] = {"ntpd", "-n", "-q", "-p", "pool.ntp.org", NULL};
    pid_t pid = -1;
    if (!posix_spawnp(&pid, "ntpd", NULL, NULL, argv, environ)) {
        for (int i=0; i<200; ++i) {
            if (waitpid(pid, NULL, WNOHANG) == pid) { pid = -1; break; }
            struct timespec pause = {0, 100000000L};
            nanosleep(&pause, NULL);
        }
        if (pid > 0) { kill(pid, SIGTERM); while(waitpid(pid,NULL,0)<0 && errno==EINTR) {} }
    }
    pthread_mutex_lock(&ntp_lock); ntp_running=0; pthread_mutex_unlock(&ntp_lock);
    return NULL;
}
void em_hal_time_ntpd_update(void) {
    pthread_mutex_lock(&ntp_lock);
    if (!ntp_running) {
        pthread_t thread;
        ntp_running=1;
        if (pthread_create(&thread,NULL,ntp_worker,NULL)) ntp_running=0;
        else pthread_detach(thread);
    }
    pthread_mutex_unlock(&ntp_lock);
}

void em_hal_time_set_time(uint32_t year, uint32_t month, uint32_t day, 
						 uint32_t hour, uint32_t min, uint32_t second)
{
    struct tm _tm = {0};
    struct timeval tv;
    time_t timep;

    _tm.tm_isdst = -1;
    _tm.tm_year = year - 1900;
	_tm.tm_mon = month - 1;
	_tm.tm_mday = day;
	_tm.tm_hour = hour;
	_tm.tm_min = min;
	_tm.tm_sec = second;
	
    timep = mktime(&_tm);
    tv.tv_sec = timep;
    tv.tv_usec = 0;
	
    if(settimeofday(&tv, (struct timezone *) 0) < 0)
    {
        printf("settimeofday error");
    }
}

void em_hal_time_get_time(uint32_t* year, uint32_t* month, uint32_t* day,
	uint32_t* hour, uint32_t* min, uint32_t* second, uint32_t* day_week)
{
	time_t timep;   
    struct tm* _tm;
	
    timep = time(NULL);	
    _tm = localtime(&timep);

	*year = _tm->tm_year + 1900;
	*month = _tm->tm_mon + 1;
	*day = _tm->tm_mday;
	*hour = _tm->tm_hour;
	*min = _tm->tm_min;
	*second = _tm->tm_sec;
	*day_week = _tm->tm_wday;	
}


void em_hal_time_get_time_by_input(time_t *timep, uint32_t* year, uint32_t* month, uint32_t* day,
	uint32_t* hour, uint32_t* min, uint32_t* second, uint32_t* day_week)
{
    struct tm* _tm;
    _tm = localtime(timep);

	*year = _tm->tm_year + 1900;
	*month = _tm->tm_mon + 1;
	*day = _tm->tm_mday;
	*hour = _tm->tm_hour;
	*min = _tm->tm_min;
	*second = _tm->tm_sec;
	*day_week = _tm->tm_wday;	
}
