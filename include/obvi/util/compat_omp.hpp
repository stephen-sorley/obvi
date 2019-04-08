/* Compatibility header for conditionally including OpenMP.
 *
 * If code was built with OpenMP enabled, this will include <omp.h> and define various macros
 * that can be used to conditionally include OpenMP directives from later versions of OpenMP.
 *
 * If code was built with OpenMP disabled, this will define a bunch of stub functions inline.
 *
 * Stub functions taken from the OpenMP 2.5 spec, Appendix B.1, found here:
 *   https://www.openmp.org/wp-content/uploads/spec25.pdf
 *
 * * * * * * * * * * * *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Stephen Sorley
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * * * * * * * * * * * *
 */
#ifndef OBVI_COMPAT_OMP_HPP
#define OBVI_COMPAT_OMP_HPP


#ifdef _OPENMP // If built with OpenMP support enabled:
#   include <omp.h>

#   if _OPENMP >= 200505 && !defined(OPENMP_2_5)
#       define OPENMP_2_5
#   endif
#   if _OPENMP >= 200805 && !defined(OPENMP_3_0)
#       define OPENMP_3_0
#   endif
#   if _OPENMP >= 201107 && !defined(OPENMP_3_1)
#       define OPENMP_3_1
#   endif
#   if _OPENMP >= 201307 && !defined(OPENMP_4_0)
#       define OPENMP_4_0
#   endif
#   if _OPENMP >= 201511 && !defined(OPENMP_4_5)
#       define OPENMP_4_5
#   endif
#   if _OPENMP >= 201811 && !defined(OPENMP_5_0)
#       define OPENMP_5_0
#   endif

#else // If not built with OpenMP support enabled:
#   include <stdio.h>
#   include <stdlib.h>
static inline void omp_set_num_threads(int num_threads) {}
static inline int omp_get_num_threads(void) { return 1; }
static inline int omp_get_max_threads(void) { return 1; }
static inline int omp_get_thread_num(void) { return 0; }
static inline int omp_get_num_procs(void) { return 1; }
static inline void omp_set_dynamic(int dynamic_threads) {}
static inline int omp_get_dynamic(void) { return 0; }
static inline int omp_in_parallel(void) { return 0; }
static inline void omp_set_nested(int nested) {}
static inline int omp_get_nested(void) { return 0; }

enum omp_lock_t {
    UNLOCKED = -1,
    INIT,
    LOCKED
};
static inline void omp_init_lock(omp_lock_t *lock) {
    *lock = UNLOCKED;
}
static inline void omp_destroy_lock(omp_lock_t *lock) {
    *lock = INIT;
}
static inline void omp_set_lock(omp_lock_t *lock) {
    if (*lock == UNLOCKED) {
        *lock = LOCKED;
    } else if (*lock == LOCKED) {
        fprintf(stderr, "error: deadlock in using lock variable\n");
        exit(1);
    } else {
        fprintf(stderr, "error: lock not initialized\n");
        exit(1);
    }
}
static inline void omp_unset_lock(omp_lock_t *lock) {
    if (*lock == LOCKED) {
        *lock = UNLOCKED;
    } else if (*lock == UNLOCKED) {
        fprintf(stderr, "error: lock not set\n");
        exit(1);
    } else {
        fprintf(stderr, "error: lock not initialized\n");
        exit(1);
    }
}
static inline int omp_test_lock(omp_lock_t *lock)
{
    if (*lock == UNLOCKED) {
        *lock = LOCKED;
        return 1;
    } else if (*lock == LOCKED) {
        return 0;
    } else {
        fprintf(stderr, "error: lock not initialized\n");
        exit(1);
    }
}
#ifndef OMP_NEST_LOCK_T
struct omp_nest_lock_t { /* This really belongs in omp.h */
    int owner;
    int count;
};
#endif
enum {NOOWNER=-1, MASTER = 0};
static inline void omp_init_nest_lock(omp_nest_lock_t *nlock) {
    nlock->owner = NOOWNER;
    nlock->count = 0;
}
static inline void omp_destroy_nest_lock(omp_nest_lock_t *nlock) {
    nlock->owner = NOOWNER;
    nlock->count = UNLOCKED;
}
static inline void omp_set_nest_lock(omp_nest_lock_t *nlock) {
    if (nlock->owner == MASTER && nlock->count >= 1) {
        nlock->count++;
    } else if (nlock->owner == NOOWNER && nlock->count == 0) {
        nlock->owner = MASTER;
        nlock->count = 1;
    } else {
        fprintf(stderr, "error: lock corrupted or not initialized\n");
        exit(1);
    }
}
static inline void omp_unset_nest_lock(omp_nest_lock_t *nlock) {
    if (nlock->owner == NOOWNER && nlock->count >= 1) {
        nlock->count--;
        if (nlock->count == 0) {
            nlock->owner = NOOWNER;
        }
    } else if (nlock->owner == NOOWNER && nlock->count == 0) {
        fprintf(stderr, "error: lock not set\n");
        exit(1);
    } else {
        fprintf(stderr, "error: lock corrupted or not initialized\n");
        exit(1);
    }
}
static inline int omp_test_nest_lock(omp_nest_lock_t *nlock) {
    omp_set_nest_lock(nlock);
    return nlock->count;
}

static inline double omp_get_wtime(void) {
    /* This function does not provide a working wallclock timer. Replace it with a version
       customized for the target machine. */
    return 0.0;
}
static inline double omp_get_wtick(void)
{
    /* This function does not provide a working clock tick function. Replace it with a version
       customized for the target machine. */
    return 365. * 86400.;
}
#endif //defined(_OPENMP)

#endif //OBVI_COMPAT_OMP_HPP
