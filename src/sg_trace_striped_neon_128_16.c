/**
 * @file
 *
 * @author jeffrey.daily@gmail.com
 *
 * Copyright (c) 2015 Battelle Memorial Institute.
 */
#include "config.h"

#include <stdint.h>
#include <stdlib.h>



#define SG_TRACE
#define SG_SUFFIX _striped_neon_128_16
#define SG_SUFFIX_PROF _striped_profile_neon_128_16
#include "sg_helper.h"

#include "parasail.h"
#include "parasail/memory.h"
#include "parasail/internal_neon.h"

#define SWAP(A,B) { simde__m128i* tmp = A; A = B; B = tmp; }

#define NEG_INF (INT16_MIN/(int16_t)(2))


static inline void arr_store(
        simde__m128i *array,
        simde__m128i vH,
        int32_t t,
        int32_t seglen,
        int32_t d)
{
    simde_mm_store_si128(array + (1LL*d*seglen+t), vH);
}

static inline simde__m128i arr_load(
        simde__m128i *array,
        int32_t t,
        int32_t seglen,
        int32_t d)
{
    return simde_mm_load_si128(array + (1LL*d*seglen+t));
}

#define FNAME parasail_sg_flags_trace_striped_neon_128_16
#define PNAME parasail_sg_flags_trace_striped_profile_neon_128_16

parasail_result_t* FNAME(
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap, const parasail_matrix_t *matrix,
        int s1_beg, int s1_end, int s2_beg, int s2_end)
{
    /* declare local variables */
    parasail_profile_t *profile = NULL;
    parasail_result_t *result = NULL;

    /* validate inputs */
    PARASAIL_CHECK_NULL(s1);
    PARASAIL_CHECK_GT0(s1Len);
    PARASAIL_CHECK_NULL(s2);
    PARASAIL_CHECK_GT0(s2Len);
    PARASAIL_CHECK_GE0(open);
    PARASAIL_CHECK_GE0(gap);
    PARASAIL_CHECK_NULL(matrix);

    /* initialize local variables */
    profile = parasail_profile_create_neon_128_16(s1, s1Len, matrix);
    if (!profile) return NULL;
    result = PNAME(profile, s2, s2Len, open, gap, s1_beg, s1_end, s2_beg, s2_end);

    parasail_profile_free(profile);

    return result;
}

parasail_result_t* PNAME(
        const parasail_profile_t * const restrict profile,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap,
        int s1_beg, int s1_end, int s2_beg, int s2_end)
{
    /* declare local variables */
    int32_t i = 0;
    int32_t j = 0;
    int32_t k = 0;
    int s1Len = 0;
    int32_t end_query = 0;
    int32_t end_ref = 0;
    const parasail_matrix_t *matrix = NULL;
    int32_t segWidth = 0;
    int32_t segLen = 0;
    int32_t offset = 0;
    int32_t position = 0;
    simde__m128i* restrict vProfile = NULL;
    simde__m128i* restrict pvHStore = NULL;
    simde__m128i* restrict pvHLoad = NULL;
    simde__m128i* restrict pvE = NULL;
    simde__m128i* restrict pvEaStore = NULL;
    simde__m128i* restrict pvEaLoad = NULL;
    simde__m128i* restrict pvHT = NULL;
    int16_t* restrict boundary = NULL;
    simde__m128i vGapO;
    simde__m128i vGapE;
    simde__m128i vNegInf;
    int16_t score = 0;
    
    simde__m128i vMaxH;
    simde__m128i vPosMask;
    parasail_result_t *result = NULL;
    simde__m128i vTIns;
    simde__m128i vTDel;
    simde__m128i vTDiag;
    simde__m128i vTDiagE;
    simde__m128i vTInsE;
    simde__m128i vTDiagF;
    simde__m128i vTDelF;
    simde__m128i vTMask;
    simde__m128i vFTMask;

    /* validate inputs */
    PARASAIL_CHECK_NULL(profile);
    PARASAIL_CHECK_NULL(profile->profile16.score);
    PARASAIL_CHECK_NULL(profile->matrix);
    PARASAIL_CHECK_GT0(profile->s1Len);
    PARASAIL_CHECK_NULL(s2);
    PARASAIL_CHECK_GT0(s2Len);
    PARASAIL_CHECK_GE0(open);
    PARASAIL_CHECK_GE0(gap);

    /* initialize stack variables */
    i = 0;
    j = 0;
    k = 0;
    s1Len = profile->s1Len;
    end_query = s1Len-1;
    end_ref = s2Len-1;
    matrix = profile->matrix;
    segWidth = 8; /* number of values in vector unit */
    segLen = (s1Len + segWidth - 1) / segWidth;
    offset = (s1Len - 1) % segLen;
    position = (segWidth - 1) - (s1Len - 1) / segLen;
    vProfile = (simde__m128i*)profile->profile16.score;
    vGapO = simde_mm_set1_epi16(open);
    vGapE = simde_mm_set1_epi16(gap);
    vNegInf = simde_mm_set1_epi16(NEG_INF);
    score = NEG_INF;
    vMaxH = vNegInf;
    
    vPosMask = simde_mm_cmpeq_epi16(simde_mm_set1_epi16(position),
            simde_mm_set_epi16(0,1,2,3,4,5,6,7));
    vTIns  = simde_mm_set1_epi16(PARASAIL_INS);
    vTDel  = simde_mm_set1_epi16(PARASAIL_DEL);
    vTDiag = simde_mm_set1_epi16(PARASAIL_DIAG);
    vTDiagE= simde_mm_set1_epi16(PARASAIL_DIAG_E);
    vTInsE = simde_mm_set1_epi16(PARASAIL_INS_E);
    vTDiagF= simde_mm_set1_epi16(PARASAIL_DIAG_F);
    vTDelF = simde_mm_set1_epi16(PARASAIL_DEL_F);
    vTMask = simde_mm_set1_epi16(PARASAIL_ZERO_MASK);
    vFTMask= simde_mm_set1_epi16(PARASAIL_F_MASK);

    /* initialize result */
    result = parasail_result_new_trace(segLen, s2Len, 16, sizeof(simde__m128i));
    if (!result) return NULL;

    /* set known flags */
    result->flag |= PARASAIL_FLAG_SG | PARASAIL_FLAG_STRIPED
        | PARASAIL_FLAG_TRACE
        | PARASAIL_FLAG_BITS_16 | PARASAIL_FLAG_LANES_8;
    result->flag |= s1_beg ? PARASAIL_FLAG_SG_S1_BEG : 0;
    result->flag |= s1_end ? PARASAIL_FLAG_SG_S1_END : 0;
    result->flag |= s2_beg ? PARASAIL_FLAG_SG_S2_BEG : 0;
    result->flag |= s2_end ? PARASAIL_FLAG_SG_S2_END : 0;

    /* initialize heap variables */
    pvHStore = parasail_memalign_simde__m128i(16, segLen);
    pvHLoad  = parasail_memalign_simde__m128i(16, segLen);
    pvE      = parasail_memalign_simde__m128i(16, segLen);
    pvEaStore= parasail_memalign_simde__m128i(16, segLen);
    pvEaLoad = parasail_memalign_simde__m128i(16, segLen);
    pvHT     = parasail_memalign_simde__m128i(16, segLen);
    boundary = parasail_memalign_int16_t(16, s2Len+1);

    /* validate heap variables */
    if (!pvHStore) return NULL;
    if (!pvHLoad) return NULL;
    if (!pvE) return NULL;
    if (!pvEaStore) return NULL;
    if (!pvEaLoad) return NULL;
    if (!pvHT) return NULL;
    if (!boundary) return NULL;

    /* initialize H and E */
    {
        int32_t index = 0;
        for (i=0; i<segLen; ++i) {
            int32_t segNum = 0;
            simde__m128i h;
            simde__m128i e;
            for (segNum=0; segNum<segWidth; ++segNum) {
                int64_t tmp = s1_beg ? 0 : (-open-gap*(segNum*segLen+i));
                h.i16[segNum] = tmp < INT16_MIN ? INT16_MIN : tmp;
                tmp = tmp - open;
                e.i16[segNum] = tmp < INT16_MIN ? INT16_MIN : tmp;
            }
            simde_mm_store_si128(&pvHStore[index], h);
            simde_mm_store_si128(&pvE[index], e);
            simde_mm_store_si128(&pvEaStore[index], e);
            ++index;
        }
    }

    /* initialize uppder boundary */
    {
        boundary[0] = 0;
        for (i=1; i<=s2Len; ++i) {
            int64_t tmp = s2_beg ? 0 : (-open-gap*(i-1));
            boundary[i] = tmp < INT16_MIN ? INT16_MIN : tmp;
        }
    }

    for (i=0; i<segLen; ++i) {
        arr_store(result->trace->trace_table, vTDiagE, i, segLen, 0);
    }

    /* outer loop over database sequence */
    for (j=0; j<s2Len; ++j) {
        simde__m128i vEF_opn;
        simde__m128i vE;
        simde__m128i vE_ext;
        simde__m128i vF;
        simde__m128i vF_ext;
        simde__m128i vFa;
        simde__m128i vFa_ext;
        simde__m128i vH;
        simde__m128i vH_dag;
        const simde__m128i* vP = NULL;

        /* Initialize F value to -inf.  Any errors to vH values will be
         * corrected in the Lazy_F loop. */
        vF = vNegInf;

        /* load final segment of pvHStore and shift left by 2 bytes */
        vH = simde_mm_load_si128(&pvHStore[segLen - 1]);
        vH = simde_mm_slli_si128(vH, 2);

        /* insert upper boundary condition */
        vH = simde_mm_insert_epi16(vH, boundary[j], 0);

        /* Correct part of the vProfile */
        vP = vProfile + matrix->mapper[(unsigned char)s2[j]] * segLen;

        /* Swap the 2 H buffers. */
        SWAP(pvHLoad, pvHStore)
        SWAP(pvEaLoad, pvEaStore)

        /* inner loop to process the query sequence */
        for (i=0; i<segLen; ++i) {
            vE = simde_mm_load_si128(pvE + i);

            /* Get max from vH, vE and vF. */
            vH_dag = simde_mm_add_epi16(vH, simde_mm_load_si128(vP + i));
            vH = simde_mm_max_epi16(vH_dag, vE);
            vH = simde_mm_max_epi16(vH, vF);
            /* Save vH values. */
            simde_mm_store_si128(pvHStore + i, vH);
            

            {
                simde__m128i vTAll = arr_load(result->trace->trace_table, i, segLen, j);
                simde__m128i case1 = simde_mm_cmpeq_epi16(vH, vH_dag);
                simde__m128i case2 = simde_mm_cmpeq_epi16(vH, vF);
                simde__m128i vT = simde_mm_blendv_epi8(
                        simde_mm_blendv_epi8(vTIns, vTDel, case2),
                        vTDiag, case1);
                simde_mm_store_si128(pvHT + i, vT);
                vT = simde_mm_or_si128(vT, vTAll);
                arr_store(result->trace->trace_table, vT, i, segLen, j);
            }

            vEF_opn = simde_mm_sub_epi16(vH, vGapO);

            /* Update vE value. */
            vE_ext = simde_mm_sub_epi16(vE, vGapE);
            vE = simde_mm_max_epi16(vEF_opn, vE_ext);
            simde_mm_store_si128(pvE + i, vE);
            {
                simde__m128i vEa = simde_mm_load_si128(pvEaLoad + i);
                simde__m128i vEa_ext = simde_mm_sub_epi16(vEa, vGapE);
                vEa = simde_mm_max_epi16(vEF_opn, vEa_ext);
                simde_mm_store_si128(pvEaStore + i, vEa);
                if (j+1<s2Len) {
                    simde__m128i cond = simde_mm_cmpgt_epi16(vEF_opn, vEa_ext);
                    simde__m128i vT = simde_mm_blendv_epi8(vTInsE, vTDiagE, cond);
                    arr_store(result->trace->trace_table, vT, i, segLen, j+1);
                }
            }

            /* Update vF value. */
            vF_ext = simde_mm_sub_epi16(vF, vGapE);
            vF = simde_mm_max_epi16(vEF_opn, vF_ext);
            if (i+1<segLen) {
                simde__m128i vTAll = arr_load(result->trace->trace_table, i+1, segLen, j);
                simde__m128i cond = simde_mm_cmpgt_epi16(vEF_opn, vF_ext);
                simde__m128i vT = simde_mm_blendv_epi8(vTDelF, vTDiagF, cond);
                vT = simde_mm_or_si128(vT, vTAll);
                arr_store(result->trace->trace_table, vT, i+1, segLen, j);
            }

            /* Load the next vH. */
            vH = simde_mm_load_si128(pvHLoad + i);
        }

        /* Lazy_F loop: has been revised to disallow adjecent insertion and
         * then deletion, so don't update E(i, i), learn from SWPS3 */
        vFa_ext = vF_ext;
        vFa = vF;
        for (k=0; k<segWidth; ++k) {
            int64_t tmp = s2_beg ? -open : (boundary[j+1]-open);
            int16_t tmp2 = tmp < INT16_MIN ? INT16_MIN : tmp;
            simde__m128i vHp = simde_mm_load_si128(&pvHLoad[segLen - 1]);
            vHp = simde_mm_slli_si128(vHp, 2);
            vHp = simde_mm_insert_epi16(vHp, boundary[j], 0);
            vEF_opn = simde_mm_slli_si128(vEF_opn, 2);
            vEF_opn = simde_mm_insert_epi16(vEF_opn, tmp2, 0);
            vF_ext = simde_mm_slli_si128(vF_ext, 2);
            vF_ext = simde_mm_insert_epi16(vF_ext, NEG_INF, 0);
            vF = simde_mm_slli_si128(vF, 2);
            vF = simde_mm_insert_epi16(vF, tmp2, 0);
            vFa_ext = simde_mm_slli_si128(vFa_ext, 2);
            vFa_ext = simde_mm_insert_epi16(vFa_ext, NEG_INF, 0);
            vFa = simde_mm_slli_si128(vFa, 2);
            vFa = simde_mm_insert_epi16(vFa, tmp2, 0);
            for (i=0; i<segLen; ++i) {
                vH = simde_mm_load_si128(pvHStore + i);
                vH = simde_mm_max_epi16(vH,vF);
                simde_mm_store_si128(pvHStore + i, vH);
                
                {
                    simde__m128i vTAll;
                    simde__m128i vT;
                    simde__m128i case1;
                    simde__m128i case2;
                    simde__m128i cond;
                    vHp = simde_mm_add_epi16(vHp, simde_mm_load_si128(vP + i));
                    case1 = simde_mm_cmpeq_epi16(vH, vHp);
                    case2 = simde_mm_cmpeq_epi16(vH, vF);
                    cond = simde_mm_andnot_si128(case1,case2);
                    vTAll = arr_load(result->trace->trace_table, i, segLen, j);
                    vT = simde_mm_load_si128(pvHT + i);
                    vT = simde_mm_blendv_epi8(vT, vTDel, cond);
                    simde_mm_store_si128(pvHT + i, vT);
                    vTAll = simde_mm_and_si128(vTAll, vTMask);
                    vTAll = simde_mm_or_si128(vTAll, vT);
                    arr_store(result->trace->trace_table, vTAll, i, segLen, j);
                }
                /* Update vF value. */
                {
                    simde__m128i vTAll = arr_load(result->trace->trace_table, i, segLen, j);
                    simde__m128i cond = simde_mm_cmpgt_epi16(vEF_opn, vFa_ext);
                    simde__m128i vT = simde_mm_blendv_epi8(vTDelF, vTDiagF, cond);
                    vTAll = simde_mm_and_si128(vTAll, vFTMask);
                    vTAll = simde_mm_or_si128(vTAll, vT);
                    arr_store(result->trace->trace_table, vTAll, i, segLen, j);
                }
                vEF_opn = simde_mm_sub_epi16(vH, vGapO);
                vF_ext = simde_mm_sub_epi16(vF, vGapE);
                {
                    simde__m128i vEa = simde_mm_load_si128(pvEaLoad + i);
                    simde__m128i vEa_ext = simde_mm_sub_epi16(vEa, vGapE);
                    vEa = simde_mm_max_epi16(vEF_opn, vEa_ext);
                    simde_mm_store_si128(pvEaStore + i, vEa);
                    if (j+1<s2Len) {
                        simde__m128i cond = simde_mm_cmpgt_epi16(vEF_opn, vEa_ext);
                        simde__m128i vT = simde_mm_blendv_epi8(vTInsE, vTDiagE, cond);
                        arr_store(result->trace->trace_table, vT, i, segLen, j+1);
                    }
                }
                if (! simde_mm_movemask_epi8(
                            simde_mm_or_si128(
                                simde_mm_cmpgt_epi16(vF_ext, vEF_opn),
                                simde_mm_cmpeq_epi16(vF_ext, vEF_opn))))
                    goto end;
                /*vF = simde_mm_max_epi16(vEF_opn, vF_ext);*/
                vF = vF_ext;
                vFa_ext = simde_mm_sub_epi16(vFa, vGapE);
                vFa = simde_mm_max_epi16(vEF_opn, vFa_ext);
                vHp = simde_mm_load_si128(pvHLoad + i);
            }
        }
end:
        {
            /* extract vector containing last value from the column */
            simde__m128i vCompare;
            vH = simde_mm_load_si128(pvHStore + offset);
            vCompare = simde_mm_and_si128(vPosMask, simde_mm_cmpgt_epi16(vH, vMaxH));
            vMaxH = simde_mm_max_epi16(vH, vMaxH);
            if (simde_mm_movemask_epi8(vCompare)) {
                end_ref = j;
            }
        }
    }

    /* max last value from all columns */
    if (s2_end)
    {
        for (k=0; k<position; ++k) {
            vMaxH = simde_mm_slli_si128(vMaxH, 2);
        }
        score = (int16_t) simde_mm_extract_epi16(vMaxH, 7);
        end_query = s1Len-1;
    }

    /* max of last column */
    if (s1_end)
    {
        /* Trace the alignment ending position on read. */
        int16_t *t = (int16_t*)pvHStore;
        int32_t column_len = segLen * segWidth;
        for (i = 0; i<column_len; ++i, ++t) {
            int32_t temp = i / segWidth + i % segWidth * segLen;
            if (temp >= s1Len) continue;
            if (*t > score) {
                score = *t;
                end_query = temp;
                end_ref = s2Len-1;
            }
            else if (*t == score && end_ref == s2Len-1 && temp < end_query) {
                end_query = temp;
            }
        }
    }

    if (!s1_end && !s2_end) {
        /* extract last value from the last column */
        {
            simde__m128i vH = simde_mm_load_si128(pvHStore + offset);
            for (k=0; k<position; ++k) {
                vH = simde_mm_slli_si128(vH, 2);
            }
            score = (int16_t) simde_mm_extract_epi16 (vH, 7);
            end_ref = s2Len - 1;
            end_query = s1Len - 1;
        }
    }

    

    result->score = score;
    result->end_query = end_query;
    result->end_ref = end_ref;

    parasail_free(boundary);
    parasail_free(pvHT);
    parasail_free(pvEaLoad);
    parasail_free(pvEaStore);
    parasail_free(pvE);
    parasail_free(pvHLoad);
    parasail_free(pvHStore);

    return result;
}

SG_IMPL_ALL
SG_IMPL_PROF_ALL


