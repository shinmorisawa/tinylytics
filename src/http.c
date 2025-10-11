#include <microhttpd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include "http.h"
#include "db.h"

extern int is_running;


enum MHD_Result http_request_handler(void* cls, struct MHD_Connection* connection, const char* url, const char* method, const char* version, const char* upload_data, unsigned long* upload_data_size, void** con_cls) {
    if (*con_cls == NULL) {
        *con_cls = malloc(sizeof(int));
        return MHD_YES;
    }

    if (*upload_data_size != 0) {
        if (strcmp(url, "/track") == 0 && *upload_data_size > 128) {
            char hash[129];
            hash[128] = '\0';
            
            memcpy(hash, upload_data, 128);

            size_t path_len = (*upload_data_size > 128) ? (*upload_data_size - 128) : 1;
            char* path = malloc(path_len + 1);
            memset(path, 0, path_len + 1);
            
            if (*upload_data_size > 128)
                memcpy(path, upload_data + 128, *upload_data_size - 128);
            else
                path[0] = '/';

            time_t now = time(NULL);
            int len = snprintf(NULL, 0, "INSERT INTO hits (timestamp, user_hash, path) VALUES (%ld, '%s', '%s')", now, hash, path);
            
            char* buf = malloc(len + 1);
            snprintf(buf, len + 1, "INSERT INTO hits (timestamp, user_hash, path) VALUES (%ld, '%s', '%s')", now, hash, path);
            
            db_write(buf);
            
            free(path);
            free(buf);
        }

        *upload_data_size = 0;
        return MHD_YES;
    }

    struct MHD_Response* response = NULL;

    if (strcmp(url, "/track") == 0) {
        response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
        MHD_queue_response(connection, MHD_HTTP_NO_CONTENT, response);
    } else if (strcmp(url, "/ip") == 0) {
        const union MHD_ConnectionInfo* ci = MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS);
        struct sockaddr* addr = (struct sockaddr*)ci->client_addr;
        char ip[INET6_ADDRSTRLEN];

        if (addr->sa_family == AF_INET) {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)addr;
            inet_ntop(AF_INET, &ipv4->sin_addr, ip, sizeof(ip));
        } else if (addr->sa_family == AF_INET6) {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)addr;
            inet_ntop(AF_INET6, &ipv6->sin6_addr, ip, sizeof(ip));
        }

        fprintf(stdout, "[http] sending client ip address\n");
        fflush(stdout);

        response = MHD_create_response_from_buffer(strlen(ip), (void*)&ip, MHD_RESPMEM_MUST_COPY);
        MHD_queue_response(connection, MHD_HTTP_OK, response);
    }

    if (response)
        MHD_destroy_response(response);

    free(*con_cls);
    *con_cls = NULL;

    return MHD_YES;
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
