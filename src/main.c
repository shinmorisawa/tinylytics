#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "db.h"
#include "http.h"
#include "ipc.h"

volatile int is_running = 1;

void handler(int signo) {
    is_running = 0;
}

int main(void) {
    fprintf(stdout, "[main] installing signal handlers\n");
    fflush(stdout);
    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    pthread_t thread[3];

    if (pthread_create(&thread[0], NULL, db_init, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }

    if (pthread_create(&thread[1], NULL, http_init, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }

    if (pthread_create(&thread[2], NULL, ipc_init, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }

    fprintf(stdout, "[main] sleeping until signal..\n");
    fflush(stdout);

    while (is_running) {
        sleep(1);
    }

    is_running = 0;
    db_write(NULL);
    fprintf(stdout, "[main] exiting...\n");

    for (int i = 0; i < sizeof(thread) / sizeof(thread[0]); i++) {
        pthread_join(thread[i], NULL);
    }

    fprintf(stdout, "[main] joined threads\n");

    return 0;
}
