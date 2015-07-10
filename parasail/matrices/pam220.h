/**
 * @file
 *
 * @author jeff.daily@pnnl.gov
 *
 * Copyright (c) 2015 Battelle Memorial Institute.
 *
 * This file was converted to C code from the raw file found at
 * ftp://ftp.cbi.pku.edu.cn/pub/software/blast/matrices/PAM220, the
 * Center for Bioinformatics, Peking University, China.
 */
#ifndef _PARASAIL_PAM220_H_
#define _PARASAIL_PAM220_H_

#include "parasail.h"
#include "pam_map.h"

#ifdef __cplusplus
extern "C" {
#endif

/* # */
/* # This matrix was produced by "pam" Version 1.0.6 [28-Jul-93] */
/* # */
/* # PAM 220 substitution matrix, scale = ln(2)/3 = 0.231049 */
/* # */
/* # Expected score = -1.06, Entropy = 0.437 bits */
/* # */
/* # Lowest score = -8, Highest score = 17 */
/* # */

static const int parasail_pam220_[] = {
/*        A   R   N   D   C   Q   E   G   H   I   L   K   M   F   P   S   T   W   Y   V   B   Z   X   * */
/* A */   2, -2,  0,  0, -2, -1,  0,  1, -2, -1, -2, -1, -1, -4,  1,  1,  1, -6, -4,  0,  0,  0,  0, -8,
/* R */  -2,  7,  0, -2, -4,  1, -1, -3,  2, -2, -3,  4, -1, -5,  0,  0, -1,  2, -5, -3, -1,  0, -1, -8,
/* N */   0,  0,  3,  2, -4,  1,  2,  0,  2, -2, -3,  1, -2, -4, -1,  1,  0, -4, -2, -2,  2,  1,  0, -8,
/* D */   0, -2,  2,  4, -6,  2,  4,  0,  1, -3, -5,  0, -3, -6, -1,  0,  0, -8, -5, -3,  4,  3, -1, -8,
/* C */  -2, -4, -4, -6, 12, -6, -6, -4, -4, -3, -7, -6, -6, -5, -3,  0, -3, -8,  0, -2, -5, -6, -3, -8,
/* Q */  -1,  1,  1,  2, -6,  5,  3, -2,  3, -2, -2,  1, -1, -5,  0, -1, -1, -5, -5, -2,  1,  4, -1, -8,
/* E */   0, -1,  2,  4, -6,  3,  4,  0,  1, -2, -4,  0, -2, -6, -1,  0, -1, -8, -5, -2,  3,  4, -1, -8,
/* G */   1, -3,  0,  0, -4, -2,  0,  5, -3, -3, -5, -2, -3, -5, -1,  1,  0, -8, -6, -2,  0, -1, -1, -8,
/* H */  -2,  2,  2,  1, -4,  3,  1, -3,  7, -3, -2,  0, -3, -2,  0, -1, -2, -3,  0, -3,  1,  2, -1, -8,
/* I */  -1, -2, -2, -3, -3, -2, -2, -3, -3,  5,  2, -2,  2,  1, -2, -2,  0, -6, -1,  4, -2, -2, -1, -8,
/* L */  -2, -3, -3, -5, -7, -2, -4, -5, -2,  2,  6, -3,  4,  2, -3, -3, -2, -2, -1,  2, -4, -3, -2, -8,
/* K */  -1,  4,  1,  0, -6,  1,  0, -2,  0, -2, -3,  5,  1, -6, -1,  0,  0, -4, -5, -3,  0,  0, -1, -8,
/* M */  -1, -1, -2, -3, -6, -1, -2, -3, -3,  2,  4,  1,  8,  0, -2, -2, -1, -5, -3,  2, -3, -2, -1, -8,
/* F */  -4, -5, -4, -6, -5, -5, -6, -5, -2,  1,  2, -6,  0, 10, -5, -4, -4,  0,  7, -2, -5, -6, -3, -8,
/* P */   1,  0, -1, -1, -3,  0, -1, -1,  0, -2, -3, -1, -2, -5,  7,  1,  0, -6, -6, -1, -1,  0, -1, -8,
/* S */   1,  0,  1,  0,  0, -1,  0,  1, -1, -2, -3,  0, -2, -4,  1,  2,  2, -3, -3, -1,  0,  0,  0, -8,
/* T */   1, -1,  0,  0, -3, -1, -1,  0, -2,  0, -2,  0, -1, -4,  0,  2,  3, -6, -3,  0,  0, -1,  0, -8,
/* W */  -6,  2, -4, -8, -8, -5, -8, -8, -3, -6, -2, -4, -5,  0, -6, -3, -6, 17,  0, -7, -6, -7, -5, -8,
/* Y */  -4, -5, -2, -5,  0, -5, -5, -6,  0, -1, -1, -5, -3,  7, -6, -3, -3,  0, 11, -3, -3, -5, -3, -8,
/* V */   0, -3, -2, -3, -2, -2, -2, -2, -3,  4,  2, -3,  2, -2, -1, -1,  0, -7, -3,  5, -2, -2, -1, -8,
/* B */   0, -1,  2,  4, -5,  1,  3,  0,  1, -2, -4,  0, -3, -5, -1,  0,  0, -6, -3, -2,  3,  2, -1, -8,
/* Z */   0,  0,  1,  3, -6,  4,  4, -1,  2, -2, -3,  0, -2, -6,  0,  0, -1, -7, -5, -2,  2,  4, -1, -8,
/* X */   0, -1,  0, -1, -3, -1, -1, -1, -1, -1, -2, -1, -1, -3, -1,  0,  0, -5, -3, -1, -1, -1, -1, -8,
/* * */  -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8,  1
};

static const parasail_matrix_t parasail_pam220 = {
    "pam220",
    parasail_pam220_,
    parasail_pam_map,
    24,
    0
};

#ifdef __cplusplus
}
#endif

#endif /* _PARASAIL_PAM220_H_ */

