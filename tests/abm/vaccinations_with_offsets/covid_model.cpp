#include "../../../include/abm.h"
#include <chrono>

/***************************************************** 
 *
 * ABM run of COVID-19 SEIR in New Rochelle, NY 
 *
 ******************************************************/

int main()
{
	// Time in days, space in km
	double dt = 0.25;
	// Max number of steps to simulate
	int tmax = 1;	
	// Print agent info this many steps
	int dt_out_agents = 100; 
	// Number of initially infected
	int inf0 = 4;
	// Number of agents in different stages of COVID-19
	int N_active = 66, N_vac = 51342;
	// Have agents vaccinated already
	bool vaccinate = true;
	// Don't vaccinate in the setup phase to have agents 
	// vaccinated with a time offset
	bool dont_vac = true; 
	// Custom vaccination offset
	const bool custom_vac_offsets = true;

	// File with all the input files names
	std::string fin("input_data/input_files_all_vac_reopen.txt");

	// Output file names
	// Active at the current step - all, detected and not 
	std::ofstream ftot_inf_cur("output/infected_with_time.txt");
	// Cumulative
	std::ofstream ftot_inf("output/total_infected.txt");
	std::ofstream ftot_dead("output/dead_with_time.txt");

	// This initializes the core of the model
	ABM abm(dt);
	abm.simulation_setup(fin, inf0, custom_vac_offsets);

	// Initialization for vaccination/reopening studies
	abm.initialize_vac_and_reopening(dont_vac);
	// Create a COVID-19 population with previously vaccinated at random times
	abm.initialize_active_cases(N_active, vaccinate, N_vac);	

	// Save offsets for checking
	const std::vector<Agent>& agents = abm.vector_of_agents();
	std::vector<double> offsets;
	for (const auto& agent : agents) {
		offsets.push_back(agent.get_vac_time_offset());
	}

	// Write to file
	// Unused options of the AbmIO object
	std::string delim(" ");
	bool one_file = true;
	std::vector<size_t> dims = {0,0,0};

	AbmIO io("all_offsets.txt", delim, one_file, dims);

	io.write_vector<double>(offsets); 
}
