/***********************************************************************
 * Copyright 2010 Szymon Jakubczak
 * Copyright Henry Minsky (hqm@alum.mit.edu) 1991-2009
 *
 * This software library is licensed under terms of the GNU GENERAL
 * PUBLIC LICENSE
 *
 *
 * RSCODE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RSCODE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rscode.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Commercial licensing is available under a separate license, please
 * contact author for details.
 *
 * Source code is available at http://rscode.sourceforge.net
 * Berlekamp-Peterson and Berlekamp-Massey Algorithms for error-location
 *
 * From Cain, Clark, "Error-Correction Coding For Digital Communications", pp. 205.
 *
 * This finds the coefficients of the error locator polynomial.
 *
 * The roots are then found by looking for the values of a^n
 * where evaluating the polynomial yields zero.
 *
 * Error correction is done using the error-evaluator equation  on pp 207.
 *
 */

#include <stdio.h>
#include "ecc.h"

#include "galois.h"
#include "poly.h"

RS::Decoder::Decoder() {
  init_galois_tables();
}


/**********************************************************
 * Reed Solomon Decoder
 *
 * Computes the syndrome of a codeword. Puts the results
 * into the synBytes[] array.
 */

void
RS::Decoder::decode_data(unsigned char data[], int nbytes)
{
  int i, j, sum;
  for (j = 0; j < NPAR;  j++) {
    sum = 0;
    for (i = 0; i < nbytes; i++) {
      sum = data[i] ^ gmult(gexp[j+1], sum);
    }
    synBytes[j]  = sum;
  }
}


/* Check if the syndrome is zero */
int
RS::Decoder::check_syndrome (void)
{
 int i, nz = 0;
 for (i =0 ; i < NPAR; i++) {
  if (synBytes[i] != 0) {
      nz = 1;
      break;
  }
 }
 return nz;
}

// NOTE: globals: NErasures, synBytes, Lambda
/* From  Cain, Clark, "Error-Correction Coding For Digital Communications", pp. 216. */
void
RS::Decoder::Modified_Berlekamp_Massey (void)
{
  int n, L, L2, k, d, i;
  int psi[MAXDEG], psi2[MAXDEG], D[MAXDEG];
  int gamma[MAXDEG];

  /* initialize Gamma, the erasure locator polynomial */
  init_gamma(gamma);

  /* initialize to z */
  copy_poly(D, gamma);
  mul_z_poly(D);

  copy_poly(psi, gamma);
  k = -1; L = NErasures;

  for (n = NErasures; n < NPAR; n++) {

    d = compute_discrepancy(psi, synBytes, L, n);

    if (d != 0) {
      /* psi2 = psi - d*D */
      for (i = 0; i < MAXDEG; i++) psi2[i] = psi[i] ^ gmult(d, D[i]);

      if (L < (n-k)) {
        L2 = n-k;
        k = n-L;
        /* D = scale_poly(ginv(d), psi); */
        for (i = 0; i < MAXDEG; i++) D[i] = gmult(psi[i], ginv(d));
        L = L2;
      }

      /* psi = psi2 */
      copy_poly(psi, psi2);
    }

    mul_z_poly(D);
  }

  copy_poly(Lambda, psi);
  compute_modified_omega();
}

// NOTE: globals: NErasures, ErasureLocals
/* gamma = product (1-z*a^Ij) for erasure locs Ij */
void
RS::Decoder::init_gamma (int gamma[])
{
  int e, tmp[MAXDEG];

  zero_poly(gamma);
  zero_poly(tmp);
  gamma[0] = 1;

  for (e = 0; e < NErasures; e++) {
    copy_poly(tmp, gamma);
    scale_poly(gexp[ErasureLocs[e]], tmp);
    mul_z_poly(tmp);
    add_polys(gamma, tmp);
  }
}


/* given Psi (called Lambda in Modified_Berlekamp_Massey) and synBytes,
   compute the combined erasure/error evaluator polynomial as
   Psi*S mod z^4
  */
void
RS::Decoder::compute_modified_omega ()
{
  int i;
  int product[MAXDEG*2];

  mult_polys(product, Lambda, synBytes);
  zero_poly(Omega);
  for(i = 0; i < NPAR; i++) Omega[i] = product[i];

}


// NO Globals below this point

int
RS::Decoder::compute_discrepancy (int lambda[], int S[], int L, int n)
{
  int i, sum=0;

  for (i = 0; i <= L; i++)
    sum ^= gmult(lambda[i], S[n-i]);
  return (sum);
}




/* Finds all the roots of an error-locator polynomial with coefficients
 * Lambda[j] by evaluating Lambda at successive values of alpha.
 *
 * This can be tested with the decoder's equations case.
 */

// NOTE: globals: Lambda, NErrors, ErrorLocs
void
RS::Decoder::Find_Roots (void)
{
  int sum, r, k;
  NErrors = 0;

  for (r = 1; r < 256; r++) {
    sum = 0;
    /* evaluate lambda at r */
    for (k = 0; k < NPAR+1; k++) {
      sum ^= gmult(gexp[(k*r)%255], Lambda[k]);
    }
    if (sum == 0) {
      ErrorLocs[NErrors] = (255-r); NErrors++;
      if (DEBUG) fprintf(stderr, "Root found at r = %d, (255-r) = %d\n", r, (255-r));
    }
  }
}

// NOTE: globals: Nerasures, ErasureLocs, ErrorLocs, Lambda, Omega
/* Combined Erasure And Error Magnitude Computation
 *
 * Pass in the codeword, its size in bytes, as well as
 * an array of any known erasure locations, along the number
 * of these erasures.
 *
 * Evaluate Omega(actually Psi)/Lambda' at the roots
 * alpha^(-i) for error locs i.
 *
 * Returns 1 if everything ok, or 0 if an out-of-bounds error is found
 *
 */
int
RS::Decoder::correct_errors_erasures (unsigned char codeword[],
			 int csize,
			 int nerasures,
			 int erasures[])
{
  int r, i, j, err;

  /* If you want to take advantage of erasure correction, be sure to
     set NErasures and ErasureLocs[] with the locations of erasures.
     */
  NErasures = nerasures;
  for (i = 0; i < NErasures; i++) ErasureLocs[i] = erasures[i];

  Modified_Berlekamp_Massey();
  Find_Roots();


  if ((NErrors <= NPAR) && NErrors > 0) {

    /* first check for illegal error locs */
    for (r = 0; r < NErrors; r++) {
      if (ErrorLocs[r] >= csize) {
        if (DEBUG) fprintf(stderr, "Error loc i=%d outside of codeword length %d\n", i, csize);
        return(0);
      }
    }

    for (r = 0; r < NErrors; r++) {
      int num, denom;
      i = ErrorLocs[r];
      /* evaluate Omega at alpha^(-i) */

      num = 0;
      for (j = 0; j < MAXDEG; j++)
        num ^= gmult(Omega[j], gexp[((255-i)*j)%255]);

      /* evaluate Lambda' (derivative) at alpha^(-i) ; all odd powers disappear */
      denom = 0;
      for (j = 1; j < MAXDEG; j += 2) {
        denom ^= gmult(Lambda[j], gexp[((255-i)*(j-1)) % 255]);
      }

      err = gmult(num, ginv(denom));
      if (DEBUG) fprintf(stderr, "Error magnitude %#x at loc %d\n", err, csize-i);

      codeword[csize-i-1] ^= err;
    }
    return(1);
  }
  else {
    if (DEBUG && NErrors) fprintf(stderr, "Uncorrectable codeword\n");
    return(0);
  }
}

