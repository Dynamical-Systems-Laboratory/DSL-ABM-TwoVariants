#include "../include/data_management_interface.h"

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

//
// Standard data collection
//


// Retrieve number of infected agents at this time step
int DataManagementInterface::get_num_infected() const
{
	int infected_count = 0;
	for (const auto& agent : agents){
		if (agent.infected())
			++infected_count;
	}
	return infected_count;
}

// Retrieve number of infected agents at this time step
std::vector<int> DataManagementInterface::get_num_infected_strains(const int n_strains) const
{
	std::vector<int> infected_count(n_strains, 0);
	for (const auto& agent : agents){
		if (agent.infected()) {
			++infected_count.at(agent.get_strain()-1);
		}
	}
	return infected_count;
}

// Retrieve number of exposed agents at this time step
int DataManagementInterface::get_num_exposed() const
{
	int exposed_count = 0;
	for (const auto& agent : agents){
		if (agent.exposed())
			++exposed_count;
	}
	return exposed_count;
}

// Number of infected - confirmed
int DataManagementInterface::get_num_active_cases() const
{
	int active_count = 0;
	for (const auto& agent : agents){
		if ((agent.infected() && agent.tested_covid_positive())
			 || (agent.symptomatic_non_covid() && agent.home_isolated()
					 && agent.tested_false_positive())){
			++active_count;
		}
	}
	return active_count;
}

// Number of agents currently undergoing each treatment 
std::vector<int> DataManagementInterface::get_treatment_data() const
{
	// IH, HN, ICU
	std::vector<int> treatments(3,0);
	for (const auto& agent : agents){
		if (agent.home_isolated()){
			++treatments.at(0);
		}else if (agent.hospitalized()){
			++treatments.at(1);
		}else if (agent.hospitalized_ICU()){
			++treatments.at(2);
		}
	}
	return treatments;
}

//
// I/O
//

// General function for reading an object from a file
std::vector<std::vector<std::string>> DataManagementInterface::read_object(std::string fname)
{
	// AbmIO settings
	std::string delim(" ");
	bool sflag = true;
	std::vector<size_t> dims = {0,0,0};

	// 2D vector with a parsed line in each inner
	// vector
	std::vector<std::vector<std::string>> obj_vec_2D;

	// Read and return a copy
	AbmIO abm_io(fname, delim, sflag, dims);
	obj_vec_2D = abm_io.read_vector<std::string>();

	return obj_vec_2D;
}

//
// Saving simulation state
//

// Save current household information to file 
void DataManagementInterface::print_households(const std::string fname) const
{
	print_places<Household>(households, fname);
}	

// Save current school information to file 
void DataManagementInterface::print_schools(const std::string fname) const
{
	print_places<School>(schools, fname);
}


// Save current workplaces information to file 
void DataManagementInterface::print_workplaces(const std::string fname) const
{
	print_places<Workplace>(workplaces, fname);
}

// Save current hospital information to file 
void DataManagementInterface::print_hospitals(const std::string fname) const
{
	print_places<Hospital>(hospitals, fname);
}

// Save current retirement home information to file 
void DataManagementInterface::print_retirement_home(const std::string fname) const
{
	print_places<RetirementHome>(retirement_homes, fname);
}

// Save current information on leisure locations to file 
void DataManagementInterface::print_leisure_locations(const std::string& fname) const
{
	print_places<Leisure>(leisure_locations, fname);
}

// Save current information on transit type 
void DataManagementInterface::print_transit(const std::string& fname, const std::string& mode) const
{
	if (mode == "carpool") {
		print_places<Transit>(carpools, fname);
	} else if (mode == "public") {
		print_places<Transit>(public_transit, fname);
	} else {
		throw std::invalid_argument("Wrong type of transit to print. Acceptable types: carpool or public");
	}	
}

// Save IDs of all agents in all households
void DataManagementInterface::print_agents_in_households(const std::string filename) const
{
	print_agents_in_places<Household>(households, filename);
}

// Save IDs of all agents in all schools
void DataManagementInterface::print_agents_in_schools(const std::string filename) const
{
	print_agents_in_places<School>(schools, filename);
}

// Save IDs of all agents in all workplaces 
void DataManagementInterface::print_agents_in_workplaces(const std::string filename) const
{
	print_agents_in_places<Workplace>(workplaces, filename);
}

// Save IDs of all agents in all hospitals 
void DataManagementInterface::print_agents_in_hospitals(const std::string filename) const
{
	print_agents_in_places<Hospital>(hospitals, filename);
}

// Save current agent information to file 
void DataManagementInterface::print_agents(const std::string fname) const
{
	// AbmIO settings
	std::string delim(" ");
	bool sflag = true;
	std::vector<size_t> dims = {0,0,0};	

	// Write data to file
	AbmIO abm_io(fname, delim, sflag, dims);
	abm_io.write_vector<Agent>(agents);	
}

