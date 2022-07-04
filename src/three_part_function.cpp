#include "../include/three_part_function.h"

/***************************************************** 
 * class: ThreePartFunction 
 * 
 * Stores and provides a dependence on a variable
 * characterized by a linear increase, plateau, and
 * a linear decrease  
 * 
 *****************************************************/

// Create from four points
void ThreePartFunction::setup_properties()
{
	// Compute slopes and intercepts
	s_inc = (y1-y0)/(t1-t0);
	i_inc = -s_inc*t0+y0;
	s_dec = (y3-y2)/(t3-t2);
	i_dec = -s_dec*t2+y2;
}

// Implementation of the function with three regions
double ThreePartFunction::y_value(const double t) const
{
	if ( t < t1 ) { 
		// Linear increase
		return (s_inc*t + i_inc); 
	} else if ( t > t2 ) {
		// Linear decrease
		return (y2 > y3 ? std::max(y3, (s_dec*t + i_dec)) : std::min(y3, (s_dec*t + i_dec)));
	} else {
		// Plateau
		return y1;
	}
}
