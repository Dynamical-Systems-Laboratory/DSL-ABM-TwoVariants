#ifndef VACCINATIONS_H
#define VACCINATIONS_H

#include <forward_list>
#include "common.h"
#include "utils.h"
#include "./io_operations/abm_io.h"
#include "./io_operations/load_parameters.h"
#include "agent.h"
#include "infection.h"
#include "three_part_function.h"
#include "four_part_function.h"

/***************************************************** 
 * class: Vaccinations
 * 
 * Operations related to vaccinating, choice 
 * and setup of newly vaccinated agents 
 * 
 ******************************************************/

class Vaccinations {

public:

	Vaccinations() = default;
	
	/**
	 * \brief Creates a Vaccinations object with custom attributes
	 *
	 * @param infile - name of the file with the input parameters
	 * @param data_dir - directory with tabulated vaccine properties (accessible path with / at the end)
	 */
	Vaccinations(const std::string& infile, const std::string& data_dir) : input_file(infile) 
		{ load_vaccination_parameters(input_file, data_dir); use_offsets_from_file = false; } 

	/**
	 * \brief Creates a Vaccinations object with custom attributes and custom time offsets for initialization
	 *
	 * @param infile - name of the file with the input parameters
	 * @param data_dir - directory with tabulated vaccine properties (accessible path with / at the end)
	 * @param offset_file - file with time offsets
	 * @param infection - object governing all random ops
	 */
	Vaccinations(const std::string& infile, const std::string& data_dir, const std::string& offset_file, Infection& infection) : input_file(infile) 
		{ load_vaccination_parameters(input_file, data_dir); 
		  load_and_shuffle_time_offsets(offset_file, infection); use_offsets_from_file = true; }

	/**
	 * \brief Creates a Vaccinations object with custom attributes and custom time offsets for initialization
	 *
	 * @param infile - name of the file with the input parameters
	 * @param data_dir - directory with tabulated vaccine properties (accessible path with / at the end)
	 * @param offset_file - file with time offsets
	 * @param offset_file_boosters - file with time offsets for boosters
	 * @param infection - object governing all random ops
	 */
	Vaccinations(const std::string& infile, const std::string& data_dir, const std::string& offset_file, Infection& infection, const std::string& offset_file_boosters) : input_file(infile) 
		{ load_vaccination_parameters(input_file, data_dir); 
		  load_and_shuffle_time_offsets(offset_file, offset_file_boosters, infection); use_offsets_from_file = true; }


	/**
	 * \brief Randomly vaccinates requested number of agents
	 * \details Randomly selects eligible agents, vaccinates them 
	 *			and performs all the necessary setup operations;
     *			Returns how many agents were eligible
     *			for vaccination;
     *
	 * @param agents - vector of Agent objects (individual agents) 
	 * @param n_vac - number of agents to vaccinate 
	 * @param n_boost - number of booster doses to distribute
	 * @param infection_parameters - map of property-value pairs
	 * @param time - current time
	 *
	 * @returns the number of vaccinated agents 
	 */
	std::vector<int> vaccinate_random(std::vector<Agent>& agents, int n_vac, 
							int n_boost, Infection& infection, const double time);
	/**
	 * \brief Randomly vaccinates requested number of agents with a negative time offset
	 * \details Randomly selects eligible agents, vaccinates them 
	 *			and performs all the necessary setup operations;
     *			Returns how many agents were eligible
     *			for vaccination; the time offset can be negative, it is in the 
	 * 			past compared to the time specified in the call; the offset 
	 * 			range is determined by the settings in vaccination parameters.
     *
	 * @param agents - vector of Agent objects (individual agents) 
	 * @param n_vac - number of agents to vaccinate
	 * @param n_boost - number of booster doses to distribute 
	 * @param infection_parameters - map of property-value pairs
	 * @param time - current time
	 *
	 * @returns the number of vaccinated agents 
	 */
	int vaccinate_random_time_offset(std::vector<Agent>& agents, int n_vac, 
							const int n_boost, Infection& infection, const double time);

	/**
	 * \brief Vaccinates requested number of agents belonging to a specific group
	 * \details Selects eligible agents from the group, vaccinates them 
	 *			and performs all the necessary setup operations;
     *			Returns how many agents were eligible
     *			for vaccination;
     *
	 * @param agents - vector of Agent objects (individual agents) 
	 * @param group_name - tag representing the name of the group to vaccinate
	 * @param n_vac - number of agents to vaccinate 
	 * @param infection_parameters - map of property-value pairs
	 * @param time - current time
	 * @param vaccinate_all (optional) - true if vaccinate all eligible members of that group
	 *
	 * @returns the number of vaccinated agents 
	 */
	int vaccinate_group(std::vector<Agent>& agents, const std::string& group_name, 
						const int n_vac, Infection& infection, const double time, 
						const bool vaccinate_all = false);
	
	/// Get const reference to vaccination properties (for testing)
	const std::map<std::string, std::map<std::string, std::vector<std::vector<double>>>>& get_vaccination_data() const { return vac_types_properties; }

	/// Const reference to vaccination parameter map
	const std::map<std::string, double>& get_vaccination_parameters() const 
		{ return vaccination_parameters; }

	/// Returns maximum number of agents currently eligible for vaccination
	int max_eligible_random(const std::vector<Agent>& agents);

	/// Returns maximum number of agents in a group currently eligible for vaccination
	int max_eligible_group(const std::vector<Agent>& agents, const std::string& group_name);

	/// Minimum age to get vaccinated
	int get_min_vac_age() const { return vaccination_parameters.at("Minimum vaccination age"); }
	/// Vaccinates agents with provided IDs and sets all the agent properties
	void vaccinate_and_setup(std::vector<Agent>& agents, std::vector<int>& agent_IDs,
								Infection& infection, const double time);
	/// Vaccinates agents with provided IDs and sets all the agent properties while applying a negative time offset
	void vaccinate_and_setup_time_offset(std::vector<Agent>& agents, std::vector<int>& agent_IDs, 
										Infection& infection, const double time, const int n_boosted);
private:

	// Path to the file with vaccination parameters
	std::string input_file;
	// Map of property tag - property value pairs
	std::map<std::string, double> vaccination_parameters;
	// Map of vaccination types and probabilities of receiving them
	// The probabilities for subtypes of each general type are assumed to be a CDF
	// General types are "one dose" and "two doses", and subtypes are the specific
	// vaccination variants (e.g. two doses type 1 can be Moderna, and type 2 Pheizer) 
	std::map<std::string, std::vector<double>> vac_types_probs;
	// Outer maps are vaccination types, inner are properties altered by
	// vaccinating in an extent that correspond to each type
	std::map<std::string, std::map<std::string, std::vector<std::vector<double>>>> vac_types_properties;
	// Vector with time offsets
	std::vector<double> time_offsets;
	// Vector with time offsets (boosters)
	std::vector<double> time_offsets_boosters;
	// Flag to use those and not uniform
	bool use_offsets_from_file = false;
	// ID of the strain for which this vaccine was developed (valid ID starts with 1)
	int strain_id = 0;
	// Total number of strains
	int num_strains = 0;
	// Reduction factors for benefits with respect to each other strain
	std::vector<std::map<std::string, double>> other_strains;

	/// Load parameters related to vaccinations store in a map
	void load_vaccination_parameters(const std::string&, const std::string&);

	/// Create a parameter entry in vac_types_properties for another strain
	void add_other_strain(const std::string&, const std::string&, const std::map<std::string, double>&);

	/// Load and shuffle time custom offsets 
	void load_and_shuffle_time_offsets(const std::string&, Infection&);

	/// Load and shuffle time custom offsets with boosters 
	void load_and_shuffle_time_offsets(const std::string&, const std::string&, Infection&);

	/// Copy parameters in lst into nested vec 
	void copy_vaccination_dependencies(std::forward_list<double>&& lst,
			std::vector<std::vector<double>>& vec);

	/**
	 * \brief Select agents eligible for vaccination based on criteria valid for all agents
	 *
	 * @param agents - all the agents in the simulation
	 * @param max_boost - number of agents that are eligible for booster shots 
	 *
     * @returns - vector of IDs of eligible agents (index in the vector is ID-1)
	 */
	std::vector<int> filter_general(const std::vector<Agent>& agents, int& max_boost);
	
	/**
	 * \brief Select agents in a given group eligible for vaccination based on criteria valid for all agents
	 *
	 * @param agents - all the agents in the simulation
	 * @param infection_parameters - map of property-value pairs
	 * @param group_name - tag representing the name of the group to vaccinate
	 *
     * @returns - vector of IDs of eligible agents (index in the vector is ID-1)
	 */
	std::vector<int> filter_general_and_group(const std::vector<Agent>& agents, 
						const std::string& group_name);
		
	/// True if agent meets core criteria for vaccination eligibility
	bool check_general(const Agent& agent, int& max_boost);	
	
	/// True if agent is in the target vaccination group 
	bool check_group(const Agent& agent, const std::string& group_name);

	/// Assign benefits for this strain and other relevant strains for one dose vaccine
	void set_regular_one_dose(Agent& agent, const std::string& tag, const double time);
	
	/// Assign benefits for this strain and other relevant strains for two dose vaccine
	void set_regular_two_dose(Agent& agent, const std::string& tag, const double time);

	/// Assign benefits for a booster that consider other relevant strains 
	void set_booster(Agent& agent, const std::string& tag, const double time, 
								const double next_step, const double max_end, const double tot_end);
};

#endif
