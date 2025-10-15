#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ipc.h"

extern int is_running;

void* ipc_init(void* arg) {
    int client_sock, sock;
    struct sockaddr_un peer, addr;

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1)
        return NULL;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, IPC_PATH, sizeof(addr.sun_path) - 1);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        return NULL;

    if (listen(sock, LISTEN_BACKLOG) == -1)
        return NULL;

    socklen_t peer_size;

    fprintf(stdout, "[ipc] starting ipc server (WIP!!)\n");
    fflush(stdout);

    while (is_running) {
        peer_size = sizeof(peer);
        client_sock = accept(sock, (struct sockaddr*)&peer, &peer_size);

        if (client_sock == -1)
            continue;

        ipc_handle(client_sock);
    } 

    return NULL;
}

void ipc_send(int socket, const char* msg) {
    ssize_t n = write(socket, msg, strlen(msg));

    if (n < 0)
        fprintf(stdout, "[ipc] write failed");
}

/* CALLER MUST FREE!! */
char* ipc_recv(int socket, size_t size) {
    char* buf = malloc(size);
    ssize_t n = read(socket, buf, size);

    if (n < 0)
        fprintf(stdout, "[ipc] read failed");
    
    return buf;
}

void ipc_handle(int socket) {
    
}
