#pragma once

namespace SurfaceMesh{

class NoiseGenerator{
public:
    /// @brief generate a sample of a specific gaussian distribution using the Box-Muller transform.
    ///        See http://en.wikipedia.org/wiki/Box-Muller_transform
    /// @param mean the mean of the distribution
    /// @param stddev the standard deviation of the distribution
    /// @return a sample of this distribution
    /// @note Stole from (GPL) https://code.google.com/p/nll/source/browse/trunk/nll/math-distribution-gaussian.h
    static inline double randn( const double mean, const double stddev ){
       double u1 = ( static_cast<double> ( rand() ) + 1 ) / ( (double)RAND_MAX + 1 );
       double u2 = ( static_cast<double> ( rand() ) + 1 ) / ( (double)RAND_MAX + 1 );
       assert( -2 * log( u1 ) >= 0 );
       double t1 = sqrt( -2 * log( u1 ) ) * cos( 2 * M_PI * u2 );
       return mean + stddev * t1;
    }
};

}
