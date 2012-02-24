/*
 * Copyright 2010 Szymon Jakubczak
 * Copyright Phil Karn, KA9Q
 * Copyright 2010 Project Spiral
 */

#include "defs.h"

#if __cplusplus
extern "C" {
#endif

/* State info for instance of Viterbi decoder */
struct v;
/**
 * Create a new instance of a Viterbi decoder
 * @param len = FRAMEBITS (unpadded! data bits)
 */
struct v *viterbi_alloc(int len);

/**
 * Destroy instance of a Viterbi decoder
 */
void viterbi_free(struct v *vp);

/**
 * Initialize decoder for start of new frame
 */
void viterbi_init(struct v *vp, int starting_state);

/**
 * Decode one frame worth of data
 * NOTE: nbits has to match what was passed to viterbi_alloc(...)
 * FIXME: store nbits in struct v?
 */
void viterbi_decode(struct v *vp, const COMPUTETYPE *symbols, unsigned char *data, int nbits);

// set the viterbi decoder to use a specific implementation
extern void viterbi_generic(struct v *vp);
extern void viterbi_spiral(struct v *vp);

#if __cplusplus
};  // extern "C"
#endif
