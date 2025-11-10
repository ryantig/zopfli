/*
SIMD optimizations for Zopfli
Supports ARM NEON (Apple Silicon) and x86 SSE2
*/

#ifndef ZOPFLI_SIMD_H_
#define ZOPFLI_SIMD_H_

/* Feature detection */
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
  #define ZOPFLI_HAS_NEON 1
  #include <arm_neon.h>
#elif defined(__SSE2__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || defined(_M_X64)
  #define ZOPFLI_HAS_SSE2 1
  #include <emmintrin.h>
#else
  #define ZOPFLI_HAS_SIMD 0
#endif

#if defined(ZOPFLI_HAS_NEON) || defined(ZOPFLI_HAS_SSE2)
  #define ZOPFLI_HAS_SIMD 1
#endif

/*
SIMD-optimized GetMatch for comparing byte sequences.
Returns the position where scan and match differ.
*/
#ifdef ZOPFLI_HAS_NEON

static inline const unsigned char* GetMatch_SIMD(
    const unsigned char* __restrict__ scan,
    const unsigned char* __restrict__ match,
    const unsigned char* end,
    const unsigned char* safe_end) {
  
  /* Use NEON for 16-byte comparisons */
  while (scan + 16 <= safe_end) {
    uint8x16_t v_scan = vld1q_u8(scan);
    uint8x16_t v_match = vld1q_u8(match);
    uint8x16_t v_cmp = vceqq_u8(v_scan, v_match);
    
    /* Check if all bytes are equal */
    uint64x2_t v_cmp64 = vreinterpretq_u64_u8(v_cmp);
    uint64_t low = vgetq_lane_u64(v_cmp64, 0);
    uint64_t high = vgetq_lane_u64(v_cmp64, 1);
    
    if ((low & high) != 0xFFFFFFFFFFFFFFFFULL) {
      /* Found a difference, find exact position */
      for (int i = 0; i < 16; i++) {
        if (scan[i] != match[i]) {
          return scan + i;
        }
      }
    }
    
    scan += 16;
    match += 16;
  }
  
  /* Handle remaining bytes with 8-byte comparisons */
  while (scan + 8 <= safe_end) {
    if (*((const uint64_t*)scan) != *((const uint64_t*)match)) {
      break;
    }
    scan += 8;
    match += 8;
  }
  
  /* Final byte-by-byte comparison */
  while (scan < end && *scan == *match) {
    scan++;
    match++;
  }
  
  return scan;
}

#elif defined(ZOPFLI_HAS_SSE2)

static inline const unsigned char* GetMatch_SIMD(
    const unsigned char* __restrict__ scan,
    const unsigned char* __restrict__ match,
    const unsigned char* end,
    const unsigned char* safe_end) {
  
  /* Use SSE2 for 16-byte comparisons */
  while (scan + 16 <= safe_end) {
    __m128i v_scan = _mm_loadu_si128((const __m128i*)scan);
    __m128i v_match = _mm_loadu_si128((const __m128i*)match);
    __m128i v_cmp = _mm_cmpeq_epi8(v_scan, v_match);
    int mask = _mm_movemask_epi8(v_cmp);
    
    if (mask != 0xFFFF) {
      /* Found a difference, find exact position */
      int pos = __builtin_ctz(~mask);
      return scan + pos;
    }
    
    scan += 16;
    match += 16;
  }
  
  /* Handle remaining bytes with 8-byte comparisons */
  while (scan + 8 <= safe_end) {
    if (*((const uint64_t*)scan) != *((const uint64_t*)match)) {
      break;
    }
    scan += 8;
    match += 8;
  }
  
  /* Final byte-by-byte comparison */
  while (scan < end && *scan == *match) {
    scan++;
    match++;
  }
  
  return scan;
}

#else

/* No SIMD available, use scalar version */
static inline const unsigned char* GetMatch_SIMD(
    const unsigned char* __restrict__ scan,
    const unsigned char* __restrict__ match,
    const unsigned char* end,
    const unsigned char* safe_end) {
  
  /* 8-byte comparisons */
  while (scan < safe_end && *((const uint64_t*)scan) == *((const uint64_t*)match)) {
    scan += 8;
    match += 8;
  }
  
  /* Final byte-by-byte comparison */
  while (scan < end && *scan == *match) {
    scan++;
    match++;
  }
  
  return scan;
}

#endif /* SIMD variants */

#endif /* ZOPFLI_SIMD_H_ */

