#include "atomic.h"
#include <stdlib.h>
#include <pthread.h>


#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>


static inline void futex_wait(atomic int *futex_word, int value)
{
    syscall(SYS_futex, futex_word, FUTEX_WAIT, value, NULL, NULL, 0);
}

static inline void futex_wake(atomic int *futex_word, int limit)
{
    syscall(SYS_futex, futex_word, FUTEX_WAKE, limit, NULL, NULL, 0);
}

int cmpxchg(atomic int* futex_word, int expected, int desired) {
  compare_exchange_strong(futex_word, &expected, desired, seq_cst, seq_cst);
  return expected;
}

int xchg(atomic int * futex_word, int new_value) {
  return exchange(futex_word, new_value, seq_cst);
}

#if MUTEX==1
#include "drepper1.h"
#elif MUTEX==2
#include "drepper2.h"
#elif MUTEX==3
#include "drepper3.h"
#elif MUTEX==4
#include "drepper3b.h"
#elif MUTEX==5
#include "gustedt1.h"
#elif MUTEX==6
#include "gustedt2.h"
#else
#error MUTEX not defined to a recognised value
#endif

#if NTHREADS <= 0
#error Must define NTHREADS
#endif

#if ELEMS_PER_THREAD <= 0
#error Must define ELEMS_PER_THREAD
#endif

struct mutex_harness
{
  struct Mutex mutex;
  int counter;
};

static void* thread_func(struct mutex_harness *mutex_harness)
{
  for (int j = 0; j < ELEMS_PER_THREAD; j++) {
    mutex_harness->mutex.lock(&mutex_harness->mutex);
    mutex_harness->counter++;
    mutex_harness->mutex.unlock(&mutex_harness->mutex);
  }
}

int main() {
  struct mutex_harness mutex_harness;
  mutex_init(&mutex_harness.mutex, NULL);
  mutex_harness.counter = 0;
  pthread_t threads[NTHREADS];
  for (int i = 0; i < NTHREADS; i++) {
    pthread_create(&threads[i], NULL, thread_func, &mutex_harness);
  }
  for (int i = 0; i < NTHREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  const int expected_counter = NTHREADS * ELEMS_PER_THREAD;
  if (mutex_harness.counter != expected_counter) {
    printf("Final counter value is %d ; expected %d\n", mutex_harness.counter, expected_counter);
  } else {
    printf("Counter finished with expected value %d\n", expected_counter);
  }
}
