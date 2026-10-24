#ifndef _HTTP_MANAGER_H
#define _HTTP_MANAGER_H
#include <stddef.h>
#include <stdint.h>
typedef enum { NET_GET_WEATHER, NET_GET_LOCATION, NET_GET_CITY_SEARCH } NET_COMM_ID;
typedef struct {
    NET_COMM_ID id;
    uint32_t request_id;
    char host[128], path[512], data[64], type[10];
} net_obj;
typedef struct { char *data; size_t size; } http_resp_data_t;
typedef void (*weather_callback_fun)(char *);
typedef void (*location_callback_fun)(char *, double, double);
/* index=-1,total=0: no matches; index=-1,total=-1: request failed. */
typedef void (*search_callback_fun)(char *, char *, double, double, int, int);
int http_request_create(void);
void http_dispatch_pending(void); /* Call on the LVGL/UI thread only. */
void http_get_weather_by_coord(double lat, double lon);
void http_get_location_by_ip(void);
uint32_t http_search_city(const char *name);
void http_cancel_city_search(void);
void http_set_weather_callback(weather_callback_fun);
void http_set_location_callback(location_callback_fun);
void http_set_search_callback(search_callback_fun);
#endif
