/**
 * @file
 *
 * @author jeffrey.daily@gmail.com
 *
 * Copyright (c) 2015 Battelle Memorial Institute.
 */
#include "config.h"

#include <stdlib.h>

%(HEADER)s

#include "parasail.h"
#include "parasail/memory.h"
#include "parasail/internal_%(ISA)s.h"

#define NEG_INF %(NEG_INF)s
%(FIXES)s

#ifdef PARASAIL_TABLE
static inline void arr_store_si%(BITS)s(
        int *array,
        %(VTYPE)s vWH,
        %(INDEX)s i,
        %(INDEX)s s1Len,
        %(INDEX)s j,
        %(INDEX)s s2Len)
{
%(PRINTER)s
}
#endif

#ifdef PARASAIL_ROWCOL
static inline void arr_store_rowcol(
        int *row,
        int *col,
        %(VTYPE)s vWH,
        %(INDEX)s i,
        %(INDEX)s s1Len,
        %(INDEX)s j,
        %(INDEX)s s2Len)
{
%(PRINTER_ROWCOL)s
}
#endif

#ifdef PARASAIL_TABLE
#define FNAME %(NAME_TABLE)s
#else
#ifdef PARASAIL_ROWCOL
#define FNAME %(NAME_ROWCOL)s
#else
#define FNAME %(NAME)s
#endif
#endif

parasail_result_t* FNAME(
        const char * const restrict _s1, const int s1Len,
        const char * const restrict _s2, const int s2Len,
        const int open, const int gap, const parasail_matrix_t *matrix)
{
    /* declare local variables */
    %(INDEX)s N = 0;
    %(INDEX)s PAD = 0;
    %(INDEX)s PAD2 = 0;
    %(INDEX)s s1Len_PAD = 0;
    %(INDEX)s s2Len_PAD = 0;
    %(INT)s * restrict s1 = NULL;
    %(INT)s * restrict s2B = NULL;
    %(INT)s * restrict _H_pr = NULL;
    %(INT)s * restrict _F_pr = NULL;
    %(INT)s * restrict s2 = NULL;
    %(INT)s * restrict H_pr = NULL;
    %(INT)s * restrict F_pr = NULL;
    parasail_result_t *result = NULL;
    %(INDEX)s i = 0;
    %(INDEX)s j = 0;
    %(INDEX)s end_query = 0;
    %(INDEX)s end_ref = 0;
    %(INT)s score = 0;
    %(VTYPE)s vNegInf;
    %(VTYPE)s vNegInf0;
    %(VTYPE)s vOpen;
    %(VTYPE)s vGap;
    %(VTYPE)s vZero;
    %(VTYPE)s vOne;
    %(VTYPE)s vN;
    %(VTYPE)s vNegOne;
    %(VTYPE)s vI;
    %(VTYPE)s vJreset;
    %(VTYPE)s vMaxH;
    %(VTYPE)s vEndI;
    %(VTYPE)s vEndJ;
    %(VTYPE)s vILimit;
    %(VTYPE)s vJLimit;
    %(SATURATION_CHECK_DECL)s

    /* validate inputs */
    PARASAIL_CHECK_NULL(_s1);
    PARASAIL_CHECK_GT0(s1Len);
    PARASAIL_CHECK_NULL(_s2);
    PARASAIL_CHECK_GT0(s2Len);
    PARASAIL_CHECK_GE0(open);
    PARASAIL_CHECK_GE0(gap);
    PARASAIL_CHECK_NULL(matrix);
        
    /* initialize stack variables */
    N = %(LANES)s; /* number of values in vector */
    PAD = N-1;
    PAD2 = PAD*2;
    s1Len_PAD = s1Len+PAD;
    s2Len_PAD = s2Len+PAD;
    i = 0;
    j = 0;
    end_query = 0;
    end_ref = 0;
    score = NEG_INF;
    vNegInf = %(VSET1)s(NEG_INF);
    vNegInf0 = %(VRSHIFT)s(vNegInf, %(BYTES)s); /* shift in a 0 */
    vOpen = %(VSET1)s(open);
    vGap  = %(VSET1)s(gap);
    vZero = %(VSET1)s(0);
    vOne = %(VSET1)s(1);
    vN = %(VSET1)s(N);
    vNegOne = %(VSET1)s(-1);
    vI = %(VSET)s(%(DIAG_I)s);
    vJreset = %(VSET)s(%(DIAG_J)s);
    vMaxH = vNegInf;
    vEndI = vNegInf;
    vEndJ = vNegInf;
    vILimit = %(VSET1)s(s1Len);
    vJLimit = %(VSET1)s(s2Len);
    %(SATURATION_CHECK_INIT)s

    /* initialize result */
#ifdef PARASAIL_TABLE
    result = parasail_result_new_table1(s1Len, s2Len);
#else
#ifdef PARASAIL_ROWCOL
    result = parasail_result_new_rowcol1(s1Len, s2Len);
#else
    result = parasail_result_new();
#endif
#endif
    if (!result) return NULL;

    /* set known flags */
    result->flag |= PARASAIL_FLAG_SW | PARASAIL_FLAG_DIAG
        | PARASAIL_FLAG_BITS_%(WIDTH)s | PARASAIL_FLAG_LANES_%(LANES)s;
#ifdef PARASAIL_TABLE
    result->flag |= PARASAIL_FLAG_TABLE;
#endif
#ifdef PARASAIL_ROWCOL
    result->flag |= PARASAIL_FLAG_ROWCOL;
#endif

    /* initialize heap variables */
    s1 = parasail_memalign_%(INT)s(%(ALIGNMENT)s, s1Len+PAD);
    s2B= parasail_memalign_%(INT)s(%(ALIGNMENT)s, s2Len+PAD2);
    _H_pr = parasail_memalign_%(INT)s(%(ALIGNMENT)s, s2Len+PAD2);
    _F_pr = parasail_memalign_%(INT)s(%(ALIGNMENT)s, s2Len+PAD2);
    s2 = s2B+PAD; /* will allow later for negative indices */
    H_pr = _H_pr+PAD;
    F_pr = _F_pr+PAD;

    /* validate heap variables */
    if (!s1) return NULL;
    if (!s2B) return NULL;
    if (!_H_pr) return NULL;
    if (!_F_pr) return NULL;

    /* convert _s1 from char to int in range 0-23 */
    for (i=0; i<s1Len; ++i) {
        s1[i] = matrix->mapper[(unsigned char)_s1[i]];
    }
    /* pad back of s1 with dummy values */
    for (i=s1Len; i<s1Len_PAD; ++i) {
        s1[i] = 0; /* point to first matrix row because we don't care */
    }

    /* convert _s2 from char to int in range 0-23 */
    for (j=0; j<s2Len; ++j) {
        s2[j] = matrix->mapper[(unsigned char)_s2[j]];
    }
    /* pad front of s2 with dummy values */
    for (j=-PAD; j<0; ++j) {
        s2[j] = 0; /* point to first matrix row because we don't care */
    }
    /* pad back of s2 with dummy values */
    for (j=s2Len; j<s2Len_PAD; ++j) {
        s2[j] = 0; /* point to first matrix row because we don't care */
    }

    /* set initial values for stored row */
    for (j=0; j<s2Len; ++j) {
        H_pr[j] = 0;
        F_pr[j] = NEG_INF;
    }
    /* pad front of stored row values */
    for (j=-PAD; j<0; ++j) {
        H_pr[j] = NEG_INF;
        F_pr[j] = NEG_INF;
    }
    /* pad back of stored row values */
    for (j=s2Len; j<s2Len+PAD; ++j) {
        H_pr[j] = NEG_INF;
        F_pr[j] = NEG_INF;
    }

    /* iterate over query sequence */
    for (i=0; i<s1Len; i+=N) {
        %(VTYPE)s vNH = vNegInf0;
        %(VTYPE)s vWH = vNegInf0;
        %(VTYPE)s vE = vNegInf;
        %(VTYPE)s vF = vNegInf;
        %(VTYPE)s vJ = vJreset;
        %(DIAG_MATROW_DECL)s
        %(VTYPE)s vIltLimit = %(VCMPLT)s(vI, vILimit);
        /* iterate over database sequence */
        for (j=0; j<s2Len+PAD; ++j) {
            %(VTYPE)s vMat;
            %(VTYPE)s vNWH = vNH;
            vNH = %(VRSHIFT)s(vWH, %(BYTES)s);
            vNH = %(VINSERT)s(vNH, H_pr[j], %(LAST_POS)s);
            vF = %(VRSHIFT)s(vF, %(BYTES)s);
            vF = %(VINSERT)s(vF, F_pr[j], %(LAST_POS)s);
            vF = %(VMAX)s(
                    %(VSUB)s(vNH, vOpen),
                    %(VSUB)s(vF, vGap));
            vE = %(VMAX)s(
                    %(VSUB)s(vWH, vOpen),
                    %(VSUB)s(vE, vGap));
            vMat = %(VSET)s(
                    %(DIAG_MATROW_USE)s
                    );
            vNWH = %(VADD)s(vNWH, vMat);
            vWH = %(VMAX)s(vNWH, vE);
            vWH = %(VMAX)s(vWH, vF);
            vWH = %(VMAX)s(vWH, vZero);
            /* as minor diagonal vector passes across the j=-1 boundary,
             * assign the appropriate boundary conditions */
            {
                %(VTYPE)s cond = %(VCMPEQ)s(vJ,vNegOne);
                vWH = %(VANDNOT)s(cond, vWH);
                vF = %(VBLEND)s(vF, vNegInf, cond);
                vE = %(VBLEND)s(vE, vNegInf, cond);
            }
            %(SATURATION_CHECK_MID)s
#ifdef PARASAIL_TABLE
            arr_store_si%(BITS)s(result->tables->score_table, vWH, i, s1Len, j, s2Len);
#endif
#ifdef PARASAIL_ROWCOL
            arr_store_rowcol(result->rowcols->score_row, result->rowcols->score_col, vWH, i, s1Len, j, s2Len);
#endif
            H_pr[j-%(LAST_POS)s] = (%(INT)s)%(VEXTRACT)s(vWH,0);
            F_pr[j-%(LAST_POS)s] = (%(INT)s)%(VEXTRACT)s(vF,0);
            /* as minor diagonal vector passes across table, extract
             * max values within the i,j bounds */
            {
                %(VTYPE)s cond_valid_J = %(VAND)s(
                        %(VCMPGT)s(vJ, vNegOne),
                        %(VCMPLT)s(vJ, vJLimit));
                %(VTYPE)s cond_valid_IJ = %(VAND)s(cond_valid_J, vIltLimit);
                %(VTYPE)s cond_eq = %(VCMPEQ)s(vWH, vMaxH);
                %(VTYPE)s cond_max = %(VCMPGT)s(vWH, vMaxH);
                %(VTYPE)s cond_all = %(VAND)s(cond_max, cond_valid_IJ);
                %(VTYPE)s cond_Jlt = %(VCMPLT)s(vJ, vEndJ);
                vMaxH = %(VBLEND)s(vMaxH, vWH, cond_all);
                vEndI = %(VBLEND)s(vEndI, vI, cond_all);
                vEndJ = %(VBLEND)s(vEndJ, vJ, cond_all);
                cond_all = %(VAND)s(cond_Jlt, cond_eq);
                cond_all = %(VAND)s(cond_all, cond_valid_IJ);
                vEndI = %(VBLEND)s(vEndI, vI, cond_all);
                vEndJ = %(VBLEND)s(vEndJ, vJ, cond_all);
            }
            vJ = %(VADD)s(vJ, vOne);
        }
        vI = %(VADD)s(vI, vN);
    }

    /* alignment ending position */
    {
        %(INT)s *t = (%(INT)s*)&vMaxH;
        %(INT)s *i = (%(INT)s*)&vEndI;
        %(INT)s *j = (%(INT)s*)&vEndJ;
        %(INDEX)s k;
        for (k=0; k<N; ++k, ++t, ++i, ++j) {
            if (*t > score) {
                score = *t;
                end_query = *i;
                end_ref = *j;
            }
            else if (*t == score) {
                if (*j < end_ref) {
                    end_query = *i;
                    end_ref = *j;
                }
                else if (*j == end_ref && *i < end_query) {
                    end_query = *i;
                    end_ref = *j;
                }
            }
        }
    }

    %(SATURATION_CHECK_FINAL)s

    result->score = score;
    result->end_query = end_query;
    result->end_ref = end_ref;

    parasail_free(_F_pr);
    parasail_free(_H_pr);
    parasail_free(s2B);
    parasail_free(s1);

    return result;
}

