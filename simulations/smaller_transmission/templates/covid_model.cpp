#include "../../../include/abm.h"
#include <chrono>

int main()
{
	// Time in days, space in km
	double dt = 0.25;
	// Max number of steps to simulate
	int tmax = 360;	
	// Print agent info this many steps
	int dt_out_agents = 1000; 
	// Number of initially infected
	std::vector<int> inf0{39, 1};
	// Number of agents in different stages of COVID-19
	std::vector<int> N_active{339, 0}, N_vac{71285, 0}, 
						N_boost{0, 0};
	int n_recovered = 3463; 
	// Have agents vaccinated already
	bool vaccinate = true;
	// Don't vaccinate in the setup phase to have agents 
	// vaccinated with a time offset
	bool dont_vac = true; 
	// Custom vaccination offset
	const bool custom_vac_offsets = true;
	// Number of strains
	const int n_strains = 2;

	// File with all the input files names
	std::string fin("input_data/input_files_all.txt");

	// Output file names
	// Active at the current step - all, detected and not 
	std::ofstream ftot_inf_cur("output/infected_with_time.txt");
	std::ofstream ftot_inf_cur_strains_1("output/infected_with_time_s1.txt");
	std::ofstream ftot_inf_cur_strains_2("output/infected_with_time_s2.txt");
	// Cumulative
	std::ofstream ftot_inf("output/total_infected.txt");
	std::ofstream ftot_inf_s1("output/total_infected_strain_1.txt");
	std::ofstream ftot_inf_s2("output/total_infected_strain_2.txt");
	std::ofstream ftot_dead("output/dead_with_time.txt");

	// This initializes the core of the model
	ABM abm(dt);
	abm.simulation_setup(fin, inf0, custom_vac_offsets);
	abm.initialize_active_cases(N_active, vaccinate, N_vac, 
									N_boost, n_recovered);	

   	// Data collection
	std::vector<int> active_count(tmax+1);
    std::vector<int> infected_count(tmax+1);
    std::vector<std::vector<int>> infected_strain_count(n_strains, std::vector<int>(tmax+1));
	std::vector<std::vector<int>> active_count_strains(n_strains, std::vector<int>(tmax+1));
    std::vector<int> total_dead(tmax+1);
	std::vector<int> temp_strains(n_strains);

    // For time measurement
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	// Simulation
	for (int ti = 0; ti<=tmax; ++ti){
        // Collect data
		// Active
        active_count.at(ti) = abm.get_num_infected();
		active_count.at(ti) = abm.get_num_infected();
		temp_strains = abm.get_num_infected_strains(n_strains);
		active_count_strains.at(0).at(ti) = temp_strains.at(0);
		active_count_strains.at(1).at(ti) = temp_strains.at(1);
        // Total infected
		infected_count.at(ti) = abm.get_total_infected();
        infected_strain_count.at(0).at(ti) = abm.get_total_infected_strain().at(0);
        infected_strain_count.at(1).at(ti) = abm.get_total_infected_strain().at(1);
  		// Casualties
      	total_dead.at(ti) = abm.get_total_dead();
		// Propagate
		abm.transmit_infection();
	}

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::seconds> (end - begin).count() << "[s]" << std::endl;

	// Should be no vaccines and no boosters after initial
	const std::vector<Agent>& agents = abm.vector_of_agents();
	int boosted = 0, vaxed = 0;
	for (const auto& agent : agents) {
		if (agent.got_booster()) {
			++boosted;
		}
		if (agent.vaccinated()) {
			++vaxed;
		}
	}
	std::cout << "Number of agents with a booster: " << boosted << std::endl;
	std::cout << "Number of vaccinated agents: " << vaxed << std::endl;

    // Totals
    std::copy(active_count.begin(), active_count.end(), std::ostream_iterator<int>(ftot_inf_cur, " "));
    std::copy(active_count_strains.at(0).begin(), active_count_strains.at(0).end(), std::ostream_iterator<int>(ftot_inf_cur_strains_1, " "));
    std::copy(active_count_strains.at(1).begin(), active_count_strains.at(1).end(), std::ostream_iterator<int>(ftot_inf_cur_strains_2, " "));
    std::copy(infected_count.begin(), infected_count.end(), std::ostream_iterator<int>(ftot_inf, " "));
    std::copy(infected_strain_count.at(0).begin(), infected_strain_count.at(0).end(), std::ostream_iterator<int>(ftot_inf_s1, " "));
    std::copy(infected_strain_count.at(1).begin(), infected_strain_count.at(1).end(), std::ostream_iterator<int>(ftot_inf_s2, " "));
    std::copy(total_dead.begin(), total_dead.end(), std::ostream_iterator<int>(ftot_dead, " "));

    // Print total values
    std::cout << "Total number of infected agents: " << abm.get_total_infected() << "\n"
              << "Total number of casualities: " << abm.get_total_dead() << "\n"
              << "Total number of recovered agents: " << abm.get_total_recovered() << "\n";

}
