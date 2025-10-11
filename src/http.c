#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "http.h"

extern int is_running;

enum MHD_Result http_request_handler(void* cls, struct MHD_Connection* connection, const char* url, const char* method, const char* version, const char* upload_data, unsigned long* upload_data_size, void** con_cls) {
    if (*con_cls == NULL) {
        *con_cls = malloc(1);
        return MHD_YES;
    }

    if (strcmp(method, "POST") != 0) 
        return MHD_NO;

    if (*upload_data_size != 0) {
        fprintf(stdout, "[http] received post data: %.*s\n", (int)*upload_data_size, upload_data);
        fflush(stdout);
        *upload_data_size = 0;
        return MHD_YES;
    }

    struct MHD_Response* response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_NO_CONTENT, response);
    MHD_destroy_response(response);
    free(*con_cls);
    *con_cls = NULL;
    return ret;
}

void* http_init(void* arg) {
    struct MHD_Daemon* daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, HTTP_PORT, NULL, NULL, &http_request_handler, NULL, MHD_OPTION_END);
    if (!daemon)
        fprintf(stderr, "[http] init failed");

    fprintf(stdout, "[http] started on port %d\n", HTTP_PORT);
    fflush(stdout);

    while (is_running) {
        sleep(1);
    }

    fprintf(stdout, "[http] stopping cleanly\n");
    fflush(stdout);

    MHD_stop_daemon(daemon);
    return NULL;
}
