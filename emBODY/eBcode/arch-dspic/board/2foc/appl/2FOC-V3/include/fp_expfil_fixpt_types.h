/*
 * Non-Degree Granting Education License -- for use at non-degree
 * granting, nonprofit, educational organizations only. Not for
 * government, commercial, or other organizational use.
 * File: fp_expfil_fixpt_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Jan-2021 11:54:31
 */

#ifndef FP_EXPFIL_FIXPT_TYPES_H
#define FP_EXPFIL_FIXPT_TYPES_H

/* Include Files */
#include "rtwtypes.h"

/* Type Definitions */
#ifndef typedef_int64m_T
#define typedef_int64m_T

typedef struct {
  unsigned long chunks[2];
} int64m_T;

#endif                                 /*typedef_int64m_T*/

#ifndef typedef_int96m_T
#define typedef_int96m_T

typedef struct {
  unsigned long chunks[3];
} int96m_T;

#endif                                 /*typedef_int96m_T*/
#endif

/*
 * File trailer for fp_expfil_fixpt_types.h
 *
 * [EOF]
 */
