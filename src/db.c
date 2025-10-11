#include <sqlite3.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "db.h"

static sqlite3* db;
EventQueue* q;

extern int is_running;

void* db_init(void* arg) {
    int rc;
    
    rc = sqlite3_open("tiny.sqlite", &db);
    if (rc) {
        fprintf(stderr, "[db] can't open db: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    q = malloc(sizeof(EventQueue));
    pthread_mutex_init(&q->queue_mutex, NULL);
    pthread_cond_init(&q->queue_cond, NULL);
    q->queue = NULL;
    q->size = 0;
    q->capacity = 0;

    db_write("BEGIN");
    db_write("CREATE TABLE IF NOT EXISTS hits ( id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp INTEGER NOT NULL, user_hash TEXT NOT NULL, path TEXT NOT NULL, extra TEXT )");
    db_write("CREATE TABLE IF NOT EXISTS users ( user_hash TEXT PRIMARY KEY, first_seen INTEGER, last_seen INTEGER, total_hits INTEGER DEFAULT 0 )");
    db_write("CREATE INDEX IF NOT EXISTS idx_hits_user ON hits(user_hash)");
    db_write("CREATE INDEX IF NOT EXISTS idx_hits_timestamp ON hits(timestamp)");
    db_write("CREATE INDEX IF NOT EXISTS idx_hits_path ON hits(path)");
    db_write("COMMIT");

    db_loop();

    for (size_t i = 0; i < q->size; i++) {
        free((void*)q->queue[i]->query);
        free(q->queue[i]);
    }

    free(q->queue);
    pthread_mutex_destroy(&q->queue_mutex);
    pthread_cond_destroy(&q->queue_cond);
    free(q);

    sqlite3_close(db);
    return NULL;
}

void db_write(const char* query) {
    Event* e = malloc(sizeof(Event));
    if (query == NULL) 
        e->query = NULL;
    else
        e->query = strdup(query);
    e->callback = NULL;

    db_queue_push(e);
}

void db_queue_push(Event* e) {
    pthread_mutex_lock(&q->queue_mutex);

    if (q->size == q->capacity) {
        size_t new = q->capacity ? q->capacity * 2 : 4;
        q->queue = realloc(q->queue, new * sizeof(Event*));
        q->capacity = new;
    } 

    q->queue[q->size++] = e;
    
    pthread_cond_signal(&q->queue_cond);
    pthread_mutex_unlock(&q->queue_mutex);
}

void db_queue_pop(Event** e) {
    pthread_mutex_lock(&q->queue_mutex);

    if (q->size == 0) {
        pthread_mutex_unlock(&q->queue_mutex);
        return;
    }

    *e = q->queue[0];
    memmove(q->queue, q->queue + 1, (q->size - 1) * sizeof(Event*));
    q->size--;

    pthread_mutex_unlock(&q->queue_mutex);
}

static void db_queue_pop_nolock(Event** e) {
    if (q->size == 0) {
        return;
    }

    *e = q->queue[0];
    memmove(q->queue, q->queue + 1, (q->size - 1) * sizeof(Event*));
    q->size--;
}

void db_read(const char* query, int (*row_callback)(void* what, int argc, char** argv, char** colnames)) {
    Event* e = malloc(sizeof(Event));
    e->query = strdup(query);
    e->callback = row_callback;

    db_queue_push(e);
}

void db_loop() {
    Event* e;
    char* error = NULL;
    while (is_running) {
        pthread_mutex_lock(&q->queue_mutex);

        while (q->size == 0 && is_running) {
            pthread_cond_wait(&q->queue_cond, &q->queue_mutex);
        }

        if (!is_running) {
            pthread_mutex_unlock(&q->queue_mutex);
            break;
        }

        db_queue_pop_nolock(&e);

        if (e->query == NULL) {
            pthread_mutex_unlock(&q->queue_mutex);
            break;
        }

        if (e) {
            fprintf(stdout, "[db] running query: %s\n", e->query);
            if (e->callback) {
                sqlite3_exec(db, e->query, e->callback, NULL, &error);
            } else {
                sqlite3_exec(db, e->query, NULL, NULL, &error);
            }

            if (error != SQLITE_OK) {
                fprintf(stderr, "[db] error: %s\n", error);
                sqlite3_free(error);
            }

            free((void*)e->query);
            free(e);
        }

        e = NULL;

        pthread_mutex_unlock(&q->queue_mutex);
    }
    return;
}
