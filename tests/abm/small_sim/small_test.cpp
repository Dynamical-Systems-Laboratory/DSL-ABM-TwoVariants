#include "abm_tests.h"


int main()
{
	// Agents 
	std::string fin("test_data/NR_agents.txt");
	// Places
	std::string hfile("test_data/NR_households.txt");
	std::string sfile("test_data/NR_schools.txt");
	std::string wfile("test_data/NR_workplaces.txt");
	std::string hspfile("test_data/NR_hospitals.txt");
	std::string rh_file("test_data/NR_retirement_homes.txt");
	std::string cp_file("test_data/NR_carpool.txt");
	std::string pt_file("test_data/NR_public.txt");
	std::string ls_file("test_data/NR_leisure.txt");
	std::string fmob_out("test_data/mobility_probs_out.txt");

	// Model parameters
	double dt = 0.25;
	int tmax = 5;

	// File with infection parameters
	std::string pfname("test_data/infection_parameters.txt");
	// Files with age-dependent distributions
	std::string dexp_name("test_data/age_dist_exposed_never_sy.txt");
	std::string dh_name("test_data/age_dist_hospitalization.txt");
	std::string dhicu_name("test_data/age_dist_hosp_ICU.txt");
	std::string dmort_name("test_data/age_dist_mortality.txt");
	// Map for abm loading of distrinutions
	std::map<std::string, std::string> dfiles = 
		{ {"exposed never symptomatic", dexp_name}, {"hospitalization", dh_name}, {"ICU", dhicu_name}, {"mortality", dmort_name} };	
	// File with 	
	std::string tfname("test_data/tests_with_time.txt");

	ABM abm(dt, pfname, dfiles, tfname);

	// First the places
	abm.create_households(hfile);
	abm.create_schools(sfile);
	abm.create_workplaces(wfile);
	abm.create_hospitals(hspfile);
	abm.create_retirement_homes(rh_file);
	abm.create_carpools(cp_file);
	abm.create_public_transit(pt_file);
	abm.create_leisure_locations(ls_file);
	abm.initialize_mobility();

	//abm.print_mobility_probabilities(fmob_out);

	// Then the agents
	abm.create_agents(fin);
 	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();	
	
	// Simulation
	int n_lhs = 0, n_ltot = 0, n_tot = 0, n_same = 0;
	for (int ti = 0; ti<=tmax; ++ti) {
		const std::vector<Agent> agents_0 = abm.get_copied_vector_of_agents();
		
		abm.transmit_infection();
		
		const std::vector<Agent> agents_F = abm.get_copied_vector_of_agents();
		const std::vector<Leisure>& leisure_locations = abm.get_vector_of_leisure_locations();
		
		for (auto& loc : leisure_locations) {
			std::cout << loc.get_ID() << " " << loc.get_agent_IDs().size() << std::endl;
		}
		
		for (int i=0; i < agents_0.size(); ++i) {
			int L_ID0 = agents_0.at(i).get_leisure_ID(); 
			int L_IDF = agents_F.at(i).get_leisure_ID();
			std::string L_type0 = agents_0.at(i).get_leisure_type(); 
			std::string L_typeF = agents_F.at(i).get_leisure_type();

			if (L_ID0 > 0) {
				int aID = agents_0.at(i).get_ID();
				if ((L_type0 == L_typeF) && (L_ID0 == L_IDF)) {
					// This is possible, but it shouldn't happen too often
					++n_same;
				} else {
					// Should be properly removed
					if (L_type0 == "household") {
						;	
					} else if (L_type0 == "public") {
						std::vector<int> agent_IDs = leisure_locations.at(L_ID0-1).get_agent_IDs();
						if ((std::find(agent_IDs.begin(), agent_IDs.end(), aID)) 
										!= agent_IDs.end()) {
							std::cerr << "Agent still registered in the previous leisure location " << aID << std::endl;
						  	//return false;	
						}
					}
				}
			}
		}
	}
}




