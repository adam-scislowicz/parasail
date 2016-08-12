/**
 * @file
 *
 * @author jeff.daily@pnnl.gov
 *
 * Copyright (c) 2015 Battelle Memorial Institute.
 */
#include "config.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <emmintrin.h>
#endif

#include "parasail.h"
#include "parasail/memory.h"
#include "parasail/internal_sse.h"


static inline __m128i _mm_blendv_epi8_rpl(__m128i a, __m128i b, __m128i mask) {
    a = _mm_andnot_si128(mask, a);
    a = _mm_or_si128(a, _mm_and_si128(mask, b));
    return a;
}

static inline int16_t _mm_hmax_epi16_rpl(__m128i a) {
    a = _mm_max_epi16(a, _mm_srli_si128(a, 8));
    a = _mm_max_epi16(a, _mm_srli_si128(a, 4));
    a = _mm_max_epi16(a, _mm_srli_si128(a, 2));
    return _mm_extract_epi16(a, 0);
}


#ifdef PARASAIL_TABLE
static inline void arr_store_si128(
        int *array,
        __m128i vH,
        int32_t t,
        int32_t seglen,
        int32_t d,
        int32_t dlen)
{
    array[(0*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 0);
    array[(1*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 1);
    array[(2*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 2);
    array[(3*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 3);
    array[(4*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 4);
    array[(5*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 5);
    array[(6*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 6);
    array[(7*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 7);
}
#endif

#ifdef PARASAIL_ROWCOL
static inline void arr_store_col(
        int *col,
        __m128i vH,
        int32_t t,
        int32_t seglen)
{
    col[0*seglen+t] = (int16_t)_mm_extract_epi16(vH, 0);
    col[1*seglen+t] = (int16_t)_mm_extract_epi16(vH, 1);
    col[2*seglen+t] = (int16_t)_mm_extract_epi16(vH, 2);
    col[3*seglen+t] = (int16_t)_mm_extract_epi16(vH, 3);
    col[4*seglen+t] = (int16_t)_mm_extract_epi16(vH, 4);
    col[5*seglen+t] = (int16_t)_mm_extract_epi16(vH, 5);
    col[6*seglen+t] = (int16_t)_mm_extract_epi16(vH, 6);
    col[7*seglen+t] = (int16_t)_mm_extract_epi16(vH, 7);
}
#endif

#ifdef PARASAIL_TABLE
#define FNAME parasail_sw_stats_table_scan_sse2_128_16
#define PNAME parasail_sw_stats_table_scan_profile_sse2_128_16
#else
#ifdef PARASAIL_ROWCOL
#define FNAME parasail_sw_stats_rowcol_scan_sse2_128_16
#define PNAME parasail_sw_stats_rowcol_scan_profile_sse2_128_16
#else
#define FNAME parasail_sw_stats_scan_sse2_128_16
#define PNAME parasail_sw_stats_scan_profile_sse2_128_16
#endif
#endif

parasail_result_t* FNAME(
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap, const parasail_matrix_t *matrix)
{
    parasail_profile_t *profile = parasail_profile_create_stats_sse_128_16(s1, s1Len, matrix);
    parasail_result_t *result = PNAME(profile, s2, s2Len, open, gap);
    parasail_profile_free(profile);
    return result;
}

parasail_result_t* PNAME(
        const parasail_profile_t * const restrict profile,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap)
{
    int32_t i = 0;
    int32_t j = 0;
    int32_t end_query = 0;
    int32_t end_ref = 0;
    const int s1Len = profile->s1Len;
    const parasail_matrix_t *matrix = profile->matrix;
    const int32_t segWidth = 8; /* number of values in vector unit */
    const int32_t segLen = (s1Len + segWidth - 1) / segWidth;
    __m128i* const restrict pvP  = (__m128i*)profile->profile16.score;
    __m128i* const restrict pvPm = (__m128i*)profile->profile16.matches;
    __m128i* const restrict pvPs = (__m128i*)profile->profile16.similar;
    __m128i* const restrict pvE  = parasail_memalign___m128i(16, segLen);
    __m128i* const restrict pvEM = parasail_memalign___m128i(16, segLen);
    __m128i* const restrict pvES = parasail_memalign___m128i(16, segLen);
    __m128i* const restrict pvEL = parasail_memalign___m128i(16, segLen);
    __m128i* const restrict pvH  = parasail_memalign___m128i(16, segLen);
    __m128i* const restrict pvHM = parasail_memalign___m128i(16, segLen);
    __m128i* const restrict pvHS = parasail_memalign___m128i(16, segLen);
    __m128i* const restrict pvHL = parasail_memalign___m128i(16, segLen);
    __m128i* const restrict pvHMax  = parasail_memalign___m128i(16, segLen);
    __m128i* const restrict pvHMMax = parasail_memalign___m128i(16, segLen);
    __m128i* const restrict pvHSMax = parasail_memalign___m128i(16, segLen);
    __m128i* const restrict pvHLMax = parasail_memalign___m128i(16, segLen);
    __m128i* const restrict pvGapper = parasail_memalign___m128i(16, segLen);
    __m128i* const restrict pvGapperL = parasail_memalign___m128i(16, segLen);
    __m128i vGapO = _mm_set1_epi16(open);
    __m128i vGapE = _mm_set1_epi16(gap);
    const int16_t NEG_LIMIT = (-open < matrix->min ?
        INT16_MIN + open : INT16_MIN - matrix->min) + 1;
    const int16_t POS_LIMIT = INT16_MAX - matrix->max - 1;
    __m128i vZero = _mm_setzero_si128();
    __m128i vOne = _mm_set1_epi16(1);
    int16_t score = NEG_LIMIT;
    int16_t matches = 0;
    int16_t similar = 0;
    int16_t length = 0;
    __m128i vNegLimit = _mm_set1_epi16(NEG_LIMIT);
    __m128i vPosLimit = _mm_set1_epi16(POS_LIMIT);
    __m128i vSaturationCheckMin = vPosLimit;
    __m128i vSaturationCheckMax = vNegLimit;
    __m128i vMaxH = vNegLimit;
    __m128i vMaxM = vNegLimit;
    __m128i vMaxS = vNegLimit;
    __m128i vMaxL = vNegLimit;
    __m128i vMaxHUnit = vNegLimit;
    __m128i vNegInfFront = _mm_set_epi16(0,0,0,0,0,0,0,NEG_LIMIT);
    __m128i vSegLenXgap = _mm_add_epi16(vNegInfFront,
            _mm_slli_si128(_mm_set1_epi16(-segLen*gap), 2));
    __m128i vSegLen = _mm_slli_si128(_mm_set1_epi16(segLen), 2);
#ifdef PARASAIL_TABLE
    parasail_result_t *result = parasail_result_new_table3(segLen*segWidth, s2Len);
#else
#ifdef PARASAIL_ROWCOL
    parasail_result_t *result = parasail_result_new_rowcol3(segLen*segWidth, s2Len);
    const int32_t offset = (s1Len - 1) % segLen;
    const int32_t position = (segWidth - 1) - (s1Len - 1) / segLen;
#else
    parasail_result_t *result = parasail_result_new();
#endif
#endif

    parasail_memset___m128i(pvH, vZero, segLen);
    parasail_memset___m128i(pvHM, vZero, segLen);
    parasail_memset___m128i(pvHS, vZero, segLen);
    parasail_memset___m128i(pvHL, vZero, segLen);
    parasail_memset___m128i(pvE, vNegLimit, segLen);
    parasail_memset___m128i(pvEM, vZero, segLen);
    parasail_memset___m128i(pvES, vZero, segLen);
    parasail_memset___m128i(pvEL, vZero, segLen);
    {
        __m128i vGapper = _mm_sub_epi16(vZero,vGapO);
        __m128i vGapperL = vOne;
        for (i=segLen-1; i>=0; --i) {
            _mm_store_si128(pvGapper+i, vGapper);
            _mm_store_si128(pvGapperL+i, vGapperL);
            vGapper = _mm_sub_epi16(vGapper, vGapE);
            vGapperL = _mm_add_epi16(vGapperL, vOne);
        }
    }

    /* outer loop over database sequence */
    for (j=0; j<s2Len; ++j) {
        __m128i vE;
        __m128i vE_ext;
        __m128i vE_opn;
        __m128i vEM;
        __m128i vES;
        __m128i vEL;
        __m128i vHt;
        __m128i vHtM;
        __m128i vHtS;
        __m128i vHtL;
        __m128i vF;
        __m128i vF_ext;
        __m128i vF_opn;
        __m128i vFM;
        __m128i vFS;
        __m128i vFL;
        __m128i vH;
        __m128i vHM;
        __m128i vHS;
        __m128i vHL;
        __m128i vHp;
        __m128i vHpM;
        __m128i vHpS;
        __m128i vHpL;
        __m128i *pvW;
        __m128i vW;
        __m128i *pvWM;
        __m128i vWM;
        __m128i *pvWS;
        __m128i vWS;
        __m128i case1;
        __m128i case2;
        __m128i case0;
        __m128i vGapper;
        __m128i vGapperL;

        /* calculate E */
        /* calculate Ht */
        /* calculate F and H first pass */
        vHp = _mm_load_si128(pvH+(segLen-1));
        vHpM = _mm_load_si128(pvHM+(segLen-1));
        vHpS = _mm_load_si128(pvHS+(segLen-1));
        vHpL = _mm_load_si128(pvHL+(segLen-1));
        vHp = _mm_slli_si128(vHp, 2);
        vHpM = _mm_slli_si128(vHpM, 2);
        vHpS = _mm_slli_si128(vHpS, 2);
        vHpL = _mm_slli_si128(vHpL, 2);
        pvW = pvP + matrix->mapper[(unsigned char)s2[j]]*segLen;
        pvWM= pvPm+ matrix->mapper[(unsigned char)s2[j]]*segLen;
        pvWS= pvPs+ matrix->mapper[(unsigned char)s2[j]]*segLen;
        vHt = vZero;
        vF = vNegLimit;
        vFM = vZero;
        vFS = vZero;
        vFL = vZero;
        for (i=0; i<segLen; ++i) {
            vH = _mm_load_si128(pvH+i);
            vHM= _mm_load_si128(pvHM+i);
            vHS= _mm_load_si128(pvHS+i);
            vHL= _mm_load_si128(pvHL+i);
            vE = _mm_load_si128(pvE+i);
            vEM= _mm_load_si128(pvEM+i);
            vES= _mm_load_si128(pvES+i);
            vEL= _mm_load_si128(pvEL+i);
            vW = _mm_load_si128(pvW+i);
            vWM = _mm_load_si128(pvWM+i);
            vWS = _mm_load_si128(pvWS+i);
            vGapper = _mm_load_si128(pvGapper+i);
            vGapperL = _mm_load_si128(pvGapperL+i);
            vE_opn = _mm_sub_epi16(vH, vGapO);
            vE_ext = _mm_sub_epi16(vE, vGapE);
            case1 = _mm_cmpgt_epi16(vE_opn, vE_ext);
            vE = _mm_max_epi16(vE_opn, vE_ext);
            vEM = _mm_blendv_epi8_rpl(vEM, vHM, case1);
            vES = _mm_blendv_epi8_rpl(vES, vHS, case1);
            vEL = _mm_blendv_epi8_rpl(vEL, vHL, case1);
            vEL = _mm_add_epi16(vEL, vOne);
            vGapper = _mm_add_epi16(vHt, vGapper);
            case1 = _mm_or_si128(
                    _mm_cmpgt_epi16(vF, vGapper),
                    _mm_cmpeq_epi16(vF, vGapper));
            vF = _mm_max_epi16(vF, vGapper);
            vFM = _mm_blendv_epi8_rpl(vHtM, vFM, case1);
            vFS = _mm_blendv_epi8_rpl(vHtS, vFS, case1);
            vFL = _mm_blendv_epi8_rpl(
                    _mm_add_epi16(vHtL, vGapperL),
                    vFL, case1);
            vHp = _mm_add_epi16(vHp, vW);
            vHpM = _mm_add_epi16(vHpM, vWM);
            vHpS = _mm_add_epi16(vHpS, vWS);
            vHpL = _mm_add_epi16(vHpL, vOne);
            case1 = _mm_cmpgt_epi16(vE, vHp);
            vHt = _mm_max_epi16(vE, vHp);
            vHtM = _mm_blendv_epi8_rpl(vHpM, vEM, case1);
            vHtS = _mm_blendv_epi8_rpl(vHpS, vES, case1);
            vHtL = _mm_blendv_epi8_rpl(vHpL, vEL, case1);
            _mm_store_si128(pvE+i, vE);
            _mm_store_si128(pvEM+i, vEM);
            _mm_store_si128(pvES+i, vES);
            _mm_store_si128(pvEL+i, vEL);
            _mm_store_si128(pvH+i, vHp);
            _mm_store_si128(pvHM+i, vHpM);
            _mm_store_si128(pvHS+i, vHpS);
            _mm_store_si128(pvHL+i, vHpL);
            vHp = vH;
            vHpM = vHM;
            vHpS = vHS;
            vHpL = vHL;
        }

        /* pseudo prefix scan on F and H */
        vHt = _mm_slli_si128(vHt, 2);
        vHtM = _mm_slli_si128(vHtM, 2);
        vHtS = _mm_slli_si128(vHtS, 2);
        vHtL = _mm_slli_si128(vHtL, 2);
        vGapper = _mm_load_si128(pvGapper);
        vGapperL = _mm_load_si128(pvGapperL);
        vGapper = _mm_add_epi16(vHt, vGapper);
        case1 = _mm_or_si128(
                _mm_cmpgt_epi16(vGapper, vF),
                _mm_cmpeq_epi16(vGapper, vF));
        vF = _mm_max_epi16(vF, vGapper);
        vFM = _mm_blendv_epi8_rpl(vFM, vHtM, case1);
        vFS = _mm_blendv_epi8_rpl(vFS, vHtS, case1);
        vFL = _mm_blendv_epi8_rpl(
                vFL,
                _mm_add_epi16(vHtL, vGapperL),
                case1);
        for (i=0; i<segWidth-2; ++i) {
            __m128i vFt = _mm_slli_si128(vF, 2);
            __m128i vFtM = _mm_slli_si128(vFM, 2);
            __m128i vFtS = _mm_slli_si128(vFS, 2);
            __m128i vFtL = _mm_slli_si128(vFL, 2);
            vFt = _mm_add_epi16(vFt, vSegLenXgap);
            case1 = _mm_or_si128(
                    _mm_cmpgt_epi16(vFt, vF),
                    _mm_cmpeq_epi16(vFt, vF));
            vF = _mm_max_epi16(vF, vFt);
            vFM = _mm_blendv_epi8_rpl(vFM, vFtM, case1);
            vFS = _mm_blendv_epi8_rpl(vFS, vFtS, case1);
            vFL = _mm_blendv_epi8_rpl(
                    vFL,
                    _mm_add_epi16(vFtL, vSegLen),
                    case1);
        }

        /* calculate final H */
        vF = _mm_slli_si128(vF, 2);
        vFM = _mm_slli_si128(vFM, 2);
        vFS = _mm_slli_si128(vFS, 2);
        vFL = _mm_slli_si128(vFL, 2);
        vF = _mm_add_epi16(vF, vNegInfFront);
        case1 = _mm_cmpgt_epi16(vF, vHt);
        vH = _mm_max_epi16(vF, vHt);
        vHM = _mm_blendv_epi8_rpl(vHtM, vFM, case1);
        vHS = _mm_blendv_epi8_rpl(vHtS, vFS, case1);
        vHL = _mm_blendv_epi8_rpl(vHtL, vFL, case1);
        for (i=0; i<segLen; ++i) {
            vHp = _mm_load_si128(pvH+i);
            vHpM = _mm_load_si128(pvHM+i);
            vHpS = _mm_load_si128(pvHS+i);
            vHpL = _mm_load_si128(pvHL+i);
            vE = _mm_load_si128(pvE+i);
            vEM = _mm_load_si128(pvEM+i);
            vES = _mm_load_si128(pvES+i);
            vEL = _mm_load_si128(pvEL+i);
            vF_opn = _mm_sub_epi16(vH, vGapO);
            vF_ext = _mm_sub_epi16(vF, vGapE);
            vF = _mm_max_epi16(vF_opn, vF_ext);
            case1 = _mm_cmpgt_epi16(vF_opn, vF_ext);
            vFM = _mm_blendv_epi8_rpl(vFM, vHM, case1);
            vFS = _mm_blendv_epi8_rpl(vFS, vHS, case1);
            vFL = _mm_blendv_epi8_rpl(vFL, vHL, case1);
            vFL = _mm_add_epi16(vFL, vOne);
            vH = _mm_max_epi16(vHp, vE);
            vH = _mm_max_epi16(vH, vF);
            vH = _mm_max_epi16(vH, vZero);
            case1 = _mm_cmpeq_epi16(vH, vHp);
            case2 = _mm_cmpeq_epi16(vH, vF);
            case0 = _mm_cmpeq_epi16(vH, vZero);
            vHM = _mm_blendv_epi8_rpl(
                    _mm_blendv_epi8_rpl(vEM, vFM, case2),
                    vHpM, case1);
            vHM = _mm_blendv_epi8_rpl(vHM, vZero, case0);
            vHS = _mm_blendv_epi8_rpl(
                    _mm_blendv_epi8_rpl(vES, vFS, case2),
                    vHpS, case1);
            vHS = _mm_blendv_epi8_rpl(vHS, vZero, case0);
            vHL = _mm_blendv_epi8_rpl(
                    _mm_blendv_epi8_rpl(vEL, vFL, case2),
                    vHpL, case1);
            vHL = _mm_blendv_epi8_rpl(vHL, vZero, case0);
            _mm_store_si128(pvH+i, vH);
            _mm_store_si128(pvHM+i, vHM);
            _mm_store_si128(pvHS+i, vHS);
            _mm_store_si128(pvHL+i, vHL);
            vSaturationCheckMin = _mm_min_epi16(vSaturationCheckMin, vH);
            vSaturationCheckMax = _mm_max_epi16(vSaturationCheckMax, vH);
            vSaturationCheckMax = _mm_max_epi16(vSaturationCheckMax, vHM);
            vSaturationCheckMax = _mm_max_epi16(vSaturationCheckMax, vHS);
            vSaturationCheckMax = _mm_max_epi16(vSaturationCheckMax, vHL);
#ifdef PARASAIL_TABLE
            arr_store_si128(result->score_table, vH, i, segLen, j, s2Len);
            arr_store_si128(result->matches_table, vHM, i, segLen, j, s2Len);
            arr_store_si128(result->similar_table, vHS, i, segLen, j, s2Len);
            arr_store_si128(result->length_table, vHL, i, segLen, j, s2Len);
#endif
            {
                __m128i cond_max = _mm_cmpgt_epi16(vH, vMaxH);
                vMaxH = _mm_max_epi16(vH, vMaxH);
                vMaxM = _mm_blendv_epi8_rpl(vMaxM, vHM, cond_max);
                vMaxS = _mm_blendv_epi8_rpl(vMaxS, vHS, cond_max);
                vMaxL = _mm_blendv_epi8_rpl(vMaxL, vHL, cond_max);
            }
        } 

        {
            __m128i vCompare = _mm_cmpgt_epi16(vMaxH, vMaxHUnit);
            if (_mm_movemask_epi8(vCompare)) {
                score = _mm_hmax_epi16_rpl(vMaxH);
                vMaxHUnit = _mm_set1_epi16(score);
                end_ref = j;
                (void)memcpy(pvHMax, pvH, sizeof(__m128i)*segLen);
                (void)memcpy(pvHMMax, pvHM, sizeof(__m128i)*segLen);
                (void)memcpy(pvHSMax, pvHS, sizeof(__m128i)*segLen);
                (void)memcpy(pvHLMax, pvHL, sizeof(__m128i)*segLen);
            }
        }

#ifdef PARASAIL_ROWCOL
        /* extract last value from the column */
        {
            int32_t k = 0;
            __m128i vH = _mm_load_si128(pvH + offset);
            __m128i vHM = _mm_load_si128(pvHM + offset);
            __m128i vHS = _mm_load_si128(pvHS + offset);
            __m128i vHL = _mm_load_si128(pvHL + offset);
            for (k=0; k<position; ++k) {
                vH = _mm_slli_si128(vH, 2);
                vHM = _mm_slli_si128(vHM, 2);
                vHS = _mm_slli_si128(vHS, 2);
                vHL = _mm_slli_si128(vHL, 2);
            }
            result->score_row[j] = (int16_t) _mm_extract_epi16 (vH, 7);
            result->matches_row[j] = (int16_t) _mm_extract_epi16 (vHM, 7);
            result->similar_row[j] = (int16_t) _mm_extract_epi16 (vHS, 7);
            result->length_row[j] = (int16_t) _mm_extract_epi16 (vHL, 7);
        }
#endif
    }

    /* Trace the alignment ending position on read. */
    {
        int16_t *t = (int16_t*)pvHMax;
        int16_t *m = (int16_t*)pvHMMax;
        int16_t *s = (int16_t*)pvHSMax;
        int16_t *l = (int16_t*)pvHLMax;
        int32_t column_len = segLen * segWidth;
        end_query = s1Len;
        for (i = 0; i<column_len; ++i, ++t, ++m, ++s, ++l) {
            if (*t == score) {
                int32_t temp = i / segWidth + i % segWidth * segLen;
                if (temp < end_query) {
                    end_query = temp;
                    matches = *m;
                    similar = *s;
                    length = *l;
                }
            }
        }
    }

#ifdef PARASAIL_ROWCOL
    for (i=0; i<segLen; ++i) {
        __m128i vH = _mm_load_si128(pvH+i);
        __m128i vHM = _mm_load_si128(pvHM+i);
        __m128i vHS = _mm_load_si128(pvHS+i);
        __m128i vHL = _mm_load_si128(pvHL+i);
        arr_store_col(result->score_col, vH, i, segLen);
        arr_store_col(result->matches_col, vHM, i, segLen);
        arr_store_col(result->similar_col, vHS, i, segLen);
        arr_store_col(result->length_col, vHL, i, segLen);
    }
#endif

    if (_mm_movemask_epi8(_mm_or_si128(
            _mm_cmplt_epi16(vSaturationCheckMin, vNegLimit),
            _mm_cmpgt_epi16(vSaturationCheckMax, vPosLimit)))) {
        result->saturated = 1;
        score = 0;
        matches = 0;
        similar = 0;
        length = 0;
        end_query = 0;
        end_ref = 0;
    }

    result->score = score;
    result->matches = matches;
    result->similar = similar;
    result->length = length;
    result->end_query = end_query;
    result->end_ref = end_ref;

    parasail_free(pvGapperL);
    parasail_free(pvGapper);
    parasail_free(pvHLMax);
    parasail_free(pvHSMax);
    parasail_free(pvHMMax);
    parasail_free(pvHMax);
    parasail_free(pvHL);
    parasail_free(pvHS);
    parasail_free(pvHM);
    parasail_free(pvH);
    parasail_free(pvEL);
    parasail_free(pvES);
    parasail_free(pvEM);
    parasail_free(pvE);

    return result;
}


