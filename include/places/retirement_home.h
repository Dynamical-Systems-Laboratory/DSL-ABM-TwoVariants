#ifndef RETIREMENT_HOME_H
#define RETIREMENT_HOME_H

#include "place.h"

class Place;

/***************************************************** 
 * class: RetirementHome 
 * 
 * Defines and stores attributes of a single 
 * retirement home 
 * 
 *****************************************************/

class RetirementHome : public Place{
public:

	//
	// Constructors
	//

	/**
	 * \brief Creates a RetirementHome object with default attributes
	 */
	RetirementHome() = default;

	/**
	 * \brief Creates a RetirementHome object
	 * \details Retirement home with custom ID, location, and infection parameters
	 *
	 * @param rh_ID - ID of the home
	 * @param xi - x coordinate of the home
	 * @param yi - y coordinate of the home
	 * @param severity_cor - severity correction for symptomatic
	 * @param psi_e - employee absenteeism correction
	 * @param strain_no - number of strains
	 */
	RetirementHome(const int rh_ID, const double xi, const double yi, 
			const double severity_cor, const double psi_e, const int strain_no) : 
		psi_emp(psi_e), Place(rh_ID, xi, yi, severity_cor, strain_no) { } 

	//
	// Infection related computations
	//

	/** 
	 *  \brief Include exposed employee contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_exposed_employee(const double inf_var, const double beta_j, 
			const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*beta_j; }

	/** 
	 *  \brief Include symptomatic employee contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_symptomatic_employee(const double inf_var, const double beta_j, 
			const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*ck*beta_j*psi_emp; }

	/** 
	 *  \brief Include contribution of a symptomatic, home isolated agent in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_symptomatic_home_isolated(const double inf_var, const double beta_j, 
			const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*ck*beta_j;}

	/** 
	 *  \brief Include contribution of an exposed, home isolated agent in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_exposed_home_isolated(const double inf_var, const double beta_j, 
			const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*beta_j; }
	
	//
	// Getters
	//

	double get_absenteeism_correction() const { return psi_emp; }

	//
 	// I/O
	//

	/**
	 * \brief Save information about a RetirementHome object
	 * \details Saves to a file, everything but detailed agent 
	 * 		information; order is ID | x | y | number of agents | 
	 * 		ck | psis 
	 * 		Delimiter is a space.
	 * @param where - output stream
	 */
	void print_basic(std::ostream& where) const override;

private:
	// Absenteeism correction - employee
	double psi_emp = 0.0;
};

#endif
