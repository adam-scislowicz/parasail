/**
 * @file
 *
 * @author jeff.daily@pnnl.gov
 *
 * Copyright (c) 2015 Battelle Memorial Institute.
 *
 * This file was converted to C code from the raw file found at
 * ftp://ftp.cbi.pku.edu.cn/pub/software/blast/matrices/PAM310, the
 * Center for Bioinformatics, Peking University, China.
 */
#ifndef _PARASAIL_PAM310_H_
#define _PARASAIL_PAM310_H_

#include "parasail.h"
#include "pam_map.h"

#ifdef __cplusplus
extern "C" {
#endif

/* # */
/* # This matrix was produced by "pam" Version 1.0.6 [28-Jul-93] */
/* # */
/* # PAM 310 substitution matrix, scale = ln(2)/4 = 0.173287 */
/* # */
/* # Expected score = -0.779, Entropy = 0.238 bits */
/* # */
/* # Lowest score = -9, Highest score = 22 */
/* # */

static const int parasail_pam310_[] = {
/*        A   R   N   D   C   Q   E   G   H   I   L   K   M   F   P   S   T   W   Y   V   B   Z   X   * */
/* A */   2, -1,  0,  0, -2,  0,  0,  1, -1,  0, -2, -1, -1, -4,  1,  1,  1, -6, -4,  0,  0,  0,  0, -9,
/* R */  -1,  6,  0, -1, -4,  1, -1, -2,  2, -2, -3,  4,  0, -5,  0,  0, -1,  3, -4, -2,  0,  0, -1, -9,
/* N */   0,  0,  2,  2, -4,  1,  2,  1,  2, -2, -3,  1, -2, -4,  0,  1,  0, -4, -2, -2,  2,  1,  0, -9,
/* D */   0, -1,  2,  4, -5,  2,  4,  1,  1, -2, -4,  0, -2, -6, -1,  0,  0, -7, -5, -2,  3,  3, -1, -9,
/* C */  -2, -4, -4, -5, 15, -6, -6, -3, -4, -2, -6, -6, -6, -4, -3,  0, -2, -9,  1, -2, -5, -6, -3, -9,
/* Q */   0,  1,  1,  2, -6,  4,  3, -1,  3, -2, -2,  1, -1, -5,  0,  0, -1, -5, -4, -2,  2,  3,  0, -9,
/* E */   0, -1,  2,  4, -6,  3,  4,  1,  1, -2, -3,  0, -2, -6,  0,  0,  0, -7, -5, -2,  3,  3, -1, -9,
/* G */   1, -2,  1,  1, -3, -1,  1,  5, -2, -2, -4, -1, -3, -5,  0,  1,  0, -8, -6, -1,  1,  0, -1, -9,
/* H */  -1,  2,  2,  1, -4,  3,  1, -2,  7, -2, -2,  0, -2, -2,  0, -1, -1, -3,  0, -2,  1,  2,  0, -9,
/* I */   0, -2, -2, -2, -2, -2, -2, -2, -2,  4,  3, -2,  3,  1, -2, -1,  0, -5, -1,  4, -2, -2, -1, -9,
/* L */  -2, -3, -3, -4, -6, -2, -3, -4, -2,  3,  7, -3,  4,  3, -3, -3, -2, -2,  0,  2, -3, -3, -1, -9,
/* K */  -1,  4,  1,  0, -6,  1,  0, -1,  0, -2, -3,  5,  0, -5, -1,  0,  0, -3, -5, -2,  1,  1, -1, -9,
/* M */  -1,  0, -2, -2, -6, -1, -2, -3, -2,  3,  4,  0,  6,  1, -2, -2, -1, -4, -2,  2, -2, -2, -1, -9,
/* F */  -4, -5, -4, -6, -4, -5, -6, -5, -2,  1,  3, -5,  1, 11, -5, -4, -3,  1,  9, -1, -5, -5, -2, -9,
/* P */   1,  0,  0, -1, -3,  0,  0,  0,  0, -2, -3, -1, -2, -5,  6,  1,  1, -6, -5, -1,  0,  0,  0, -9,
/* S */   1,  0,  1,  0,  0,  0,  0,  1, -1, -1, -3,  0, -2, -4,  1,  1,  1, -3, -3, -1,  1,  0,  0, -9,
/* T */   1, -1,  0,  0, -2, -1,  0,  0, -1,  0, -2,  0, -1, -3,  1,  1,  2, -6, -3,  0,  0,  0,  0, -9,
/* W */  -6,  3, -4, -7, -9, -5, -7, -8, -3, -5, -2, -3, -4,  1, -6, -3, -6, 22,  0, -7, -6, -6, -4, -9,
/* Y */  -4, -4, -2, -5,  1, -4, -5, -6,  0, -1,  0, -5, -2,  9, -5, -3, -3,  0, 12, -2, -4, -4, -2, -9,
/* V */   0, -2, -2, -2, -2, -2, -2, -1, -2,  4,  2, -2,  2, -1, -1, -1,  0, -7, -2,  4, -2, -2,  0, -9,
/* B */   0,  0,  2,  3, -5,  2,  3,  1,  1, -2, -3,  1, -2, -5,  0,  1,  0, -6, -4, -2,  3,  2,  0, -9,
/* Z */   0,  0,  1,  3, -6,  3,  3,  0,  2, -2, -3,  1, -2, -5,  0,  0,  0, -6, -4, -2,  2,  3,  0, -9,
/* X */   0, -1,  0, -1, -3,  0, -1, -1,  0, -1, -1, -1, -1, -2,  0,  0,  0, -4, -2,  0,  0,  0, -1, -9,
/* * */  -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9,  1
};

static const parasail_matrix_t parasail_pam310 = {
    "pam310",
    parasail_pam310_,
    parasail_pam_map,
    24,
    0
};

#ifdef __cplusplus
}
#endif

#endif /* _PARASAIL_PAM310_H_ */

