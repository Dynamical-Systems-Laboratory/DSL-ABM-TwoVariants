#ifndef HOUSEHOLD_H
#define HOUSEHOLD_H

#include "place.h"
#include "../infection.h"

class Place;
class Infection;

/***************************************************** 
 * class: Household
 * 
 * Defines and stores attributes of a single household
 * 
 *****************************************************/

class Household : public Place{
public:

	//
	// Constructors
	//

	/**
	 * \brief Creates a Household object with default attributes
	 */
	Household() = default;

	/**
  	 * \brief Creates a Household object
	 * \details Household with custom ID, location, and infection parameters
	 *
  	 * @param house_ID - ID of the household
  	 * @param xi - x coordinate of the household
  	 * @param yi - y coordinate of the household
  	 * @param alpha_exp - household size correction
  	 * @param severity_cor - severity correction for symptomatic
	 * @param strain_no - number of strains
  	 */
	Household(int house_ID, double xi, double yi, const double alpha_exp, const double severity_cor, const int strain_no) : 
		alpha(alpha_exp), Place(house_ID, xi, yi, severity_cor, strain_no) { } 

	//
 	// I/O
	//

	/**
	 * \brief Save information about a Household object
	 * \details Saves to a file, everything but detailed agent 
	 * 		information; order is ID | x | y | number of agents | 
	 * 		 ck | alpha
	 * 		Delimiter is a space.
	 *
	 * 	@param where - output stream
	 */
	void print_basic(std::ostream& where) const override;

	//
	// Infection related computations
	//

	/**
	 * \brief Calculates and stores probability contribution of infected agents if any 
	 */
	void compute_infected_contribution() override;

	/** 
	 *  \brief Include contribution of a symptomatic, home isolated agent in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_symptomatic_home_isolated(const double inf_var, const double beta_j, 
			const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*ck*beta_j; }

	/** 
	 *  \brief Include contribution of an exposed , home isolated agent in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_exposed_home_isolated(const double inf_var, const double beta_j, 
			const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*beta_j; }

private:
	// Household size scaling factor
	double alpha = 0.0;
};

#endif
