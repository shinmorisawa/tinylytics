#pragma once
#include <pthread.h>

typedef struct Event {
    const char* query;
    int (*callback)(void*, int, char**, char**);
} Event;

typedef struct EventQueue {
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
    Event** queue;
    size_t size;
    size_t capacity;
} EventQueue;

void* db_init(void* arg);
void db_write(const char* query);
void db_read(Event* event);
void db_loop();
void db_queue_push(Event* e);
