#ifndef HOSPITAL_H
#define HOSPITAL_H

#include "place.h"

class Place;

/***************************************************** 
 * class: Hospital 
 * 
 * Defines and stores attributes of a single hospital 
 * 
 *****************************************************/

class Hospital : public Place{
public:

	//
	// Constructors
	//

	/**
	 * \brief Creates a Hospital object with default attributes
	 */
	Hospital() = default;

	/**
	 * \brief Creates a Hospital object 
	 * \details Hospital with custom ID, location, and infection parameters
	 *
	 * @param hospital_ID - ID of the hospital 
	 * @param xi - x coordinate of the hospital 
	 * @param yi - y coordinate of the hospital
	 * @param severity_cor - severity correction for symptomatic
	 * @param strain_no - number of strains
	 */
	Hospital(const int hospital_ID, const double xi, const double yi,
			 const double severity_cor, const int strain_no) : 
			Place(hospital_ID, xi, yi, severity_cor, strain_no) { }

	//
	// Infection related computations
	//
	
	/** 
	 *  \brief Include exposed employee contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_exposed(const double inf_var, const double beta_j, 
			const int strain_id) override 
		{ lambda_sum.at(strain_id-1) += inf_var*beta_j; }

	/** 
	 *  \brief Include exposed contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
 	 */
	void add_exposed_patient(const double inf_var, const double beta_j, 
			const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*beta_j; }

	/** 
	 *  \brief Include symptomatic  contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_symptomatic_patient(const double inf_var, const double beta_j, 
			const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*ck*beta_j; }

	/** 
	 *  \brief Include tested at hospital contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_hospital_tested(const double inf_var, const double beta_j, 
			const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*ck*beta_j; }

	/** 
	 *  \brief Include tested at hospital contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_exposed_hospital_tested(const double inf_var, const double beta_j, 
			const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*beta_j; }

	/** 
	 *  \brief Include hospitalized contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_hospitalized(const double inf_var, const double beta_j, 
			const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*ck*beta_j; }

	/** 
	 *  \brief Include hospitalized in ICU contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	void add_hospitalized_ICU(const double inf_var, const double beta_j, 
			const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*ck*beta_j; }

	/// \brief Increase number of tested at that time step
	void increase_total_tested() { n_tested++; }

  	/// \brief Reset select variables of a place after transmission step
    void reset_contributions() override
 		{ std::fill(lambda_sum.begin(), lambda_sum.end(), 0.0); 
			std::fill(lambda_tot.begin(), lambda_tot.end(), 0.0); n_tested = 0;}

	/// \brief Contribution takes into account agents tested at current step
	void compute_infected_contribution() override;

	/// \brief Returns number of Flu agents being tested in a hospital at that step
	int get_n_tested() const { return n_tested; }

	/// \brief Returns the summation 
	std::vector<double> get_lambda_sum() const { return lambda_sum; }

	//
 	// I/O
	//

	/**
	 * \brief Save information about a Hospital object
	 * \details Saves to a file, everything but detailed agent 
	 * 		information; order is ID | x | y | number of agents | 
	 * 		 ck  
	 * 		Delimiter is a space.
	 * 	@param where - output stream
	 */
	void print_basic(std::ostream& where) const override;

private:
	// Number of agents being tested at a given 
	// time step (for infection probability)
	// this is both infected and not
	int n_tested = 0;
};
#endif
