#ifndef DATA_MANAGEMENT_INTERFACE_H
#define DATA_MANAGEMENT_INTERFACE_H

#include "dmi_include.h"

/***************************************************** 
 *  class: DataManagementInterface
 * 
 *  Class that stores, manages, and outputs
 *  agent-based model components: locations and agents, 
 *  and information collected during the simulation. 
 *
 *  This class is not standalone, it needs to be 
 *  initialized through the ABM interface following 
 *  proper model intialization procedures.  
 *
 *  Functionality of this class is inheritted by ABM
 * 	interface and should be accessed through ABM too.
 * 
 ******************************************************/

class DataManagementInterface {
public:

	DataManagementInterface() { }

	//
	// Standard data collection
	//

	/// Retrieve number of infected agents at this time step
	int get_num_infected() const;
	/// Retrieve number of infected agents at this time step
	std::vector<int> get_num_infected_strains(const int) const;
	/// Retrieve number of exposed
	int get_num_exposed() const;
	/// Number of infected - confirmed
	int get_num_active_cases() const;
	/// Number of agents currently undergoing each treatment
	/// @returns Vector with number of agents that are currently 
	/// home isolated | hospitalized | hospitalized in an ICU
	std::vector<int> get_treatment_data() const;
	
	/// Retrieve number of total infected
	int get_total_infected() const { return n_infected_tot; }
	/// Retrieve number of total infected with each strain
	std::vector<int> get_total_infected_strain() const { return n_infected_tot_strain; }
	/// Retrieve number of total dead 
	int get_total_dead() const { return n_dead_tot; }
	/// Retrieve number of dead that were tested 
	int get_tested_dead() const { return n_dead_tested; }
	/// Retrieve number of dead that were not tested
	int get_not_tested_dead() const { return n_dead_not_tested; }
	/// Retrieve number of total recovered
	int get_total_recovered() const { return n_recovered_tot; }
	/// Retrieve cumulative number of exposed that never developed symptoms
	int get_tot_recovering_exposed() { return n_recovering_exposed; }

	// Total tested and confirmed COVID-19 + false positives 
	int get_total_tested() const { return tot_tested; }
	int get_total_tested_positive() const { return tot_tested_pos; }
	int get_total_tested_negative() const { return tot_tested_neg; }
	int get_total_tested_false_positive() const { return tot_tested_false_pos; }
	int get_total_tested_false_negative() const { return tot_tested_false_neg; }
	int get_total_vaccinated() const { return total_vaccinated; }
	// Daily statistics
	const std::vector<int> get_infected_day() const { return n_infected_day; }
	const std::vector<int> get_dead_day() const { return n_dead_day; }
	const std::vector<int> get_recovered_day() const { return n_recovered_day; }
	const std::vector<int> get_tested_day() const { return tested_day; }
	const std::vector<int> get_tested_positive_day() const { return tested_pos_day; }	  
	const std::vector<int> get_tested_negative_day() const { return tested_neg_day; }
	const std::vector<int> get_tested_false_positive_day() const { return tested_false_pos_day; }
	const std::vector<int> get_tested_false_negative_day() const { return tested_false_neg_day; }

	//
	// Saving simulation state
	//

	/**
	 * \brief Save current household information 
	 * \details Outputs household information as 
	 * 		household ID, x and y location, total number
	 * 		of agents, number of infected agents
	 * 		One line per household		
	 * @param filename - path of the file to print to
	*/
	void print_households(const std::string filename) const;	

	/**
	 * \brief Save current retirement home information 
	 * \details Outputs retirement home information as 
	 * 		school ID, x and y location, total number
	 * 		of agents, number of infected agents
	 * 		One line per institution		
	 * @param filename - path of the file to print to
	*/
	void print_retirement_home(const std::string filename) const;	

	/**
	 * \brief Save current school information 
	 * \details Outputs school information as 
	 * 		school ID, x and y location, total number
	 * 		of agents, number of infected agents
	 * 		One line per school		
	 * @param filename - path of the file to print to
	*/
	void print_schools(const std::string filename) const;	

	/**
	 * \brief Save current workplace information 
	 * \details Outputs workplace information as 
	 * 		workplace ID, x and y location, total number
	 * 		of agents, number of infected agents, ck, 
	 * 		beta, psi, type
	 * 		One line per workplace		
	 * @param filename - path of the file to print to
	 */
	void print_workplaces(const std::string filename) const;	

	/**
	 * \brief Save current hospital information 
	 * \details Outputs hospital information as 
	 * 		hospital ID, x and y location, total number
	 * 		of agents, number of infected agents, ck, betas
	 * 		One line per hospital		
	 * @param filename - path of the file to print to
	 */
	void print_hospitals(const std::string filename) const;

	/**
	 * \brief Save current information on leisure locations 
	 * \details Outputs leisure location information as 
	 * 		location ID, x and y coordinates, total number
	 * 		of agents, number of infected agents, ck, beta, type
	 * 		One line per leisure location		
	 * @param filename - path of the file to print to
	 */
	void print_leisure_locations(const std::string& filename) const;

	/**
	 * \brief Save current information on transit type 
	 * \details Outputs transit information as 
	 * 		location ID, x and y coordinates, both set to 0.0, 
	 * 		total number of agents, number of infected agents, 
	 * 		ck, beta, type
	 * 		One line per one transit group		
	 * @param filename - path of the file to print to
	 * @param transit_mode - type of transit, currently 'carpool' or 'public'
	 */
	void print_transit(const std::string& filename, const std::string& transit_mode) const;

	/**
	 * \brief Save IDs of all agents in all households
	 * \details One line per household, 0 if no agents present
	 * @param filename - path of the file to save to
	 */
	void print_agents_in_households(const std::string filename) const;
	
	/**
	 * \brief Save IDs of all agents in all retirement homes 
	 * \details One line per retirement home, 0 if no agents present
	 * @param filename - path of the file to save to
	 */
	void print_agents_in_retirement_homes(const std::string filename) const;
	
	/**
	 * \brief Save IDs of all agents in all schools
	 * \details One line per school, 0 if no agents present
	 * @param filename - path of the file to save to
	 */
	void print_agents_in_schools(const std::string filename) const;
	
	/**
	 * \brief Save IDs of all agents in all workplaces
	 * \details One line per workplace, 0 if no agents present
	 * @param filename - path of the file to save to
	 */
	void print_agents_in_workplaces(const std::string filename) const;

	/**
	 * \brief Save IDs of all agents in all hospitals 
	 * \details One line per hospital, 0 if no agents present
	 * @param filename - path of the file to save to
	 */
	void print_agents_in_hospitals(const std::string filename) const;

	/**
	 * \brief Save current agent information 
	 * \details Outputs agent information as 
	 *		indicated in Agent constructor 
	 * 		One line per agent		
	 * @param filename - path of the file to print to
	*/
	void print_agents(const std::string filename) const;	

	//
	// Functions mainly for testing
	//
	
	/// Return a const reference to a House object vector
	const std::vector<Household>& get_vector_of_households() const { return households; }
	/// Return a const reference to a RetirementHome object vector
	const std::vector<RetirementHome>& get_vector_of_retirement_homes() const { return retirement_homes; }
	/// Return a const reference to a School object vector
	const std::vector<School>& get_vector_of_schools() const { return schools; }
	/// Return a const reference to a Workplace object vector
	const std::vector<Workplace>& get_vector_of_workplaces() const { return workplaces; }
	/// Return a const reference to a Hospital object vector
	const std::vector<Hospital>& get_vector_of_hospitals() const { return hospitals; }
	/// Return a const reference to a vector of carpool objects
	const std::vector<Transit>& get_vector_of_carpools() const { return carpools; }
	/// Return a const reference to a vector of public transit objects 
	const std::vector<Transit>& get_vector_of_public_transit() const { return public_transit; }
	/// Return a const reference to a vector of leisure locations 
	const std::vector<Leisure>& get_vector_of_leisure_locations() const { return leisure_locations; }
	/// Return a const reference to an Agent object vector
	const std::vector<Agent>& get_vector_of_agents() const { return agents; }

	/// Return a non-const reference to an Agent object vector
	std::vector<Agent>& vector_of_agents() { return agents; }
	/// Return a reference to a Hospital object vector
	std::vector<Hospital>& vector_of_hospitals() { return hospitals; }
	/// Return a reference to a Household object vector
	std::vector<Household>& vector_of_households() { return households; }
	/// Return a reference to a RetirementHome object vector
	std::vector<RetirementHome>& vector_of_retirement_homes() { return retirement_homes; }
	/// Return a reference to a School object vector
	std::vector<School>& vector_of_schools() { return schools; }
	/// Return a reference to a Workplace object vector
	std::vector<Workplace>& vector_of_workplaces() { return workplaces; }
	/// Return a reference to a vector of carpool objects
	std::vector<Transit>& vector_of_carpools() { return carpools; }
	/// Return a reference to a vector of public transit objects 
	std::vector<Transit>& vector_of_public_transit() { return public_transit; }
	/// Return a reference to a vector of leisure locations 
	std::vector<Leisure>& vector_of_leisure_locations() { return leisure_locations; }

	/// Return a reference to an Agent object vector
	std::vector<Agent>& get_vector_of_agents_non_const()  { return agents; }
	/// Return a copy of an Agent object vector
	std::vector<Agent> get_copied_vector_of_agents() const { return agents; }
	/// Return a copy of a House object vector
	std::vector<Household> get_copied_vector_of_households() const { return households; }
	/// Return a copy of a RetirementHome object vector
	std::vector<RetirementHome> get_copied_vector_of_retirement_homes() const { return retirement_homes; }
	/// Return a copy of a School object vector
	std::vector<School> get_copied_vector_of_schools() const { return schools; }
	/// Return a copy of a Workplace object vector
	std::vector<Workplace> get_copied_vector_of_workplaces() const { return workplaces; }
	/// Return a copy of a Hospital object vector
	std::vector<Hospital> get_copied_vector_of_hospitals() const { return hospitals; }
	/// Return a copy of a vector of carpool objects
 	std::vector<Transit> get_copied_vector_of_carpools() const { return carpools; }
	/// Return a copy of a vector of public transit objects 
	std::vector<Transit> get_copied_vector_of_public_transit() const { return public_transit; }
	/// Return a copy of a vector of leisure locations 
	std::vector<Leisure> get_copied_vector_of_leisure_locations() const { return leisure_locations; }

	// Virtual dtor - avoid UDB and memory leaks
	virtual ~DataManagementInterface() { }

protected:

	// Vectors of individual model objects
	std::vector<Agent> agents;
	std::vector<Household> households;
	std::vector<RetirementHome> retirement_homes;
	std::vector<School> schools;
	std::vector<Workplace> workplaces;
	std::vector<Hospital> hospitals;
	std::vector<Transit> carpools;
	std::vector<Transit> public_transit;
	std::vector<Leisure> leisure_locations;

	// Disease toll - total 
	int n_infected_tot = 0;
	std::vector<int> n_infected_tot_strain = {};
	int n_dead_tot = 0;
	int n_dead_tested = 0;
	int n_dead_not_tested = 0;
	int n_recovered_tot = 0;
	int n_recovering_exposed = 0;
	// Testing statistics
	int tot_tested = 0;
	int tot_tested_pos = 0;
	int tot_tested_neg = 0;
	int tot_tested_false_pos = 0;
	int tot_tested_false_neg = 0;
	// New cases
	std::vector<int> n_infected_day = {};
	std::vector<int> n_dead_day = {};
	std::vector<int> n_recovered_day = {};
	std::vector<int> tested_day = {};
	std::vector<int> tested_pos_day = {};	
	std::vector<int> tested_neg_day = {};
	std::vector<int> tested_false_pos_day = {};
	std::vector<int> tested_false_neg_day = {};
	// Number of vaccinated
	int total_vaccinated = 0;
	// Number of boosted
	int total_boosted = 0;

	/**
	 * \brief Read object information from a file	
	 * @param filename - path of the file to print to
	 */
	std::vector<std::vector<std::string>> read_object(std::string filename);
	
	/// Print basic places information to a file
	template <typename T>
	void print_places(std::vector<T> places, const std::string fname) const;

	/// Print all agent IDs in a particular type of place to a file
	template <typename T>
	void print_agents_in_places(std::vector<T> places, const std::string fname) const;

};

// Write Place objects
template <typename T>
void DataManagementInterface::print_places(std::vector<T> places, const std::string fname) const
{
	// AbmIO settings
	std::string delim(" ");
	bool sflag = true;
	std::vector<size_t> dims = {0,0,0};	

	// Write data to file
	AbmIO abm_io(fname, delim, sflag, dims);
	abm_io.write_vector<T>(places);
}

// Write agent IDs in Place objects
template <typename T>
void DataManagementInterface::print_agents_in_places(std::vector<T> places, const std::string fname) const
{
	// AbmIO settings
	std::string delim(" ");
	bool sflag = true;
	std::vector<size_t> dims = {0,0,0};	

	// First collect the data into a nested vector
	std::vector<std::vector<int>> agents_all_places;
	for (const auto& place : places){
		std::vector<int> agent_IDs = place.get_agent_IDs();
		// If no agents, store a 0
		if (agent_IDs.empty())
			agents_all_places.push_back({0});
		else
			agents_all_places.push_back(agent_IDs);
	}

	// Then write data to file, one line per place
	AbmIO abm_io(fname, delim, sflag, dims);
	abm_io.write_vector<int>(agents_all_places);
}

#endif
