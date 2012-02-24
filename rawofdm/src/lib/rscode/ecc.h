/* Reed Solomon Coding for glyphs
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
 *
 * Source code is available at http://rscode.sourceforge.net
 *
 * Commercial licensing is available under a separate license, please
 * contact author for details.
 *
 */

/****************************************************************

  Below is NPAR, the only compile-time parameter you should have to
  modify.

  It is the number of parity bytes which will be appended to
  your data to create a codeword.

  Note that the maximum codeword size is 255, so the
  sum of your message length plus parity should be less than
  or equal to this maximum limit.

  In practice, you will get slooow error correction and decoding
  if you use more than a reasonably small number of parity bytes.
  (say, 10 or 20)

  ****************************************************************/

#ifndef ECC_INCLUDED
#define ECC_INCLUDED

#define NPAR 16

/* Maximum degree of various polynomials. */
#define MAXDEG (NPAR*2)

#define DEBUG 0

namespace RS {

/*************************************/
class Encoder {
  /* Encoder parity bytes */
  int pBytes[MAXDEG];
  /* generator polynomial */
  int genPoly[MAXDEG*2]; // actually should be static

  void build_codeword (unsigned char msg[], int nbytes, unsigned char dst[]);
public:
  Encoder();
  void encode_data (unsigned char msg[], int nbytes, unsigned char dst[]);
};

class Decoder {
  /* Decoder syndrome bytes */
  int synBytes[MAXDEG];

  /* The Error Locator Polynomial, also known as Lambda or Sigma. Lambda[0] == 1 */
  int Lambda[MAXDEG];

  /* The Error Evaluator Polynomial */
  int Omega[MAXDEG];

  /* error locations found using Chien's search*/
  int ErrorLocs[256];
  int NErrors;

  /* erasure flags */
  int ErasureLocs[256];
  int NErasures;

  void init_gamma(int gamma[]);
  void compute_modified_omega ();
  int compute_discrepancy (int lambda[], int S[], int L, int n);
  void Modified_Berlekamp_Massey (void);
  void Find_Roots (void);

public:
  Decoder();
  int check_syndrome (void);
  void decode_data (unsigned char data[], int nbytes);
  /* Error location routines */
  int correct_errors_erasures (unsigned char codeword[], int csize,int nerasures, int erasures[]);
};

}

#endif