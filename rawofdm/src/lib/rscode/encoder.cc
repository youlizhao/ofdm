/*
 * Reed Solomon Encoder/Decoder
 *
 * Copyright 2010 Szymon Jakubczak
 * Copyright Henry Minsky (hqm@alum.mit.edu) 1991-2009
 *
 * This software library is licensed under terms of the GNU GENERAL
 * PUBLIC LICENSE
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

 * Commercial licensing is available under a separate license, please
 * contact author for details.
 *
 * Source code is available at http://rscode.sourceforge.net
 */

#include <stdio.h>
#include "ecc.h"

#include "galois.h"
#include "poly.h"

static void
compute_genpoly (int nbytes, int genpoly[]);

/* Initialize lookup tables, polynomials, etc. */
RS::Encoder::Encoder()
{
    /* Initialize the galois field arithmetic tables */
    init_galois_tables();

    /* Compute the encoder generator polynomial */
    compute_genpoly(NPAR, genPoly);
}



/* Append the parity bytes onto the end of the message */
void
RS::Encoder::build_codeword (unsigned char msg[], int nbytes, unsigned char dst[])
{
  int i;

  for (i = 0; i < nbytes; i++) dst[i] = msg[i];

  for (i = 0; i < NPAR; i++) {
    dst[i+nbytes] = pBytes[NPAR-1-i];
  }
}

/* Create a generator polynomial for an n byte RS code.
 * The coefficients are returned in the genPoly arg.
 * Make sure that the genPoly array which is passed in is
 * at least n+1 bytes long.
 */
static void
compute_genpoly (int nbytes, int genpoly[])
{
  int i, tp[256], tp1[256];

  /* multiply (x + a^n) for n = 1 to nbytes */

  zero_poly(tp1);
  tp1[0] = 1;

  for (i = 1; i <= nbytes; i++) {
    zero_poly(tp);
    tp[0] = gexp[i];		/* set up x+a^n */
    tp[1] = 1;

    mult_polys(genpoly, tp, tp1);
    copy_poly(tp1, genpoly);
  }
}

/* Simulate a LFSR with generator polynomial for n byte RS code.
 * Pass in a pointer to the data array, and amount of data.
 *
 * The parity bytes are deposited into pBytes[], and the whole message
 * and parity are copied to dest to make a codeword.
 *
 */

void
RS::Encoder::encode_data (unsigned char msg[], int nbytes, unsigned char dst[])
{
  int i, LFSR[NPAR+1],dbyte, j;

  for(i=0; i < NPAR+1; i++) LFSR[i]=0;

  for (i = 0; i < nbytes; i++) {
    dbyte = msg[i] ^ LFSR[NPAR-1];
    for (j = NPAR-1; j > 0; j--) {
      LFSR[j] = LFSR[j-1] ^ gmult(genPoly[j], dbyte);
    }
    LFSR[0] = gmult(genPoly[0], dbyte);
  }

  for (i = 0; i < NPAR; i++)
    pBytes[i] = LFSR[i];

  build_codeword(msg, nbytes, dst);
}

