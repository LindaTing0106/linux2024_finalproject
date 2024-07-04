#pragma once

#if USE_PTHREADS

#include <pthread.h>

#define mutex_t pthread_mutex_t
#define mutexattr_t pthread_mutexattr_t
#define mutex_init(m, attr) pthread_mutex_init(m, attr)
#define mutex_destroy(m) pthread_mutex_destroy(m)
#define MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#define mutex_trylock(m) (!pthread_mutex_trylock(m))
#define mutex_lock pthread_mutex_lock
#define mutex_unlock pthread_mutex_unlock
#define mutexattr_setprotocol pthread_mutexattr_setprotocol
#define PRIO_NONE PTHREAD_PRIO_NONE
#define PRIO_INHERIT PTHREAD_PRIO_INHERIT

#else

#include "atomic.h"
#include <stdio.h>
#include <stdint.h>

typedef struct {
    int protocol;
} mutexattr_t;

typedef  struct Mutex mutex_t;
struct Mutex
{
  atomic int futex_word;
  void (*lock)(mutex_t*);
  void (*unlock)(mutex_t*);
};


static void Mutex_lock(mutex_t* mutex)
{
  int old_value; 
  if ((old_value = cmpxchg(&(mutex->futex_word), 0, 1)) != 0)
  {
    do {
      if (old_value == 2 
          || cmpxchg(&mutex->futex_word, 1, 2) != 0)
        futex_wait(&mutex->futex_word, 2);
      old_value = cmpxchg(&mutex->futex_word, 0, 2);     
      } while (old_value != 0);
  }
}


static void Mutex_unlock(mutex_t* mutex)
{
  if (fetch_sub(&mutex->futex_word, 1, seq_cst) != 1) {
      store(&mutex->futex_word, 0, seq_cst);
      futex_wake(&mutex->futex_word, 1);
  }
}

static inline void mutex_init(mutex_t* mutex, mutexattr_t* mutexattr)
{
    atomic_init(&mutex->futex_word, 0);

    // default method
    mutex->lock = Mutex_lock;
    mutex->unlock = Mutex_unlock;
}

static void mutex_lock(mutex_t* mutex)
{
  mutex->lock(mutex);
}

static void mutex_unlock(mutex_t* mutex)
{
  mutex->unlock(mutex);
}

static inline void mutex_destroy(mutex_t *mutex)
{
    /* Do nothing now, just for API convention. */
}

#endif