#pragma once

#include <microhttpd.h>
#define HTTP_PORT 8086

#if defined(__linux__)
    #define USE_EPOLL 1
#else
    #define USE_SELECT 1
#endif

enum MHD_Result http_request_handler(void* cls, struct MHD_Connection* connection, const char* url, const char* method, const char* version, const char* upload_data, unsigned long* upload_data_size, void** con_cls);
void* http_init(void* arg);
