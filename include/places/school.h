#ifndef SCHOOL_H
#define SCHOOL_H

#include "place.h"

class Place;

/***************************************************** 
 * class: School 
 * 
 * Defines and stores attributes of a single school
 * 
 *****************************************************/

class School : public Place{
public:

	//
	// Constructors
	//

	/**
	 * \brief Creates a School object with default attributes
	 */
	School() = default;

	/**
	 * \brief Creates a School object
	 * \details School with custom ID, location, and infection parameters
	 *
	 * @param school_ID - ID of the school 
	 * @param xi - x coordinate of the school
	 * @param yi - y coordinate of the school
	 * @param severity_cor - severity correction for symptomatic
	 * @param psi_e - employee absenteeism correction
	 * @param psi - student absenteeism correction
	 * @param strain_no - number of strains
	 */
	School(const int school_ID, const double xi, const double yi, 
			const double severity_cor, const double psi_e, const double psi, 
			const int strain_no) : 
		psi_emp(psi_e), psi_j(psi), Place(school_ID, xi, yi, severity_cor, strain_no){ } 

	//
	// Infection related computations
	//

	/** 
	 *  \brief Include exposed employee contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_emp - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_exposed_employee(const double inf_var, const double beta_emp, 
								const int strain_id)
		{ lambda_sum.at(strain_id-1) += inf_var*beta_emp; }

	/** 
	 *  \brief Include symptomatic employee contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_emp - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_symptomatic_employee(const double inf_var, const double beta_emp, 
				const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*ck*beta_emp*psi_emp; }

	/** 
	 *  \brief Include symptomatic student contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_symptomatic_student(const double inf_var, const double beta_j, 
				const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*ck*beta_j*psi_j; }

	//
	// Getters
	//
	
	double get_absenteeism_correction() const { return psi_emp; }

	//
 	// I/O
	//

	/**
	 * \brief Save information about a School object
	 * \details Saves to a file, everything but detailed agent 
	 * 		information; order is ID | x | y | number of agents | 
	 * 		ck | psis 
	 * 		Delimiter is a space.
	 * @param where - output stream
	 */
	void print_basic(std::ostream& where) const override;

private:
	// Absenteeism correction - student and employee
	double psi_j = 0.0;
	double psi_emp = 0.0;
};

#endif
