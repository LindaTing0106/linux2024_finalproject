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
#include "mutex.h"
#include "futex.h"


typedef struct CondVar cond_t;
struct CondVar{
    atomic uint32_t futex_word;
    void (*cv_wait)(cond_t* , mutex_t *);
    void (*cv_signal)(cond_t* , mutex_t *);
    //addtional
    void (*cv_broadcase)(cond_t* , mutex_t *)
};


static void condvar_wait(cond_t* cond, mutex_t *mutex)
{
    uint32_t old_value = cond->futex_word; 
    mutex_unlock(mutex);
    futex_wait(&cond->futex_word, old_value);
    mutex_lock(mutex);
}

static void condvar_signal(cond_t* cond, mutex_t *mutex)
{
    fetch_add(&cond->futex_word, 1, relaxed);
    futex_wake(&cond->futex_word, 1);
}
static void condvar_broadcase(cond_t* cond, mutex_t *mutex)
{
    fetch_add(&cond->futex_word, 1, relaxed);
    futex_requeue(&cond->futex_word, 1, &mutex->state);
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