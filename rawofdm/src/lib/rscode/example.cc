// Szymon Jakubczak, 2010
/* encode or decode stdin to stdout */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ecc.h"

unsigned char msg[256];
unsigned char code[256];

#define PAD 51 // shortened RS code
#define LEN 188

int
main (int argc, char *argv[])
{
  memset(msg, 0, sizeof(msg));
  memset(code, 0, sizeof(code));

  if (argv[1][0] == 'e') {
    RS::Encoder enc;
    // encode
    while(!feof(stdin)) {
      int nr = fread(msg + PAD, 1, LEN, stdin);
      if (nr == 0) break;
      enc.encode_data(msg, LEN + PAD, code);
      fwrite(code + PAD, 1, LEN + NPAR, stdout);
    }
  } else if (argv[1][0] == 'd') {
    RS::Decoder dec;
    // decode
    while(!feof(stdin)) {
      int nr = fread(code + PAD, 1, LEN + NPAR, stdin);
      if (nr == 0) break;
      dec.decode_data(code, LEN + NPAR + PAD);
      if (dec.check_syndrome () != 0) {
        dec.correct_errors_erasures (code, LEN + NPAR + PAD, 0, NULL);
      } // else what?
      fwrite(code + PAD, 1, LEN, stdout);
    }
  } else if (argv[1][0] == 's') {
    // strip
    while(!feof(stdin)) {
      int nr = fread(code + PAD, 1, LEN + NPAR, stdin);
      if (nr == 0) break;
      fwrite(code + PAD, 1, LEN, stdout);
    }
  }

  return 0;
}

