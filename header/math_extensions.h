#pragma once


#include <stdlib.h>
#include <limits.h>

// custom definitions
const float ERROR_EPSILON = 1e-4;

// returns a random float between 0 and 1.
inline float random_float() {
#if defined ( _WIN64 )
    unsigned int n;
    return static_cast<float>( n ) / ( static_cast<double>( UINT_MAX ) + 1.0 );
#else
    return drand48();
#endif

}