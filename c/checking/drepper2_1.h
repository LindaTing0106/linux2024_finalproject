#pragma once



#include <stdbool.h>
#include "atomic.h"
#include <stdio.h>
#include <stdint.h>
#include "spinlock.h"
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

static bool mutex_trylock(mutex_t *mutex)
{
    int old_value;

    if (old_value = cmpxchg(&(mutex->futex_word), 0, 1) !=0)
      return false;

    thread_fence(&mutex->futex_word, acquire);
    return true;
}

#define MUTEX_SPINS 128
static void Mutex_lock(mutex_t* mutex)
{
  for (int i = 0; i < MUTEX_SPINS; ++i) {
    if (mutex_trylock(mutex))
      return;
    spin_hint();
  }

  int old_value; 
  if ((old_value = cmpxchg(&(mutex->futex_word), 0, 1)) != 0)
  {
    do {
      if (old_value == 2 || cmpxchg(&mutex->futex_word, 1, 2) != 0)
        futex_wait(&mutex->futex_word, 2);
      } while (old_value = cmpxchg(&mutex->futex_word, 0, 2) != 0);
  }
  thread_fence(&mutex->futex_word, acquire);
}


static void Mutex_unlock(mutex_t* mutex)
{
  if (fetch_sub(&mutex->futex_word, 1, release) != 1) {
      store(&mutex->futex_word, 0, relaxed);
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

