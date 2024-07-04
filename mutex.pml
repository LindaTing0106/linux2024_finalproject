/* 
 * Test with 3 threads, checking critical section safety:
 *   spin -DNUM_THREADS=3 -search -ltl safe_cs -m100000 -b mutex.pml
 *
 * Test with 3 threads, checking for invalid end states:
 *   spin -DNUM_THREADS=3 -search -noclaim -m100000 -b mutex.pml
 */

#include "futex.pml"
#include "atomics.pml"

Futex futex;

inline trylock(bools ,state) {
    atomic {
        fetch_or(futex.word, 1 , state);
    }
    if
    :: state == 0 ->
        bools = 1;
    :: else ->
        bools = 0;
    fi
}

inline lock() {
  byte state;
  byte spins;
  byte bools;
  do 
  :: atomic {
       trylock(bools, state);
    }
    if 
       :: spins == 1 -> break;
       :: else 
       if
       :: bools == 1 -> goto end
       :: else -> atomic {spins = spins + 1};
       fi
    fi
  od
  
  atomic {
    xchg(futex.word, 3 , state);
  }
  atomic {
    spins = 0;
  }
  
  do
  :: atomic {
       if
       :: state == 0 ->
       break;
       :: else ->
          futex_wait(futex, 3)
          xchg(futex.word, 3 , state);
       fi
     }
  od
  end:
}

inline unlock() {
  byte state;
  d_step {
    xchg(futex.word, 0, state);
    printf("T%d decrements futex word from %d to %d\n",
           _pid, state, futex.word)
  }
  if
  :: d_step {
       state != 0 ->
       futex_wake(futex, 1)
     }
  fi
}

/*****************************************************************************/

#include "mutex_generic.pml"

/*****************************************************************************/
