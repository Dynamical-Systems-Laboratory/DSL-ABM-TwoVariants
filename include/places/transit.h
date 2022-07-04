#ifndef TRANSIT_H
#define TRANSIT_H

#include "place.h"

class Place;

/***************************************************** 
 * class: Transit 
 * 
 * Defines and stores attributes of a single transit 
 * mode object 
 * 
 *****************************************************/

class Transit : public Place{
public:

	//
	// Constructors
	//

	/**
	 * \brief Creates a Transit object with default attributes
	 */
	Transit() = default;

	/**
	 * \brief Creates a Transit object 
	 * \details Transit with custom ID, type, and infection parameters
	 *
	 * @param transit_ID - ID of the transit object 
	 * @param severity_cor - severity correction for symptomatic
	 * @param psi - absenteeism correction
	 * @param tr_type - transit type
	 * @param strain_no - number of strains
	 */
	Transit(const int transit_ID, const double severity_cor, const double psi, 
				const std::string& tr_type, const int strain_no) : 
			psi_j(psi), type(tr_type), 
			Place(transit_ID, 0.0, 0.0, severity_cor, strain_no){ }

	//
	// Infection related computations
	//

	/** 
	 *  \brief Include symptomatic contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_symptomatic(const double inf_var, const double beta_j, const int strain_id) override 
		{ lambda_sum.at(strain_id-1) += inf_var*ck*beta_j*psi_j; }

	/** 
	 *  \brief Include symptomatic contribution in the sum with non-default absenteeism correction
	 *	@param inf_var - agent infectiousness variability factor
	 *  @param psi - absenteeism correction for that agent's category
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_special_symptomatic(const double inf_var, const double psi, 
			const double beta_j, const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*ck*beta_j*psi; }

	//
	// Setters
	//
	
	void change_absenteeism_correction(const double val) { psi_j = val; }
	
	//
	// Getters
	//
	
	double get_absenteeism_correction() const { return psi_j; }

	//
 	// I/O
	//

	/**
	 * \brief Save information about a Transit object
	 * \details Saves to a file, everything but detailed agent 
	 * 		information; order is ID | x | y | number of agents | 
	 * 		ck | psi_j | type 
	 * 		Delimiter is a space.
	 * 	@param where - output stream
	 */
	void print_basic(std::ostream& where) const override;

private:
	// Absenteeism correction
	double psi_j = 0.0;
	// Transit type 
	std::string type;
};
#endif
