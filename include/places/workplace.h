#ifndef WORKPLACE_H
#define WORKPLACE_H

#include "place.h"

class Place;

/***************************************************** 
 * class: Workplace 
 * 
 * Defines and stores attributes of a single workplace
 * 
 *****************************************************/

class Workplace : public Place{
public:

	//
	// Constructors
	//

	/**
	 * \brief Creates a Workplace object with default attributes
	 */
	Workplace() = default;

	/**
	 * \brief Creates a Workplace object 
	 * \details Workplace with custom ID, location, and infection parameters
	 *
	 * @param work_ID - ID of the workplace
	 * @param xi - x coordinate of the workplace 
	 * @param yi - y coordinate of the workplace
	 * @param severity_cor - severity correction for symptomatic
	 * @param psi - absenteeism correction
	 * @param wtype - workplace type
	 * @param strain_no - number of strains
	 */
	Workplace(const int work_ID, const double xi, const double yi,
			 const double severity_cor, const double psi, 
			const std::string wtype, const int strain_no) : 
			psi_j(psi), type(wtype), 
			Place(work_ID, xi, yi, severity_cor, strain_no)
				{ frac_inf_out.resize(strain_no); }

	//
	// Infection related computations
	//

	/** 
	 *  \brief Include symptomatic contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_symptomatic(const double inf_var, const double beta_j, 
			const int strain_id) override 
		{ lambda_sum.at(strain_id-1) += inf_var*ck*beta_j*psi_j; }

	
	/// Calculates and stores probability contribution of infected agents if any 
	void compute_infected_contribution() override;

	//
	// Setters
	//
	
	void change_absenteeism_correction(const double val) { psi_j = val; }
	
	/// Assign fraction of infected at a workplace outside the modeled town
	void set_outside_infected(const double val, const int strain_id) 
		{ frac_inf_out.at(strain_id-1) = val; }

	/// Separate modification due to closures
	//void adjust_outside_infected(const double mod) { frac_inf_out *= mod; }

	//
	// Getters
	//
	
	double get_absenteeism_correction() const { return psi_j; }
	
	/// True if the workplace is outside current town
	bool outside_town() const override 
		{ return (type == "outside")?(true) : (false); }

	/// Fraction of infected for a workplace outside the modeled town
	//double get_outside_infected() const { return frac_inf_out; }

	/// Occupation type of this workplace
	std::string get_type() const override { return type; } 

	//
 	// I/O
	//

	/**
	 * \brief Save information about a Workplace object
	 * \details Saves to a file, everything but detailed agent 
	 * 		information; order is ID | x | y | number of agents | 
	 * 		ck | psi_j 
	 * 		Delimiter is a space.
	 * 	@param where - output stream
	 */
	void print_basic(std::ostream& where) const override;

private:
	// Absenteeism correction
	double psi_j = 0.0;
	// Workplace type 
	std::string type = "none";
	// Fraction of infected in an outside workplace
	std::vector<double> frac_inf_out;
};
#endif
