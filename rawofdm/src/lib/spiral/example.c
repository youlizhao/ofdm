/*
 * Copyright 2010 Szymon Jakubczak
 * Copyright 2010 Project Spiral
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <memory.h>
#include <sys/resource.h>

#include <assert.h>

#include "viterbi.h"
#include "parity.h"

#define OFFSET (127.5)
#define CLIP 255
#define MAX_RANDOM      0x7fffffff

#define LOG printf

/* Lookup table giving count of 1 bits for integers 0-255 */
static int Bitcnt[] = {
 0, 1, 1, 2, 1, 2, 2, 3,
 1, 2, 2, 3, 2, 3, 3, 4,
 1, 2, 2, 3, 2, 3, 3, 4,
 2, 3, 3, 4, 3, 4, 4, 5,
 1, 2, 2, 3, 2, 3, 3, 4,
 2, 3, 3, 4, 3, 4, 4, 5,
 2, 3, 3, 4, 3, 4, 4, 5,
 3, 4, 4, 5, 4, 5, 5, 6,
 1, 2, 2, 3, 2, 3, 3, 4,
 2, 3, 3, 4, 3, 4, 4, 5,
 2, 3, 3, 4, 3, 4, 4, 5,
 3, 4, 4, 5, 4, 5, 5, 6,
 2, 3, 3, 4, 3, 4, 4, 5,
 3, 4, 4, 5, 4, 5, 5, 6,
 3, 4, 4, 5, 4, 5, 5, 6,
 4, 5, 5, 6, 5, 6, 6, 7,
 1, 2, 2, 3, 2, 3, 3, 4,
 2, 3, 3, 4, 3, 4, 4, 5,
 2, 3, 3, 4, 3, 4, 4, 5,
 3, 4, 4, 5, 4, 5, 5, 6,
 2, 3, 3, 4, 3, 4, 4, 5,
 3, 4, 4, 5, 4, 5, 5, 6,
 3, 4, 4, 5, 4, 5, 5, 6,
 4, 5, 5, 6, 5, 6, 6, 7,
 2, 3, 3, 4, 3, 4, 4, 5,
 3, 4, 4, 5, 4, 5, 5, 6,
 3, 4, 4, 5, 4, 5, 5, 6,
 4, 5, 5, 6, 5, 6, 6, 7,
 3, 4, 4, 5, 4, 5, 5, 6,
 4, 5, 5, 6, 5, 6, 6, 7,
 4, 5, 5, 6, 5, 6, 6, 7,
 5, 6, 6, 7, 6, 7, 7, 8,
};

/* Generate gaussian random double with specified mean and std_dev */
static double normal_rand(double mean, double std_dev) {
  double fac,rsq,v1,v2;
  static double gset;
  static int iset;

  if(iset){
    /* Already got one */
    iset = 0;
    return mean + std_dev*gset;
  }
  /* Generate two evenly distributed numbers between -1 and +1
   * that are inside the unit circle
   */
  do {
    v1 = 2.0 * (double)random() / MAX_RANDOM - 1;
    v2 = 2.0 * (double)random() / MAX_RANDOM - 1;
    rsq = v1*v1 + v2*v2;
  } while(rsq >= 1.0 || rsq == 0.0);
  fac = sqrt(-2.0*log(rsq)/rsq);
  gset = v1*fac;
  iset++;
  return mean + std_dev*v2*fac;
}

// output is OFFSET +- amp * gain
static unsigned char addnoise(int sym, double amp) {
  // NOTE: probability of error depends on amp only
  int sample = OFFSET + 32.0 * normal_rand(sym ? amp : -amp, 1.0);
  /* Clip to 8-bit offset range */
  if(sample < 0)
    sample = 0;
  else if(sample > CLIP)
    sample = CLIP;

  // hard decider
  //if (sample > OFFSET) sample = CLIP; else sample = 0;
  return sample;
}


static int allocate(void **bytes, void **data, void **symbols, int nbits) {
  if (posix_memalign(bytes, 16, (nbits+(K-1))/8+1)) {
    fprintf(stderr, "Allocation of bytes array failed\n");
    return -1;
  }

  if (posix_memalign(data, 16, (nbits+(K-1))/8+1)) {
    fprintf(stderr, "Allocation of data array failed\n");
    return -1;
  }

  if (posix_memalign(symbols, 16, RATE*(nbits+(K-1))*sizeof(COMPUTETYPE))) {
    fprintf(stderr, "Allocation of symbols array failed\n");
    return -1;
  }
  return 0;
}

static void verbose() {
  int polys[RATE] = POLYS;
  LOG("Code specifications:\n");
  LOG("\t rate = 1/%d \n", RATE);
  LOG("\t K = %d (thus %d states) \n", K, NUMSTATES);
  LOG("\t frame size = %d (thus padded frame size = %d) \n", FRAMEBITS, FRAMEBITS+(K-1));
  LOG("\t polynomials = { ");
  for(int i=0; i<RATE-1; i++)
    LOG("%d, ", polys[i]);
  LOG("%d }\n", polys[RATE-1]);
  LOG("\n");
}


static void fillrandom (unsigned char *bytes, int nbits) {
#if 0
  int sr = 0;
  for(int i = 0; i < nbits; i++) {
    int bit = (random() & 1);
    sr = (sr << 1) | bit;
    bytes[i/8] = sr & 0xff;
  } // NOTE: we assume it's always even number of bytes!!
  // NOTE: the last K-1 bits are 0-pads
  for(int i = nbits; i < nbits+(K-1); i++) {
    sr = (sr << 1);
    bytes[i/8] = sr & 0xff;
  }
#else
  for(int i = 0; i < nbits/8; i++) {
    bytes[i] = random() & 0xFF;
  } // NOTE: we assume it's always even number of bytes!!
  // NOTE: the last K-1 bits are 0-pads
  for(int i = nbits; i < nbits+(K-1); i++) {
    bytes[i/8] = 0;
  }
#endif
  // fix the last byte (for us it's always the top bits that matter!)
  bytes[(nbits + (K-1))/8] = 0; // because nbits is always round 8
}

// depends: POLYS, RATE, COMPUTETYPE
static void encode (/*const*/ unsigned char *bytes, COMPUTETYPE *symbols, int nbits) {
  int polys[RATE] = POLYS;
  int sr = 0;

  // FIXME: this is slowish
  // -- remember about the padding!
  for(int i = 0; i < nbits+(K-1); i++) {
    int b = bytes[i/8];
    int j = i % 8;
    int bit = (b >> (7-j)) & 1;

    sr = (sr << 1) | bit;
    for(int k = 0; k < RATE; k++)
      *(symbols++) = parity(sr & polys[k]);
  }
}

static void noise (COMPUTETYPE *symbols, double gain, int nbits) {
  /* convert 0-1 symbols to 0..255 symbols with noise */
  for(int i = 0; i < (nbits+(K-1))*RATE; i++) {
    symbols[i] = addnoise(symbols[i], gain);
  }
}


static int count_errors(const unsigned char *a, const unsigned char *b, int nbits) {
  /* Count errors */
  int errcnt = 0;
  for(int i=0; i < nbits/8; i++) {
    errcnt+= Bitcnt[a[i] ^ b[i]];
  }
  return errcnt;
}

static int ber_trial(struct v *vp, int nbits, int ntrials, double ebn0) {
  unsigned char *bytes;
  unsigned char *data;
  COMPUTETYPE *symbols;

  if (allocate((void **)&bytes, (void **)&data, (void **)&symbols, nbits)) {
    return -1;
  }

  /* Es/No in dB */
  double esn0 = ebn0 + 10*log10(1./((double)RATE));

  /* Compute noise voltage. The 0.5 factor accounts for BPSK seeing
   * only half the noise power, and the sqrt() converts power to
   * voltage.
   */
  double gain = 1./sqrt(0.5/pow(10.,esn0/10.));

  printf("noise gain = %g\n", gain);

  int tot_errs = 0;
  int badframes = 0;

  srandom(0);

  for (int tr = 0; tr < ntrials; ++tr) {
    fillrandom(bytes, nbits);
    encode(bytes, symbols, nbits);
    noise(symbols, gain, nbits);
    viterbi_init(vp, 0);
    viterbi_decode(vp, symbols, data, nbits);
    int errcnt = count_errors(bytes, data, nbits);
    // counting errors over multiple frames
    tot_errs+= errcnt;
    if(errcnt != 0)
      badframes++;

    LOG("BER %lld/%lld (%10.3g) FER %d/%d (%10.3g)\r",
            tot_errs,(long long)nbits*(tr+1),tot_errs/((double)nbits*(tr+1)),
            badframes,tr+1,(double)badframes/(tr+1));
    fflush(stderr);
  }
  LOG("\n");

  return 0;
}

static int time_trial(struct v *vp, int nbits, int ntrials, double ebn0) {
  unsigned char *bytes;
  unsigned char *data;
  COMPUTETYPE *symbols;

  if (allocate((void **)&bytes, (void **)&data, (void **)&symbols, nbits)) {
    return -1;
  }

  double esn0 = ebn0 + 10*log10(1./((double)RATE));
  double gain = 1./sqrt(0.5/pow(10.,esn0/10.));

  srandom(0);
  fillrandom(bytes, nbits);
  encode(bytes, symbols, nbits);
  noise(symbols, gain, nbits);

  // run time trials
  struct rusage start, finish;
  double extime;

  getrusage(RUSAGE_SELF,&start);
  for (int tr = 0; tr < ntrials; ++tr) {
    viterbi_init(vp, 0);
    viterbi_decode(vp, symbols, data, nbits);
  }
  getrusage(RUSAGE_SELF,&finish);
  extime = finish.ru_utime.tv_sec - start.ru_utime.tv_sec + 1e-6*(finish.ru_utime.tv_usec - start.ru_utime.tv_usec);
  LOG("%.2f sec\n", extime);
  LOG("decoder speed: %g kbits/s\n", ntrials*nbits/extime*1e-3);
  return 0;
}

int main(int argc, const char * argv[]) {
  verbose();

  double ebn0 = EBN0;
  int trials = TRIALS;
  int framebits = FRAMEBITS;
  if (argc > 1) {
    ebn0 = atof(argv[1]);
    if (argc > 2) {
      framebits = atoi(argv[2]);
      if (argc > 3) {
        trials = atoi(argv[3]);
      }
    }
  }
  LOG("Testing at EbN0 %g\n", ebn0);

  struct v *vp;

  vp = viterbi_alloc(framebits);
  if (vp == NULL) return -1;

  viterbi_generic(vp);
  //if (ber_trial(vp, framebits, trials, ebn0)) return -1;
  //if (time_trial(vp, FRAMEBITS, TRIALS, EBN0)) return -1;

  viterbi_spiral(vp);
  if (ber_trial(vp, framebits, trials, ebn0)) return -1;
  if (time_trial(vp, framebits, trials, ebn0)) return -1;

  return 0;
}


