#pragma once

#if USE_PTHREADS

#include <pthread.h>

#define cond_t pthread_cond_t
#define cond_init(c) pthread_cond_init(c, NULL)
#define COND_INITIALIZER PTHREAD_COND_INITIALIZER
#define cond_wait(c, m) pthread_cond_wait(c, m)
#define cond_signal(c, m) pthread_cond_signal(c)
#define cond_broadcast(c, m) pthread_cond_broadcast(c)

#else

#include "atomic.h"
#include "drepper2.h"
#include <stdint.h>

typedef struct CondVar cond_t;
struct CondVar{
    atomic int futex_word;
    atomic int previous;
    void (*cv_wait)(cond_t* , mutex_t *);
    void (*cv_signal)(cond_t* , mutex_t *);
    //addtional
    void (*cv_broadcase)(cond_t* , mutex_t *);
};


static void condvar_wait(cond_t* cond, mutex_t *mutex)
{
    store(&cond->previous, &cond->futex_word, relaxed);
    int val = load(&cond->previous, relaxed);
    mutex_unlock(mutex);
    futex_wait(&cond->futex_word, val);
    mutex_lock(mutex);
}

static void condvar_signal(cond_t* cond, mutex_t *mutex)
{
    int val = 1 + load(&cond->previous, relaxed);
    store(&cond->futex_word, val, relaxed);
    futex_wake(&cond->futex_word, 1);
}
static void condvar_broadcase(cond_t* cond, mutex_t *mutex)
{
    int val = 1 + load(&cond->previous, relaxed);
    store(&cond->futex_word, val, relaxed);
    futex_requeue(&cond->futex_word, 1, &mutex->futex_word);
}

static inline void cond_init(cond_t *cond)
{
    atomic_init(&cond->futex_word, 0);

    // default method
    cond->cv_wait = condvar_wait;
    cond->cv_signal = condvar_signal;
    // addtional
    cond->cv_broadcase = condvar_broadcase;
}

static void cond_wait(cond_t* cond, mutex_t *mutex)
{
    cond->cv_wait(cond, mutex);
}

static void cond_signal(cond_t* cond, mutex_t *mutex)
{
    cond->cv_signal(cond, mutex);
}

static inline void cond_broadcast(cond_t *cond, mutex_t *mutex)
{
    cond->cv_broadcase(cond, mutex);
}

#endif