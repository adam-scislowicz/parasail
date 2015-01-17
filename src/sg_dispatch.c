/**
 * @file
 *
 * @author jeff.daily@pnnl.gov
 *
 * Copyright (c) 2014 Battelle Memorial Institute.
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */
#include "config.h"

#include <stdint.h>
#include <stdlib.h>

#include "parasail.h"
#include "parasail_cpuid.h"
#include "parasail_internal.h"

/* typedef for the sg function */
typedef parasail_result_t* parasail_func(
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap, const int matrix[24][24]);

/* forward declare the dispatcher function */
static parasail_func parasail_sg_dispatcher;
static parasail_func parasail_sg_32_dispatcher;
static parasail_func parasail_sg_16_dispatcher;
static parasail_func parasail_sg_8_dispatcher;

/* forward declare the vector impls, if needed */
#if HAVE_AVX2
static parasail_func sg_scan_avx2;
#endif
#if HAVE_SSE41
static parasail_func sg_scan_sse41;
#endif
#if HAVE_SSE2
static parasail_func sg_scan_sse2;
#endif

/* declare and initialize the sg pointer to the dispatcher function */
static parasail_func * parasail_sg_pointer = &parasail_sg_dispatcher;
static parasail_func * parasail_sg_32_pointer = &parasail_sg_32_dispatcher;
static parasail_func * parasail_sg_16_pointer = &parasail_sg_16_dispatcher;
static parasail_func * parasail_sg_8_pointer = &parasail_sg_8_dispatcher;

/* dispatcher function implementation */
static parasail_result_t* parasail_sg_dispatcher(
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap, const int matrix[24][24])
{
#if HAVE_KNC
    parasail_sg_pointer = sg_scan_knc_512_32;
#else
#if HAVE_AVX2
    if (parasail_can_use_avx2()) {
        parasail_sg_pointer = sg_scan_avx2;
    }
    else
#endif
#if HAVE_SSE41
    if (parasail_can_use_sse41()) {
        parasail_sg_pointer = sg_scan_sse41;
    }
    else
#endif
#if HAVE_SSE2
    if (parasail_can_use_sse2()) {
        parasail_sg_pointer = sg_scan_sse2;
    }
    else
#endif
#endif
    {
    }
    return parasail_sg_pointer(s1, s1Len, s2, s2Len, open, gap, matrix);
}

/* dispatcher function implementation */
static parasail_result_t* parasail_sg_32_dispatcher(
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap, const int matrix[24][24])
{
#if HAVE_KNC
    parasail_sg_pointer = sg_scan_knc_512_32;
#else
#if HAVE_AVX2
    if (parasail_can_use_avx2()) {
        parasail_sg_pointer = sg_scan_avx2_256_32;
    }
    else
#endif
#if HAVE_SSE41
    if (parasail_can_use_sse41()) {
        parasail_sg_pointer = sg_scan_sse41_128_32;
    }
    else
#endif
#if HAVE_SSE2
    if (parasail_can_use_sse2()) {
        parasail_sg_pointer = sg_scan_sse2_128_32;
    }
    else
#endif
#endif
    {
    }
    return parasail_sg_pointer(s1, s1Len, s2, s2Len, open, gap, matrix);
}

/* dispatcher function implementation */
static parasail_result_t* parasail_sg_16_dispatcher(
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap, const int matrix[24][24])
{
#if HAVE_KNC
    parasail_sg_pointer = sg_scan_knc_512_32;
#else
#if HAVE_AVX2
    if (parasail_can_use_avx2()) {
        parasail_sg_pointer = sg_scan_avx2_256_16;
    }
    else
#endif
#if HAVE_SSE41
    if (parasail_can_use_sse41()) {
        parasail_sg_pointer = sg_scan_sse41_128_16;
    }
    else
#endif
#if HAVE_SSE2
    if (parasail_can_use_sse2()) {
        parasail_sg_pointer = sg_scan_sse2_128_16;
    }
    else
#endif
#endif
    {
    }
    return parasail_sg_pointer(s1, s1Len, s2, s2Len, open, gap, matrix);
}

/* dispatcher function implementation */
static parasail_result_t* parasail_sg_8_dispatcher(
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap, const int matrix[24][24])
{
#if HAVE_KNC
    parasail_sg_pointer = sg_scan_knc_512_32;
#else
#if HAVE_AVX2
    if (parasail_can_use_avx2()) {
        parasail_sg_pointer = sg_scan_avx2_256_8;
    }
    else
#endif
#if HAVE_SSE41
    if (parasail_can_use_sse41()) {
        parasail_sg_pointer = sg_scan_sse41_128_8;
    }
    else
#endif
#if HAVE_SSE2
    if (parasail_can_use_sse2()) {
        parasail_sg_pointer = sg_scan_sse2_128_8;
    }
    else
#endif
#endif
    {
    }
    return parasail_sg_pointer(s1, s1Len, s2, s2Len, open, gap, matrix);
}

#if HAVE_AVX2
/* sg vec impl which checks for saturation */
static parasail_result_t* sg_scan_avx2(
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap, const int matrix[24][24])
{
    parasail_result_t *result;

    result = sg_scan_avx2_256_8(s1, s1Len, s2, s2Len, open, gap, matrix);
    if (result->saturated) {
        parasail_result_free(result);
        result = sg_scan_avx2_256_16(s1, s1Len, s2, s2Len, open, gap, matrix);
    }
    if (result->saturated) {
        parasail_result_free(result);
        result = sg_scan_avx2_256_32(s1, s1Len, s2, s2Len, open, gap, matrix);
    }
    return result;
}
#endif

#if HAVE_SSE41
/* sg vec impl which checks for saturation */
static parasail_result_t* sg_scan_sse41(
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap, const int matrix[24][24])
{
    parasail_result_t *result;

    result = sg_scan_sse41_128_8(s1, s1Len, s2, s2Len, open, gap, matrix);
    if (result->saturated) {
        parasail_result_free(result);
        result = sg_scan_sse41_128_16(s1, s1Len, s2, s2Len, open, gap, matrix);
    }
    if (result->saturated) {
        parasail_result_free(result);
        result = sg_scan_sse41_128_32(s1, s1Len, s2, s2Len, open, gap, matrix);
    }
    return result;
}
#endif

#if HAVE_SSE2
/* sg vec impl which checks for saturation */
static parasail_result_t* sg_scan_sse2(
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap, const int matrix[24][24])
{
    parasail_result_t *result;

    result = sg_scan_sse2_128_8(s1, s1Len, s2, s2Len, open, gap, matrix);
    if (result->saturated) {
        parasail_result_free(result);
        result = sg_scan_sse2_128_16(s1, s1Len, s2, s2Len, open, gap, matrix);
    }
    if (result->saturated) {
        parasail_result_free(result);
        result = sg_scan_sse2_128_32(s1, s1Len, s2, s2Len, open, gap, matrix);
    }
    return result;
}
#endif

/* sg implementation which simply calls the pointer,
 * first time it's the dispatcher, otherwise it's correct sg impl */
parasail_result_t* parasail_sg(
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap, const int matrix[24][24])
{
    return parasail_sg_pointer(s1, s1Len, s2, s2Len, open, gap, matrix);
}

/* sg implementation which simply calls the pointer,
 * first time it's the dispatcher, otherwise it's correct sg impl */
parasail_result_t* parasail_sg_32(
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap, const int matrix[24][24])
{
    return parasail_sg_32_pointer(s1, s1Len, s2, s2Len, open, gap, matrix);
}

/* sg implementation which simply calls the pointer,
 * first time it's the dispatcher, otherwise it's correct sg impl */
parasail_result_t* parasail_sg_16(
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap, const int matrix[24][24])
{
    return parasail_sg_16_pointer(s1, s1Len, s2, s2Len, open, gap, matrix);
}

/* sg implementation which simply calls the pointer,
 * first time it's the dispatcher, otherwise it's correct sg impl */
parasail_result_t* parasail_sg_8(
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap, const int matrix[24][24])
{
    return parasail_sg_8_pointer(s1, s1Len, s2, s2Len, open, gap, matrix);
}

