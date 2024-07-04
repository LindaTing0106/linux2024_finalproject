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

#define BUSYWAIT 10

#define set_lock(VAL)   (0x80000000u | (VAL))
#define unset_lock(VAL) (0x7FFFFFFFu & (VAL))
#define is_locked(VAL)  (0x80000000u & (VAL))

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
  uint32_t cur = cmpxchg(&mutex->futex_word, 0, set_lock(1)); 
    if (cur == 0) return; 
    for (uint32_t i = 0; i < BUSYWAIT; i++) { 
      if (is_locked(cur)) cur = unset_lock(cur) - 1; 
      uint32_t prev =
        cmpxchg(&mutex->futex_word, cur, set_lock(cur+1)); 
      if (prev == cur) return;
      cur = prev;
    }
    cur = fetch_add(&mutex->futex_word, 1, seq_cst) + 1;  
    for (;;) {  
      if (is_locked(cur)) {
        futex_wait(&mutex->futex_word, cur);
        cur = unset_lock(cur) - 1;
      }
      uint32_t prev =
        cmpxchg(&mutex->futex_word, cur, set_lock(cur)); 
      if (prev == cur) return; 
      cur = prev;
    }
}


static void Mutex_unlock(mutex_t* mutex)
{
  uint32_t prev = fetch_sub(&mutex->futex_word, set_lock(1), seq_cst);
    if (prev != set_lock(1)) {
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