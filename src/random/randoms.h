#ifndef RANDOMS_H_
#define RANDOMS_H_

#include <random>
#include "Vector3.h"

extern double erand48(unsigned short xseed[3]);

namespace mei
{

    //! Generate a random float in [0, 1)
    Float randomFloat();

}//namespace

#endif
