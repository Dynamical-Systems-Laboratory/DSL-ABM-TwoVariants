#ifndef FOUR_PART_FUNCTION_H
#define FOUR_PART_FUNCTION_H

#include "common.h"

/***************************************************** 
 * class: FourPartFunction 
 * 
 * Stores and provides a dependence on a variable
 * characterized by two potentially different, 
 * consecutive linear increases, plateau, 
 * and a linear decrease  
 * 
 *****************************************************/

class FourPartFunction {
public:

	FourPartFunction() = default;

	/**
	 * \brief Create from five points 
     *
	 * @param Pairs of (t,y) points for each critical part of the function
	 * @param offset - for the independent variable, added to all ts 
	 */ 
	FourPartFunction(const std::vector<std::vector<double>>& vec, 
					 const double offset):
		t0(vec.at(0).at(0) + offset), y0(vec.at(0).at(1)),
		t1(vec.at(1).at(0) + offset), y1(vec.at(1).at(1)),
		t2(vec.at(2).at(0) + offset), y2(vec.at(2).at(1)),
		t3(vec.at(3).at(0) + offset), y3(vec.at(3).at(1)),
		t4(vec.at(4).at(0) + offset), y4(vec.at(4).at(1))
		{ setup_properties(); }

	/**	 
     * \brief Set to a constant value valid at all times 
	 *
	 * @param val - value that will be always returned 
	 */ 
	FourPartFunction(const double val): s_inc_1(0), i_inc_1(val),
			s_inc_2(0), i_inc_2(val),
			s_dec(0), i_dec(val), y2(val) { } 

	/// Value of the function at t
	double operator()(const double t) { return y_value(t); }	
private:
	/// Calculate slopes and intercepts
	void setup_properties();
	/// Implementation of the function with three regions
	double y_value(const double t) const;
	// Points - t is the independent variable
	double t0 = 0, t1 = 0, t2 = 0, t3 = 0, t4 = 0;
	double y0 = 0, y1 = 0, y2 = 0, y3 = 0, y4 = 0;
	// Slopes and intercepts of increase and decrease regions
	double s_inc_1 = 0, i_inc_1 = 0, s_inc_2 = 0, i_inc_2 = 0;
	double s_dec = 0, i_dec = 0;
};

#endif
