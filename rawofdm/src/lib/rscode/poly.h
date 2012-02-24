#ifndef POLY_INCLUDED
#define POLY_INCLUDED

#include "ecc.h"
#include "galois.h"

/* polynomial arithmetic */
void mult_polys(int dst[], int p1[], int p2[]);


/********** polynomial arithmetic *******************/

static inline
void add_polys (int dst[], int src[])
{
  int i;
  for (i = 0; i < MAXDEG; i++) dst[i] ^= src[i];
}

static inline
void copy_poly (int dst[], int src[])
{
  int i;
  for (i = 0; i < MAXDEG; i++) dst[i] = src[i];
}

static inline
void scale_poly (int k, int poly[])
{
  int i;
  for (i = 0; i < MAXDEG; i++) poly[i] = gmult(k, poly[i]);
}

static inline
void zero_poly (int poly[])
{
  int i;
  for (i = 0; i < MAXDEG; i++) poly[i] = 0;
}


/* multiply by z, i.e., shift right by 1 */
static inline
void mul_z_poly (int src[])
{
  int i;
  for (i = MAXDEG-1; i > 0; i--) src[i] = src[i-1];
  src[0] = 0;
}


#endif
