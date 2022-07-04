#ifndef PLACE_H
#define PLACE_H

#include "../common.h"

/***************************************************** 
 * class: Place
 * 
 * Base class that defines a place  
 * 
 *****************************************************/

class Place{
public:

	//
	// Constructors
	//

	/**
	 * \brief Creates a Place object with default attributes
	 */
	Place() = default;

	/**
	 * \brief Creates a Place object 
	 * \details Place with custom ID, location, and infection parameters
	 * 			during creation of agents
	 * @param place_ID - ID of the place
	 * @param xi - x coordinate of the place (latitude)
	 * @param yi - y coordinate of the place (logitude)
	 * @param severity_cor - severity correction for symptomatic
	 * @param strain_no - number of strains
	 */
	Place(const int place_ID, const double xi, const double yi, const double severity_cor,
			const int strain_no) : 
		ID(place_ID), x(xi), y(yi), ck(severity_cor), n_strains(strain_no) 
			{ lambda_sum.resize(n_strains, 0.0);  lambda_tot.resize(n_strains, 0.0);} 

	//
	// Infection related computations
	//
	
	/** 
	 *  \brief Include exposed contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)  
	 */
	virtual void add_exposed(const double inf_var, const double beta_j, const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*beta_j; }

	/** 
	 *  \brief Include symptomatic contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 *	@param beta_j - agent transmission rate for that place
	 *	@param strain_id - agent's strain ID (1, 2, ...)
	 */
	virtual void add_symptomatic(const double inf_var, 
					const double beta_j, const int strain_id) 
		{ lambda_sum.at(strain_id-1) += inf_var*ck*beta_j; }

	/**
	 * \brief Calculates and stores fraction of infected agents if any  
	 */
	virtual void compute_infected_contribution();

	/**
	 *	\brief Reset the lambda sum of a place after transmission step
	 */
	virtual void reset_contributions() 
		{ std::fill(lambda_sum.begin(), lambda_sum.end(), 0.0); 
			std::fill(lambda_tot.begin(), lambda_tot.end(), 0.0); }

	//
	// Getters
	//

	/// Return place ID
	int get_ID() const { return ID; }

	/// Return IDs of agents registered in this place	
	std::vector<int> get_agent_IDs() const { return agent_IDs; }

	/// Return total number of agents
	int get_number_of_agents() const { return agent_IDs.size(); }

	/// Return probability contribution of infected agents
	std::vector<double> get_infected_contribution() const { return lambda_tot; }

	/// Coordinates
	double get_x() const { return x; }
	double get_y() const { return y; }

	/// True if the workplace is outside current town
	virtual bool outside_town() const 
		{ return false; }

	/// Placeholder type 
	virtual std::string get_type() const 
		{ return "none"; }

	//
 	// I/O
	//

	/**
	 * \brief Save information about a Place object
	 * \details Saves to a file, everything but detailed agent 
	 * 		information; order is ID | x | y | number of agents 	 
	 * 		Delimiter is a space.
	 * 	@param where - output stream
	 */
	virtual void print_basic(std::ostream& where) const;

	//
	// Initialization and update
	//

	/**
	 * \brief Assign agent to a given place indicating if infected
	 * @param agent_ID - global ID of the agent
	 */
	void register_agent(const int agent_ID);

	/**
	 * \brief Add a new agent to this place
	 * @param index - agent ID (starts with 1)
	 */
	void add_agent(const int index) { agent_IDs.push_back(index); }

	/**
	 * \brief Remove an agent from this place
	 * @param index - agent ID (starts with 1)
	 */
	void remove_agent(const int index);

	// Virtual destructor
	virtual ~Place() = default;

protected:
	// Place ID
	int ID = -1;
	// Location
	double x = 0.0, y = 0.0;
	// Number of strains
	int n_strains = 0;
	// IDs of agents in this place
	std::vector<int> agent_IDs;
	// Total number of agents
	int num_tot = 0;

	// Sum of agents contributions
	std::vector<double> lambda_sum;	
	// Total contribution to infection probability
	// from this place
	std::vector<double> lambda_tot;

	// Severity correction for symptomatic
	double ck = 0.0;
};

/// Overloaded ostream operator for I/O
std::ostream& operator<< (std::ostream& out, const Place& place);

#endif
