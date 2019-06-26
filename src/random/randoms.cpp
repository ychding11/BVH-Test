#include <math.h> 
#include <stdlib.h>
#include <stdio.h>  
#include <omp.h>
#include <vector>
#include <random>

#include "randoms.h"

namespace mei
{
	static std::default_random_engine generator;
	static std::uniform_real_distribution<Float> distr(0.0, 1.0);

	uint32_t wang_hash(uint32_t seed)
	{
		seed = (seed ^ 61) ^ (seed >> 16);
		seed *= 9;
		seed = seed ^ (seed >> 4);
		seed *= 0x27d4eb2d;
		seed = seed ^ (seed >> 15);
		return seed;
	}

	// Xorshift algorithm from George Marsaglia's paper
	static uint32_t rng_state;
	uint32_t rand_xorshift()
	{
		rng_state ^= (rng_state << 13);
		rng_state ^= (rng_state >> 17);
		rng_state ^= (rng_state << 5);
		return rng_state;
	}

#if 0
	//! Generate a random float in [0, 1)
	Float randomFloat()
	{
		return Float(rand_xorshift()) * (1.0 / 4294967296.0);
	}
#else
	Float randomFloat()
	{
		return distr(generator);
	}
#endif


}//namespace


