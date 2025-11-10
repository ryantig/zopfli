/*
Simple profiling macros for Zopfli
Enable with -DPROFILE_ZOPFLI
*/

#ifndef ZOPFLI_PROFILE_H_
#define ZOPFLI_PROFILE_H_

#ifdef PROFILE_ZOPFLI

#include <sys/time.h>
#include <stdio.h>

/* Global counters - defined in lz77.c */
extern unsigned long long profile_getmatch_calls;
extern unsigned long long profile_getmatch_time_us;
extern unsigned long long profile_findlongest_calls;
extern unsigned long long profile_findlongest_time_us;
extern unsigned long long profile_hash_calls;
extern unsigned long long profile_hash_time_us;

static inline unsigned long long profile_get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ULL + tv.tv_usec;
}

#define PROFILE_START(name) \
    unsigned long long profile_start_##name = profile_get_time_us();

#define PROFILE_END(name) \
    do { \
        unsigned long long profile_end_##name = profile_get_time_us(); \
        profile_##name##_time_us += (profile_end_##name - profile_start_##name); \
        profile_##name##_calls++; \
    } while(0)

#define PROFILE_PRINT() \
    do { \
        fprintf(stderr, "\n====================================\n"); \
        fprintf(stderr, "Zopfli Profiling Results\n"); \
        fprintf(stderr, "====================================\n"); \
        fprintf(stderr, "GetMatch:      %10llu calls, %10llu us total, %6.2f us/call\n", \
                profile_getmatch_calls, profile_getmatch_time_us, \
                profile_getmatch_calls ? (double)profile_getmatch_time_us / profile_getmatch_calls : 0.0); \
        fprintf(stderr, "FindLongest:   %10llu calls, %10llu us total, %6.2f us/call\n", \
                profile_findlongest_calls, profile_findlongest_time_us, \
                profile_findlongest_calls ? (double)profile_findlongest_time_us / profile_findlongest_calls : 0.0); \
        fprintf(stderr, "Hash:          %10llu calls, %10llu us total, %6.2f us/call\n", \
                profile_hash_calls, profile_hash_time_us, \
                profile_hash_calls ? (double)profile_hash_time_us / profile_hash_calls : 0.0); \
        fprintf(stderr, "====================================\n"); \
        unsigned long long total = profile_getmatch_time_us + profile_findlongest_time_us + profile_hash_time_us; \
        fprintf(stderr, "Total profiled: %llu us (%.3f seconds)\n", total, total / 1000000.0); \
        fprintf(stderr, "GetMatch:       %.1f%%\n", total ? 100.0 * profile_getmatch_time_us / total : 0.0); \
        fprintf(stderr, "FindLongest:    %.1f%%\n", total ? 100.0 * profile_findlongest_time_us / total : 0.0); \
        fprintf(stderr, "Hash:           %.1f%%\n", total ? 100.0 * profile_hash_time_us / total : 0.0); \
        fprintf(stderr, "====================================\n\n"); \
    } while(0)

#else

#define PROFILE_START(name)
#define PROFILE_END(name)
#define PROFILE_PRINT()

#endif /* PROFILE_ZOPFLI */

#endif /* ZOPFLI_PROFILE_H_ */

