#include "../include/four_part_function.h"

/***************************************************** 
 * class: FourPartFunction 
 * 
 * Stores and provides a dependence on a variable
 * characterized by two potentially different, 
 * consecutive linear increases, plateau, 
 * and a linear decrease  
 * 
 *****************************************************/

// Create from four points
void FourPartFunction::setup_properties()
{
	// Compute slopes and intercepts
	s_inc_1 = (y1-y0)/(t1-t0);
	i_inc_1 = -s_inc_1*t0+y0;
	s_inc_2 = (y2-y1)/(t2-t1);
	i_inc_2 = -s_inc_2*t1+y1;
	s_dec = (y4-y3)/(t4-t3);
	i_dec = -s_dec*t3+y3;
}

// Implementation of the function with four regions
double FourPartFunction::y_value(const double t) const
{
	if ( t < t1 ) { 
		// First part of the linear increase
		return (s_inc_1*t + i_inc_1);
	} else if ( (t >= t1) && (t < t2)) {
		// Second part of the linear increase
		return (s_inc_2*t + i_inc_2);
	} else if ( t > t3 ) {
		// Linear decrease
		return (y3 > y4 ? std::max(y4, (s_dec*t + i_dec)) : std::min(y4, (s_dec*t + i_dec)));
	} else {
		// Plateau
		return y2;
	}
}
