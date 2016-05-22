/* 
 * prng.h
 * 
 * TODO: blurb
 * 
 * NOTE: THIS SOFTWARE IS UNFIT FOR CRYPTOGRAPHIC PURPOSES!
 * 
 * The *_r functions with externalized state information allow for 
 * reentrant, thread-safe PRN generation, when fed with the address 
 * of a suitable (thread-local) object of type random_ctx_t. 
 */
 
#ifndef PRNG_H_INCLUDED
#define PRNG_H_INCLUDED

#include <limits.h>
#include <stdint.h>

/*
 * RANDOM_MAX is the largest possible value returned by random[_r](), 
 * and one more than the largest possible value returned by 
 * random_uni[_r]().
 */
#define RANDOM_MAX  (ULONG_MAX)

/*
 * Constants suitable to initialize objects of type random_ctx_t. 
 */
#define RANDOM_FLEASEED         0xf1ea5eedUL
#define RANDOM_CTX_INITIALIZER  { RANDOM_FLEASEED, 0UL, 0UL, 0UL }

/*
 * Type to hold the PRNG state information.
 */
struct random_ctx_t_struct { 
    uint64_t a, b, c, d; 
};

typedef 
    struct random_ctx_t_struct
    random_ctx_t;

/* 
 * Generate a pseudorandom number in the range 0 <= n <= RANDOM_MAX,
 * that is, pick from the closed interval [0;RANDOM_MAX].
 */
unsigned long random( void );
unsigned long random_r( random_ctx_t *ctx );

/* 
 * Initialize the pseudorandom number generator with a given seed.
 */
void srandom( unsigned long seed );
void srandom_r( random_ctx_t *ctx, unsigned long seed );

/*
 * Returns an unbiased PRN from the half-open interval [0;upper[, 
 * hence the largest possible return value is RANDOM_MAX-1. 
 * NOTE: Use random[_r]() to pick from the interval [0;RANDOM_MAX].
 * Returns 0 for upper < 2.
 */
unsigned long random_uni( unsigned long upper );
unsigned long random_uni_r( random_ctx_t *ctx, unsigned long upper );

#endif  //ndef PRNG_H_INCLUDED

/* EOF */
