#ifdef __DMC__
#define InterlockedCompareExchange(p, n, o) InterlockedCompareExchange((void**)p, (void*)n, (void*)o)
#endif

#if defined(__dietlibc__)

#include <sys/atomic.h>
#elif defined(__STDC__) && (__STDC_VERSION__ >= 201112L) && !defined(__EMSCRIPTEN__)

#include <stdatomic.h>

static __inline long
__atomic_compare_and_swap(long* ptr, long oldval, long newval) {
#if defined(__ORANGEC__)
  atomic_compare_swap(ptr, &oldval, newval);
#else
  __atomic_compare_exchange_n(ptr, &oldval, newval, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
#endif
  return oldval;
}

#define __CAS __atomic_compare_and_swap
#define __CAS_PTR __atomic_compare_and_swap

#elif(defined(__i386__) || defined(__x86_64__)) && (defined(__TINYC__) || defined(__GNUC__))
#ifdef __GNUC__
#warning GNUC
#undef __sync_val_compare_and_swap
//#define __sync_val_compare_and_swap  __CAS
#else
#endif
#undef __CAS
#define __CAS __compare_and_swap
static __inline__ long
__compare_and_swap(long* ptr, long cmp, long new) {
  unsigned long prev;
  __asm__ volatile("lock;"
#if defined(__x86_64__)
                   "cmpxchgq %1, %2;"
#else
                   "cmpxchgl %1, %2;"
#endif
                   : "=a"(prev)
                   : "q"(new), "m"(*ptr), "a"(cmp)
                   : "memory");
  return prev;
}
#elif defined(__arm__)
typedef long(_cmpxchg_t)(long oldval, long newval, long* ptr);
static inline long
__CAS(long* ptr, long oldval, long newval) {
  long actual_oldval, fail;

  while(1) {
    actual_oldval = *ptr;

    if(__builtin_expect(oldval != actual_oldval, 0))
      return actual_oldval;

    fail = (*(_cmpxchg_t*)0xffff0fc0)(actual_oldval, newval, ptr);

    if(__builtin_expect(!fail, 1))
      return oldval;
  }
}

#elif WINDOWS_NATIVE || (defined(__CYGWIN__) && __MSYS__ == 1) || defined(__POCC__)
#include <windows.h>
#define __CAS(ptr, oldval, newval) InterlockedCompareExchange(ptr, newval, oldval)
#else
#define __CAS(ptr, oldval, newval) __sync_val_compare_and_swap(ptr, oldval, newval)
#endif

#ifndef __CAS
#define __CAS __sync_val_compare_and_swap
#endif
#ifndef __CAS_PTR
#define __CAS_PTR __CAS
#endif
