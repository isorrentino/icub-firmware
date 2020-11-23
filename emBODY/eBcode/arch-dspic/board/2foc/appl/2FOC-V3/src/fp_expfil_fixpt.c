/*
 * Non-Degree Granting Education License -- for use at non-degree
 * granting, nonprofit, educational organizations only. Not for
 * government, commercial, or other organizational use.
 * File: fp_expfil_fixpt.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 23-Nov-2020 16:02:15
 */

/* Include Files */
#include "fp_expfil_fixpt.h"
#include <string.h>

/* Function Declarations */
static long MultiWord2sLong(const unsigned long u[]);
static void MultiWordAdd(const unsigned long u1[], const unsigned long u2[],
  unsigned long y[], int n);
static void MultiWordSignedWrap(const unsigned long u1[], int n1, unsigned int
  n2, unsigned long y[]);
static void MultiWordSub(const unsigned long u1[], const unsigned long u2[],
  unsigned long y[], int n);
static void sLong2MultiWord(long u, unsigned long y[], int n);
static void sMultiWord2MultiWord(const unsigned long u1[], int n1, unsigned long
  y[], int n);
static void sMultiWordMul(const unsigned long u1[], int n1, const unsigned long
  u2[], int n2, unsigned long y[], int n);
static void sMultiWordShr(const unsigned long u1[], int n1, unsigned int n2,
  unsigned long y[], int n);
static void ssuMultiWordMul(const unsigned long u1[], int n1, const unsigned
  long u2[], int n2, unsigned long y[], int n);

/* Function Definitions */

/*
 * Arguments    : const unsigned long u[]
 * Return Type  : long
 */
static long MultiWord2sLong(const unsigned long u[])
{
  return (long)u[0];
}

/*
 * Arguments    : const unsigned long u1[]
 *                const unsigned long u2[]
 *                unsigned long y[]
 *                int n
 * Return Type  : void
 */
static void MultiWordAdd(const unsigned long u1[], const unsigned long u2[],
  unsigned long y[], int n)
{
  int i;
  unsigned long u1i;
  unsigned long yi;
  int carry = 0;
  for (i = 0; i < n; i++) {
    u1i = u1[i];
    yi = (u1i + u2[i]) + carry;
    y[i] = yi;
    if ((unsigned int)carry != 0U) {
      carry = (yi <= u1i);
    } else {
      carry = (yi < u1i);
    }
  }
}

/*
 * Arguments    : const unsigned long u1[]
 *                int n1
 *                unsigned int n2
 *                unsigned long y[]
 * Return Type  : void
 */
static void MultiWordSignedWrap(const unsigned long u1[], int n1, unsigned int
  n2, unsigned long y[])
{
  int n1m1;
  unsigned long mask;
  unsigned long ys;
  n1m1 = n1 - 1;
  if (0 <= n1m1 - 1) {
    memcpy(&y[0], &u1[0], n1m1 * sizeof(unsigned long));
  }

  mask = 1UL << (31U - n2);
  if ((u1[n1m1] & mask) != 0UL) {
    ys = MAX_uint32_T;
  } else {
    ys = 0UL;
  }

  mask = (mask << 1U) - 1UL;
  y[n1m1] = (u1[n1m1] & mask) | (~mask & ys);
}

/*
 * Arguments    : const unsigned long u1[]
 *                const unsigned long u2[]
 *                unsigned long y[]
 *                int n
 * Return Type  : void
 */
static void MultiWordSub(const unsigned long u1[], const unsigned long u2[],
  unsigned long y[], int n)
{
  int i;
  unsigned long u1i;
  unsigned long yi;
  int borrow = 0;
  for (i = 0; i < n; i++) {
    u1i = u1[i];
    yi = (u1i - u2[i]) - borrow;
    y[i] = yi;
    if ((unsigned int)borrow != 0U) {
      borrow = (yi >= u1i);
    } else {
      borrow = (yi > u1i);
    }
  }
}

/*
 * Arguments    : long u
 *                unsigned long y[]
 *                int n
 * Return Type  : void
 */
static void sLong2MultiWord(long u, unsigned long y[], int n)
{
  unsigned long yi;
  int i;
  y[0] = (unsigned long)u;
  if (u < 0L) {
    yi = MAX_uint32_T;
  } else {
    yi = 0UL;
  }

  for (i = 1; i < n; i++) {
    y[i] = yi;
  }
}

/*
 * Arguments    : const unsigned long u1[]
 *                int n1
 *                unsigned long y[]
 *                int n
 * Return Type  : void
 */
static void sMultiWord2MultiWord(const unsigned long u1[], int n1, unsigned long
  y[], int n)
{
  int nm;
  unsigned long u1i;
  int i;
  if (n1 < n) {
    nm = n1;
  } else {
    nm = n;
  }

  if (0 <= nm - 1) {
    memcpy(&y[0], &u1[0], nm * sizeof(unsigned long));
  }

  if (n > n1) {
    if ((u1[n1 - 1] & 2147483648UL) != 0UL) {
      u1i = MAX_uint32_T;
    } else {
      u1i = 0UL;
    }

    for (i = nm; i < n; i++) {
      y[i] = u1i;
    }
  }
}

/*
 * Arguments    : const unsigned long u1[]
 *                int n1
 *                const unsigned long u2[]
 *                int n2
 *                unsigned long y[]
 *                int n
 * Return Type  : void
 */
static void sMultiWordMul(const unsigned long u1[], int n1, const unsigned long
  u2[], int n2, unsigned long y[], int n)
{
  bool isNegative1;
  bool isNegative2;
  int cb1;
  int i;
  unsigned long cb;
  unsigned long u1i;
  int k;
  unsigned int a1;
  unsigned long yk;
  unsigned int a0;
  int cb2;
  int ni;
  int j;
  unsigned int b1;
  unsigned int b0;
  unsigned long w01;
  unsigned long t;
  isNegative1 = ((u1[n1 - 1] & 2147483648UL) != 0UL);
  isNegative2 = ((u2[n2 - 1] & 2147483648UL) != 0UL);
  cb1 = 1;

  /* Initialize output to zero */
  if (0 <= n - 1) {
    memset(&y[0], 0, n * sizeof(unsigned long));
  }

  for (i = 0; i < n1; i++) {
    cb = 0UL;
    u1i = u1[i];
    if (isNegative1) {
      u1i = ~u1i + cb1;
      cb1 = (u1i < (unsigned long)cb1);
    }

    a1 = (unsigned int)(u1i >> 16U);
    a0 = (unsigned int)u1i & MAX_uint16_T;
    cb2 = 1;
    ni = n - i;
    if (n2 <= ni) {
      ni = n2;
    }

    k = i;
    for (j = 0; j < ni; j++) {
      u1i = u2[j];
      if (isNegative2) {
        u1i = ~u1i + cb2;
        cb2 = (u1i < (unsigned long)cb2);
      }

      b1 = (unsigned int)(u1i >> 16U);
      b0 = (unsigned int)u1i & MAX_uint16_T;
      u1i = (unsigned long)a1 * b0;
      w01 = (unsigned long)a0 * b1;
      yk = y[k] + cb;
      cb = (unsigned long)(yk < cb);
      t = (unsigned long)a0 * b0;
      yk += t;
      cb = (unsigned long)((int)cb + (yk < t));
      t = u1i << 16U;
      yk += t;
      cb = (unsigned long)((int)cb + (yk < t));
      t = w01 << 16U;
      yk += t;
      cb = (unsigned long)((int)cb + (yk < t));
      y[k] = yk;
      cb += u1i >> 16U;
      cb += w01 >> 16U;
      cb += (unsigned long)a1 * b1;
      k++;
    }

    if (k < n) {
      y[k] = cb;
    }
  }

  /* Apply sign */
  if (isNegative1 != isNegative2) {
    cb = 1UL;
    for (k = 0; k < n; k++) {
      yk = ~y[k] + cb;
      y[k] = yk;
      cb = (unsigned long)(yk < cb);
    }
  }
}

/*
 * Arguments    : const unsigned long u1[]
 *                int n1
 *                unsigned int n2
 *                unsigned long y[]
 *                int n
 * Return Type  : void
 */
static void sMultiWordShr(const unsigned long u1[], int n1, unsigned int n2,
  unsigned long y[], int n)
{
  int nb;
  int i;
  unsigned long ys;
  int nc;
  unsigned int nr;
  int i1;
  unsigned int nl;
  unsigned long u1i;
  unsigned long yi;
  nb = (int)n2 / 32;
  i = 0;
  if ((u1[n1 - 1] & 2147483648UL) != 0UL) {
    ys = MAX_uint32_T;
  } else {
    ys = 0UL;
  }

  if (nb < n1) {
    nc = n + nb;
    if (nc > n1) {
      nc = n1;
    }

    nr = n2 - nb * 32U;
    if (nr > 0U) {
      nl = 32U - nr;
      u1i = u1[nb];
      for (i1 = nb + 1; i1 < nc; i1++) {
        yi = u1i >> nr;
        u1i = u1[i1];
        y[i] = yi | u1i << nl;
        i++;
      }

      if (nc < n1) {
        yi = u1[nc];
      } else {
        yi = ys;
      }

      y[i] = u1i >> nr | yi << nl;
      i++;
    } else {
      for (i1 = nb; i1 < nc; i1++) {
        y[i] = u1[i1];
        i++;
      }
    }
  }

  while (i < n) {
    y[i] = ys;
    i++;
  }
}

/*
 * Arguments    : const unsigned long u1[]
 *                int n1
 *                const unsigned long u2[]
 *                int n2
 *                unsigned long y[]
 *                int n
 * Return Type  : void
 */
static void ssuMultiWordMul(const unsigned long u1[], int n1, const unsigned
  long u2[], int n2, unsigned long y[], int n)
{
  bool isNegative1;
  int cb1;
  int i;
  unsigned long cb;
  unsigned long u1i;
  int k;
  unsigned int a1;
  unsigned long yk;
  unsigned int a0;
  int ni;
  int j;
  unsigned int b1;
  unsigned int b0;
  unsigned long w01;
  unsigned long t;
  isNegative1 = ((u1[n1 - 1] & 2147483648UL) != 0UL);
  cb1 = 1;

  /* Initialize output to zero */
  if (0 <= n - 1) {
    memset(&y[0], 0, n * sizeof(unsigned long));
  }

  for (i = 0; i < n1; i++) {
    cb = 0UL;
    u1i = u1[i];
    if (isNegative1) {
      u1i = ~u1i + cb1;
      cb1 = (u1i < (unsigned long)cb1);
    }

    a1 = (unsigned int)(u1i >> 16U);
    a0 = (unsigned int)u1i & MAX_uint16_T;
    ni = n - i;
    if (n2 <= ni) {
      ni = n2;
    }

    k = i;
    for (j = 0; j < ni; j++) {
      u1i = u2[j];
      b1 = (unsigned int)(u1i >> 16U);
      b0 = (unsigned int)u1i & MAX_uint16_T;
      u1i = (unsigned long)a1 * b0;
      w01 = (unsigned long)a0 * b1;
      yk = y[k] + cb;
      cb = (unsigned long)(yk < cb);
      t = (unsigned long)a0 * b0;
      yk += t;
      cb = (unsigned long)((int)cb + (yk < t));
      t = u1i << 16U;
      yk += t;
      cb = (unsigned long)((int)cb + (yk < t));
      t = w01 << 16U;
      yk += t;
      cb = (unsigned long)((int)cb + (yk < t));
      y[k] = yk;
      cb += u1i >> 16U;
      cb += w01 >> 16U;
      cb += (unsigned long)a1 * b1;
      k++;
    }

    if (k < n) {
      y[k] = cb;
    }
  }

  /* Apply sign */
  if (isNegative1) {
    cb = 1UL;
    for (k = 0; k < n; k++) {
      yk = ~y[k] + cb;
      y[k] = yk;
      cb = (unsigned long)(yk < cb);
    }
  }
}

/*
 * FP_EXPFIL Summary of this function goes here
 *    Detailed explanation goes here
 * Arguments    : long x_pre
 *                long dx_pre
 *                long x
 *                unsigned int freq
 * Return Type  : long
 */
long fp_expfil_fixpt(long x_pre, long dx_pre, long x, unsigned int freq)
{
  unsigned long u;
  unsigned long u1;
  int64m_T r;
  int96m_T r1;
  int96m_T r2;
  int64m_T r3;
  int64m_T r4;
  int64m_T r5;
  int64m_T r6;
  int64m_T r7;
  int96m_T r8;
  int96m_T r9;
  int96m_T r10;
  int96m_T r11;

  /* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
  /*                                                                           % */
  /*            Generated by MATLAB 9.8 and Fixed-Point Designer 7.0           % */
  /*                                                                           % */
  /* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
  /* 1 */
  u = 65484UL;
  u1 = (unsigned long)dx_pre;
  sMultiWordMul((unsigned long *)&u, 1, (unsigned long *)&u1, 1, (unsigned long *)
                &r.chunks[0U], 2);
  sMultiWord2MultiWord((unsigned long *)&r.chunks[0U], 2, (unsigned long *)
                       &r1.chunks[0U], 3);
  MultiWordSignedWrap((unsigned long *)&r1.chunks[0U], 3, 30U, (unsigned long *)
                      &r2.chunks[0U]);
  sLong2MultiWord(x, (unsigned long *)&r3.chunks[0U], 2);
  MultiWordSignedWrap((unsigned long *)&r3.chunks[0U], 2, 31U, (unsigned long *)
                      &r4.chunks[0U]);
  sLong2MultiWord(x_pre, (unsigned long *)&r5.chunks[0U], 2);
  MultiWordSignedWrap((unsigned long *)&r5.chunks[0U], 2, 31U, (unsigned long *)
                      &r3.chunks[0U]);
  MultiWordSub((unsigned long *)&r4.chunks[0U], (unsigned long *)&r3.chunks[0U],
               (unsigned long *)&r6.chunks[0U], 2);
  MultiWordSignedWrap((unsigned long *)&r6.chunks[0U], 2, 31U, (unsigned long *)
                      &r7.chunks[0U]);
  u = freq;
  ssuMultiWordMul((unsigned long *)&r7.chunks[0U], 2, (unsigned long *)&u, 1,
                  (unsigned long *)&r.chunks[0U], 2);
  u = 52UL;
  ssuMultiWordMul((unsigned long *)&r.chunks[0U], 2, (unsigned long *)&u, 1,
                  (unsigned long *)&r8.chunks[0U], 3);
  MultiWordSignedWrap((unsigned long *)&r8.chunks[0U], 3, 30U, (unsigned long *)
                      &r1.chunks[0U]);
  MultiWordAdd((unsigned long *)&r2.chunks[0U], (unsigned long *)&r1.chunks[0U],
               (unsigned long *)&r9.chunks[0U], 3);
  MultiWordSignedWrap((unsigned long *)&r9.chunks[0U], 3, 30U, (unsigned long *)
                      &r10.chunks[0U]);
  sMultiWordShr((unsigned long *)&r10.chunks[0U], 3, 16U, (unsigned long *)
                &r11.chunks[0U], 3);
  return MultiWord2sLong((unsigned long *)&r11.chunks[0U]);
}

/*
 * File trailer for fp_expfil_fixpt.c
 *
 * [EOF]
 */
