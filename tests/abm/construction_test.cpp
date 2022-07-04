#include "abm_tests.h"

/***************************************************** 
 *
 * Test suite for creation of ABM objects
 * using the more concise abm.simulation_setup(fname)
 * approach to create the core componets
 *
 ******************************************************/

// Tests
bool create_households_test();
bool create_schools_test();
bool create_workplaces_test();
bool create_hospitals_test();
bool create_retirement_homes_test();
bool create_leisure_locations_test();
bool create_carpools_test();
bool create_public_transit_test();
bool create_agents_test();
bool wrong_number_of_initially_infected_test();

// Supporting functions
bool compare_places_files(std::string fname_in, std::string fname_out, 
				const std::vector<double> infection_parameters, const bool has_type = false, 
				const bool has_extra = false, const bool has_print_type = false);
bool compare_workplaces_files(std::string fname_in, std::string fname_out, 
				const std::map<std::string, double>& infection_parameters);
bool compare_transit_files(std::string fname_in, std::string fname_out, 
				const std::vector<double> infection_parameters);
bool correctly_registered(const ABM, const std::vector<std::vector<int>>, 
							const std::vector<std::vector<int>>, std::string, 
							const std::string, const std::string, const int);
bool check_initially_infected(const Agent& agent, const Flu& flu, int& n_exposed_never_sy,
								const std::map<std::string, double> infection_parameters);
bool check_fractions(int, int, double, std::string, bool need_ge = false);

int main()
{
	test_pass(create_households_test(), "Household creation");
	test_pass(create_schools_test(), "School creation");
	test_pass(create_workplaces_test(), "Workplace creation");
	test_pass(create_hospitals_test(), "Hospitals creation");
	test_pass(create_retirement_homes_test(), "Retirement homes creation");
	test_pass(create_leisure_locations_test(), "Leisure locations creation");
	test_pass(create_carpools_test(), "Carpools creation");
	test_pass(create_public_transit_test(), "Public transit creation");
	test_pass(create_agents_test(), "Agent creation");
	test_pass(wrong_number_of_initially_infected_test(), "Too many initially infected");
}

// Checks household creation from file
bool create_households_test()
{
	// Model parameters and output
	double dt = 2.0;
	std::string fin("test_data/input_files_all.txt");
	std::string fout("test_data/houses_out.txt");
	std::string ftest("test_data/NR_households.txt");
	std::vector<int> initially_infected{0, 5, 100};

	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);

	abm.print_households(fout);
	// Vector of infection parameters 
	// (order as in output file: ck, alpha)
	std::vector<double> infection_parameters = {2.0, 0.8}; 
	// Check if correct, hardcoded for places properties
	if (!compare_places_files(ftest, fout, infection_parameters)){
		std::cerr << "Error in household creation" << std::endl;
		return false;
	}
	return true;
}

// Checks school creation from file
bool create_schools_test()
{
	// Model parameters and output
	double dt = 1.0;
	std::string fin("test_data/input_files_all.txt");
	std::string fout("test_data/schools_out.txt");
	std::string ftest("test_data/NR_schools.txt");
	std::vector<int> initially_infected{0, 5, 100};

	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);
	
	abm.print_schools(fout);
	bool has_type = true;
	// Vector of infection parameters 
	// (order as in output file: ck, psi for employee, 
	// psi for middle)
	std::vector<double> infection_parameters = {2.0, 0.2, 0.1}; 
	// Check if correct, hardcoded for places properties
	if (!compare_places_files(ftest, fout, infection_parameters, has_type)){
		std::cerr << "Error in school creation" << std::endl;
		return false;
	}
	return true;
}

// Checks workplace creation from file
bool create_workplaces_test()
{
	// Model parameters and output
	double dt = 2.0;
	std::string fin("test_data/input_files_all.txt");
	std::string fout("test_data/workplaces_out.txt");
	std::string ftest("test_data/NR_workplaces.txt");
	std::vector<int> initially_infected{0, 5, 100};
	
	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);

	abm.print_workplaces(fout);

 	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters(); 
	// Check if correct, hardcoded for places properties
	if (!compare_workplaces_files(ftest, fout, infection_parameters)){
		std::cerr << "Error in workplace creation" << std::endl;
		return false;
	}
	return true;	
}

// Checks hospital creation from file
bool create_hospitals_test()
{
	// Model parameters and output
	double dt = 2.0;
	std::string fin("test_data/input_files_all.txt");
	std::string fout("test_data/hospitals_out.txt");
	std::string ftest("test_data/NR_hospitals.txt");
	std::vector<int> initially_infected{0, 5, 100};

	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);

	abm.print_hospitals(fout);
	// Vector of infection parameters 
	// (order as in output file: ck, betas for each category)
	std::vector<double> infection_parameters = {2.0}; 
	// Check if correct, hardcoded for places properties
	if (!compare_places_files(ftest, fout, infection_parameters)){
		std::cerr << "Error in hospital creation" << std::endl;
		return false;
	}
	return true;	
}

// Checks retirement home creation
bool create_retirement_homes_test()
{
	// Model parameters and output
	double dt = 2.0;
	std::string fin("test_data/input_files_all.txt");
	std::string fout("test_data/ret_homes_out.txt");
	std::string ftest("test_data/NR_retirement_homes.txt");
	std::vector<int> initially_infected{0, 5, 100};

	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);

	abm.print_retirement_home(fout);
	// Vector of infection parameters 
	// (order as in output file: ck, psi employee)
	std::vector<double> infection_parameters = {2.0, 0.0}; 
	// Check if correct, hardcoded for places properties
	if (!compare_places_files(ftest, fout, infection_parameters)){
		std::cerr << "Error in retirement home creation" << std::endl;
		return false;
	}
	return true;
}

// Checks creation of leisure locations from file
bool create_leisure_locations_test()
{
	// Model parameters and output
	double dt = 2.0;
	std::string fin("test_data/input_files_all.txt");
	std::string fout("test_data/leisure_out.txt");
	std::string ftest("test_data/NR_leisure.txt");
	std::vector<int> initially_infected{0, 5, 100};

	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);

	abm.print_leisure_locations(fout);
	// Vector of infection parameters 
	// (order as in output file: ck)
	std::vector<double> infection_parameters = {2.0}; 
	// Check if correct, hardcoded for places properties
	if (!compare_places_files(ftest, fout, infection_parameters, true, false, true)){
		std::cerr << "Error in leisure location creation" << std::endl;
		return false;
	}
	return true;	
}

// Checks creation of carpools from file
bool create_carpools_test()
{
	// Model parameters and output
	double dt = 2.0;
	std::string fin("test_data/input_files_all.txt");
	std::string fout("test_data/carpool_out.txt");
	std::string ftest("test_data/NR_carpool.txt");
	std::vector<int> initially_infected{0, 5, 100};
	
	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);

	abm.print_transit(fout, "carpool");
	// Vector of infection parameters 
	// (order as in output file: ck, psi_j)
	std::vector<double> infection_parameters = {2.0, 0.1}; 
	// Check if correct, hardcoded for places properties
	if (!compare_transit_files(ftest, fout, infection_parameters)){
		std::cerr << "Error in carpools creation" << std::endl;
		return false;
	}
	return true;	
}

// Checks creation of public transit from file
bool create_public_transit_test()
{
	// Model parameters and output
	double dt = 2.0;
	std::string fin("test_data/input_files_all.txt");
	std::string fout("test_data/public_out.txt");
	std::string ftest("test_data/NR_public.txt");
	std::vector<int> initially_infected{0, 5, 100};
	
	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);
	
	abm.print_transit(fout, "public");
	// Vector of infection parameters 
	// (order as in output file: ck)
	std::vector<double> infection_parameters = {2.0, 0.1}; 
	// Check if correct, hardcoded for places properties
	if (!compare_transit_files(ftest, fout, infection_parameters)){
		std::cerr << "Error in public transit creation" << std::endl;
		return false;
	}
	return true;	
}

// Checks agents creation from file including proper 
// distribution into places 
bool create_agents_test()
{
	// Model parameters and output
	double dt = 0.25;
	int n_exposed_never_sy = 0;
	std::vector<int> initially_infected{0, 5, 100};
	std::vector<int> actually_infected(initially_infected.size(), 0.0);	

	std::string fin("test_data/input_files_all.txt");
	std::string fout("test_data/agents_out.txt");

	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);
	
	// Acceptable transit modes to work
	std::vector<std::string> all_travel_modes = {"car", "carpool", "public", "walk",
													"other", "wfh", "None"};
	// Collect for testing
	const std::vector<Agent>& agents = abm.get_vector_of_agents_non_const();
	const std::vector<Household>& households = abm.get_vector_of_households();
	const std::vector<School>& schools = abm.get_vector_of_schools();
	const std::vector<Workplace>& workplaces = abm.get_vector_of_workplaces();
	const std::vector<Hospital>& hospitals = abm.get_vector_of_hospitals();
	const std::vector<RetirementHome>& retirement_homes = abm.get_vector_of_retirement_homes();
	const std::vector<Transit>& carpools = abm.get_vector_of_carpools();
	const std::vector<Transit>& public_transit = abm.get_vector_of_public_transit();
	const std::vector<Leisure>& leisure = abm.get_vector_of_leisure_locations();

	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters(); 
	Flu& flu = abm.get_flu_object();

	// Check registration and count initially infected as created by ABM
	for (const auto& agent : agents){ 
		const int aID = agent.get_ID();
		const std::string agent_transit = agent.get_work_travel_mode();
		if (std::find(all_travel_modes.begin(), all_travel_modes.end(), agent_transit)
						== all_travel_modes.end()) {
			std::cerr << "Invalid travel mode" << std::endl;
			return false;
		}
		if ((!agent.works()) && (!agent.hospital_employee())){
			if ((agent_transit != "None") || 
				!(float_equality<double>(agent.get_work_travel_time(), 0.0,  1e-5))) {
				std::cerr << "Agent that is not employed has non-zero travel time or invalid travel mode" << std::endl;
				return false;
			}
		}
		if (agent.infected()){
			++actually_infected.at(agent.get_strain()-1);
			if (!check_initially_infected(agent, flu, n_exposed_never_sy, infection_parameters)){
				std::cerr << "Error in initialization of infected agent" << std::endl;
				return false;	
			}	
		} 
		if (agent.student()){
			if (!find_in_place<School>(schools, aID, agent.get_school_ID())){
				std::cerr << "Agent not registered in a school" << std::endl;
				return false;
			}
		}

		// Works from home settings
		if (agent.works_from_home()) {
			if ((!agent.works()) 
							|| !(float_equality<double>(agent.get_work_travel_time(), 0.0,  1e-5))
							|| (agent.get_work_travel_mode() != "wfh")) {
				std::cerr << "Agent that works from home has invalid properties" << std::endl;
				return false;
			}
		}

		if (agent.works() && !agent.works_from_home()){
			if ((agent.get_work_travel_time() <= 0.0) || (agent_transit  == "None")
						|| (agent_transit  == "wfh")) {
				std::cerr << "Agent that works has invalid travel-related properties" << std::endl;
				return false;
			}
			if (agent.school_employee()){
				if (!find_in_place<School>(schools, aID, agent.get_work_ID())){
					std::cerr << "Agent not registered in a school as an employee" << std::endl;
					return false;
				}
			}else if (agent.retirement_home_employee()){
				if (!find_in_place<RetirementHome>(retirement_homes, aID, agent.get_work_ID())){
					std::cerr << "Agent not registered in a retirement home as an employee" << std::endl;
					return false;
				}		
			} else {
				if (!find_in_place<Workplace>(workplaces, aID, agent.get_work_ID())){
					std::cerr << "Agent not registered in a workplace" << std::endl;
					return false;
				}
			}
		}

		// Household or retirement home
		if (agent.retirement_home_resident()){
			if (!find_in_place<RetirementHome>(retirement_homes, aID, agent.get_household_ID())){
				std::cerr << "Agent not registered in a retirement home" << std::endl;
				return false;
			}	
		} else if (agent.hospital_non_covid_patient()){
			if (!find_in_place<Hospital>(hospitals, aID, agent.get_hospital_ID())){
				std::cerr << "Hospital patient without covid is not registered in a hospital" << std::endl;
				return false;
			}
		} else {
			if (!find_in_place<Household>(households, aID, agent.get_household_ID())){
				std::cerr << "Agent not registered in a household" << std::endl;
				return false;
			}
		}
		// Hospital employee
		if (agent.hospital_employee()){
			if (!find_in_place<Hospital>(hospitals, aID, agent.get_hospital_ID())){
				std::cerr << "Hospital employee is not registered in a hospital" << std::endl;
				return false;
			}
			if ((agent.get_work_travel_time() <= 0.0) || (agent_transit  == "None")
						|| (agent_transit  == "wfh")) {
				std::cerr << "Agent that works in a hospital has invalid travel-related properties" << std::endl;
				return false;
			}
		}

		// Transit
		if (agent.get_work_travel_mode() == "carpool") {
			if (!find_in_place<Transit>(carpools, aID, agent.get_carpool_ID())){
				std::cerr << "Agent not registered in a carpool" << std::endl;
				return false;
			}
		}
		if (agent.get_work_travel_mode() == "public") {
			if (!find_in_place<Transit>(public_transit, aID, agent.get_public_transit_ID())){
				std::cerr << "Agent not registered in a public transit" << std::endl;
				return false;
			}
		}
	}
	if (initially_infected != actually_infected) {
		std::cerr << "Initially infected agent numbers don't match expectations" << std::endl;
		return false;
	}
	return true;
}

// Test suite for agents that are infected at intialization
bool check_initially_infected(const Agent& agent, const Flu& flu, int& n_exposed_never_sy,
								const std::map<std::string, double> infection_parameters)
{
	if (!agent.exposed() || agent.tested() || agent.home_isolated()){
		std::cerr << "Initially infected agent should be exposed and not tested" << std::endl;
		return false;
	}
	// Flu
	const int aID = agent.get_ID();
	const std::vector<int> flu_susceptible = flu.get_susceptible_IDs();
	for (const auto& afs : flu_susceptible){
		if (afs == aID){
			std::cerr << "Infected agent should not be part of future flu poll" << std::endl; 
			return false;
		}
	}
	const std::vector<int> flu_agents = flu.get_flu_IDs();
	for (const auto& afs : flu_agents){
		if (afs == aID){
			std::cerr << "Infected agent should not be part of flu group" << std::endl; 
			return false;
		}
	}
	if (agent.recovering_exposed()){
		++n_exposed_never_sy;
		// Recovery time 
		if (agent.get_latency_end_time() < infection_parameters.at("recovery time")){
			std::cerr << "Wrong latency of agent that will not develop symptoms" << std::endl; 
			//return false;
		}
	}
	// Infectiousness variability
	if (agent.get_inf_variability_factor() < 0){
		std::cerr << "Agent's infectiousness variability out of range: " << agent.get_inf_variability_factor() << std::endl;
		//return false;
	}
	// Latency and non-infectious period
	if ((agent.get_infectiousness_start_time() > agent.get_latency_end_time()) || 
			(agent.get_latency_end_time() < 0)){
		std::cerr << "Agent's non-infectious period or latency out of range" << std::endl;
		//return false;
	}
	return true;
}

/// \brief Compare input places file with output from a Place object 
bool compare_places_files(std::string fname_in, std::string fname_out, 
				const std::vector<double> infection_parameters, 
				const bool has_type, const bool has_extra, const bool prints_type)
{
	std::ifstream input(fname_in);
	std::ifstream output(fname_out);

	// Load the first file as vectors
	std::vector<int> in_IDs;
	std::vector<double> in_coords_x, in_coords_y;

	int ID = 0;
	double x = 0.0, y = 0.0;
	std::string type = {}, out_type = {};
	std::vector<std::string> vec_types = {}; 
	int extra = 0;
	if (has_type == false){
		while (input >> ID >> x >> y){
			in_IDs.push_back(ID);
			in_coords_x.push_back(x);
			in_coords_y.push_back(y);
		}
	} else if (has_extra == false){
		// For places that have types
		while (input >> ID >> x >> y >> type){
			in_IDs.push_back(ID);
			in_coords_x.push_back(x);
			in_coords_y.push_back(y);
			vec_types.push_back(type);
		}
	} else if (has_extra == true){
		// For places that have types and an 
		// additional column 
		while (input >> ID >> x >> y >> type >> extra){
			in_IDs.push_back(ID);
			in_coords_x.push_back(x);
			in_coords_y.push_back(y);
			vec_types.push_back(type);
		}
	}

	// Now load the second (output) and compare
	// Also check if total number of agents and infected agents is 0
	int num_ag = 0;
	int num_ag_exp = 0;
	int ind = 0;
	double parameter;

	while (output >> ID >> x >> y >> num_ag){
		// Compare ID, location, agents
		if (ID != in_IDs.at(ind)){
			std::cerr << "Wrong place ID" << std::endl;
			return false;
		}
		if (!float_equality<double>(x, in_coords_x.at(ind), 1e-5)){
			std::cerr << "Wrong x coordinates" << std::endl;
			return false;
		}
		if (!float_equality<double>(y, in_coords_y.at(ind), 1e-5)){
			std::cerr << "Wrong y coordinates" << std::endl;
			return false;
		}
		if (num_ag < 0){
			std::cerr << "Number of agents should be zero or higher" << std::endl;
			return false;
		}
		// Compare infection parameters
		for (auto const& expected_parameter : infection_parameters){
			output >> parameter;
			if (!float_equality<double>(expected_parameter, parameter, 1e-5)){
				std::cerr << "Wrong infection transmission parameter" << std::endl;
				return false;
			}
		}
		if ((prints_type == true) and (has_type == true)) {
			output >> out_type;
			if (out_type != vec_types.at(ind)) {
				std::cerr << "Object types do not match" << std::endl;
				return false;
			}		
		} else if ((prints_type == true) and (has_type == false)) {
			output >> out_type;
			// Place holder of sort
			if (out_type != "None") {
				std::cerr << "Object types do not match" << std::endl;
				return false;
			}
		}
		++ind;
	}

	// In case file empty, shorter, or doesn't exist
	if (in_IDs.size() != ind){
		std::cerr << "Wrong number of locations" << std::endl;
		return false;
	}
	return true;	
}

/// \brief Compare input workplace file with output from a Workplace object 
bool compare_workplaces_files(std::string fname_in, std::string fname_out, 
				const std::map<std::string, double>& infection_parameters)
{
	std::ifstream input(fname_in);
	std::ifstream output(fname_out);

	// Load the first file as vectors
	std::vector<int> in_IDs;
	std::vector<double> in_coords_x, in_coords_y;

	int ID = 0;
	double x = 0.0, y = 0.0;
	std::string type = {}, out_type = {};
	std::vector<std::string> vec_types = {}; 
	int extra = 0;
	// For places that have types and an 
	// additional column 
	while (input >> ID >> x >> y >> type >> extra){
		in_IDs.push_back(ID);
		in_coords_x.push_back(x);
		in_coords_y.push_back(y);
		vec_types.push_back(type);
	}
	// Now load the second (output) and compare
	// Also check if total number of agents and infected agents is 0
	int num_ag = 0, num_inf = 0;
	int num_ag_exp = 0, num_inf_exp = 0;
	int ind = 0;
	double parameter;

	while (output >> ID >> x >> y >> num_ag){
		// Compare ID, location, agents
		if (ID != in_IDs.at(ind)){
			std::cerr << "Wrong place ID" << std::endl;
			return false;
		}
		if (!float_equality<double>(x, in_coords_x.at(ind), 1e-5)){
			std::cerr << "Wrong x coordinates" << std::endl;
			return false;
		}
		if (!float_equality<double>(y, in_coords_y.at(ind), 1e-5)){
			std::cerr << "Wrong y coordinates" << std::endl;
			return false;
		}
		// Compare infection parameters
		// ck
		output >> parameter;
		if (!float_equality<double>(infection_parameters.at("severity correction"), parameter, 1e-5)){
			std::cerr << "Wrong severity correction" << std::endl;
			return false;
		}
		// psi
		output >> parameter;
		if (!float_equality<double>(infection_parameters.at("work absenteeism correction"), parameter, 1e-5)){
			std::cerr << "Wrong absenteism correction" << std::endl;
			return false;
		}
		output >> out_type;
		if (out_type != vec_types.at(ind)) {
			std::cerr << "Object types do not match" << std::endl;
			return false;
		}		
		++ind;
	}

	// In case file empty, shorter, or doesn't exist
	if (in_IDs.size() != ind){
		std::cerr << "Wrong number of locations" << std::endl;
		return false;
	}
	return true;	
}

/// \brief Compare input transit file with output from a Place/Transit object 
bool compare_transit_files(std::string fname_in, std::string fname_out, 
				const std::vector<double> infection_parameters)
{
	std::ifstream input(fname_in);
	std::ifstream output(fname_out);

	// Load the first file as vectors
	std::vector<int> in_IDs;
	double in_coords_x = 0.0, in_coords_y = 0.0;
	std::vector<std::string> in_types = {}; 

	int ID = 0;
	double x = 0.0, y = 0.0;
	std::string type = {}, out_type;
	double extra_1 = 0, extra_2 = 0;
	while (input >> ID >> type >> extra_1 >> extra_2){
			in_IDs.push_back(ID);
			in_types.push_back(type);
	}

	// Now load the second (output) and compare
	// Also check if total number of agents and infected agents is 0
	int num_ag = 0, num_inf = 0;
	int num_ag_exp = 0, num_inf_exp = 0;
	int ind = 0;
	double parameter;

	while (output >> ID >> x >> y >> num_ag){
		// Compare ID, location, agents
		if (ID != in_IDs.at(ind)){
			std::cerr << "Wrong place ID" << std::endl;
			return false;
		}
		if (!float_equality<double>(x, in_coords_x, 1e-5)){
			std::cerr << "Wrong x coordinates" << std::endl;
			return false;
		}
		if (!float_equality<double>(y, in_coords_y, 1e-5)){
			std::cerr << "Wrong y coordinates" << std::endl;
			return false;
		}
		if (num_ag < 0){
			std::cerr << "Number of agents should be zero or higher" << std::endl;
			return false;
		}
		// Compare infection parameters
		for (auto const& expected_parameter : infection_parameters){
			output >> parameter;
			if (!float_equality<double>(expected_parameter, parameter, 1e-5)){
				std::cerr << "Wrong infection transmission parameter" << std::endl;
				return false;
			}
		}
		// Type	
		output >> out_type;
		if (out_type != in_types.at(ind)) {
			std::cerr << "Object types do not match" << std::endl;
			return false;
		}		
		++ind;
	}

	// In case file empty, shorter, or doesn't exist
	if (in_IDs.size() != ind){
		std::cerr << "Wrong number of transit objects" << std::endl;
		return false;
	}
	return true;	
}

/**
 * \brief Check if agents registered correctly in a given Place
 *
 * @param abm - an ABM object
 * @param place_info - vector of expected total number of agents in each place and total infected
 * @param place_agents - vector with IDs of agents in each place
 * @param place_type - type of place (house, school or work), case independent
 * @param info_file - name of the file to save basic info output to 
 * @param agent_file - name of the file to save agent IDs to
 * @param num_red_args - number of redundant arguments (i.e. infection paramters) in printed
 */ 
bool correctly_registered(const ABM abm, const std::vector<std::vector<int>> place_info, 
							const std::vector<std::vector<int>> place_agents, std::string place_type, 
							const std::string info_file, const std::string agent_file, const int num_red_args)
{

	// Save basic info and agent IDs
	place_type = str_to_lower(place_type);
	if (place_type == "house"){
		abm.print_households(info_file);
		abm.print_agents_in_households(agent_file);
	} else if (place_type == "school"){
		abm.print_schools(info_file);
		abm.print_agents_in_schools(agent_file);
	} else if (place_type == "workplace"){
		abm.print_workplaces(info_file);
		abm.print_agents_in_workplaces(agent_file);
	} else if (place_type == "hospital"){
		abm.print_hospitals(info_file);
		abm.print_agents_in_hospitals(agent_file);
	} else{
		std::cout << "Wrong place type" << std::endl;
		return false; 
	}
	
	// Check if total number of agents and infected agents are correct
	std::ifstream info_total(info_file);
	int ID = 0, num_agents = 0, num_inf = 0;
	double x = 0.0, y = 0.0, not_needed_arg = 0.0;
	int ind = 0;
	while (info_total >> ID >> x >> y >> num_agents >> num_inf){
		// Ignore remaining 
		for (int i=0; i<num_red_args; ++i)
			info_total >> not_needed_arg;

		if (num_agents != place_info.at(ind).at(0))
			return false;
		if (num_inf != place_info.at(ind).at(1))
			return false;		
		++ind;
	}

	// Check if correct agent IDs
	// Load ID file into a nested vector, one inner vector per place
	std::vector<std::vector<int>> saved_IDs;
	std::ifstream info_IDs(agent_file);
	std::string line;
	while (std::getline(info_IDs, line))
	{	
		std::istringstream iss(line);
		std::vector<int> place_IDs;
		while (iss >> ID)
			place_IDs.push_back(ID);
		saved_IDs.push_back(place_IDs);
	}

	// Compare the vectors
	return is_equal_exact<int>(place_agents, saved_IDs);
}

/// Check if num1/num2 is roughly equal to expected
bool check_fractions(int num1, int num2, double fr_expected, std::string msg, bool need_ge)
{
	double fr_tested = static_cast<double>(num1)/static_cast<double>(num2);
	if (need_ge) {
		// Only need greater or equal 
		if (fr_tested < fr_expected){
			std::cout << msg << std::endl;
			std::cout << "Computed: " << fr_tested << " Expected: " << fr_expected << std::endl; 
			return false;
		}
	} else {
		if (!float_equality<double>(fr_tested, fr_expected, 0.1)){
			std::cout << msg << std::endl;
			std::cout << "Computed: " << fr_tested << " Expected: " << fr_expected << std::endl; 
			return false;
		}
	}
	return true;
}

/// Tests if the program stops when initially infected exceed number of agents
bool wrong_number_of_initially_infected_test()
{
	// Model parameters and output
	double dt = 1.0;
	std::string fin("test_data/input_files_all.txt");
	std::vector<int> initially_infected(5, 0);
	// Error detection settings
	bool verbose = true;
	const std::runtime_error rtime("Too many initially infected");

	ABM abm_1st(dt);
	try {
		initially_infected.at(0) = 80001;
		abm_1st.simulation_setup(fin, initially_infected);
		if (verbose) {
			print_msg("No exception occurred");
		}
		return false;
	} catch (const std::exception& e) {
		if (verbose) {
			print_msg(e.what());
		}
		if (typeid(rtime) != typeid(e)) {
			std::cerr << "Initially infected with the first strain exceeding total number of " 
						<< "agents not recognized as an error" << std::endl;
			return false;
		}
	}

	ABM abm_middle(dt);
	try {
		initially_infected.at(0) = 80000;
		initially_infected.at(2) = 1;
		abm_middle.simulation_setup(fin, initially_infected);
		if (verbose) {
			print_msg("No exception occurred");
		}
		return false;
	} catch (const std::exception& e) {
		if (verbose) {
			print_msg(e.what());
		}
		if (typeid(rtime) != typeid(e)) {
			std::cerr << "Initially infected with a middle strain exceeding total number of " 
						<< "agents not recognized as an error" << std::endl;
			return false;
		}
	}

	ABM abm_last(dt);
	try {
		initially_infected.at(0) = 79999;
		initially_infected.at(4) = 1;
		abm_last.simulation_setup(fin, initially_infected);
		if (verbose) {
			print_msg("No exception occurred");
		}
		return false;
	} catch (const std::exception& e) {
		if (verbose) {
			print_msg(e.what());
		}
		if (typeid(rtime) != typeid(e)) {
			std::cerr << "Initially infected with the last strain exceeding total number of " 
						<< "agents not recognized as an error" << std::endl;
			return false;
		}
	}

	return true;
}








