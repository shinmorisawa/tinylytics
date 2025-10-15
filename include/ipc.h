#pragma once

#include <stdlib.h>

#define IPC_PATH "/tmp/.tinylytics.sock"
#define LISTEN_BACKLOG 128

void* ipc_init(void* arg);
void ipc_send(int socket, const char* msg);
char* ipc_recv(int socket, size_t size);
int ipc_db_callback(void* unused, int argc, char** argv, char** columns);
void ipc_handle(int socket);
