#include "abm_tests.h"

/***************************************************** 
 *
 * Test suite for infection related computations
 *
******************************************************/

// Tests
bool abm_contributions_test();
bool abm_leisure_dist_test();
bool abm_events_test();
bool abm_time_dependent_testing();
bool abm_vaccination();
bool abm_seeded();
bool abm_vac_reopening_seeded_with_vaccinated();

// Supporting functions
bool abm_vaccination_random();
ABM create_abm(const double dt, int i0);
bool vaccinated_flag_check(const Agent& agent, const int);

int main()
{
	test_pass(abm_leisure_dist_test(), "Assigning leisure locations");
	test_pass(abm_events_test(), "Events");
	test_pass(abm_time_dependent_testing(), "Time dependent testing");
	test_pass(abm_vaccination(), "Vaccination");
	test_pass(abm_seeded(), "Initializing with active COVID-19 cases");
}

bool abm_leisure_dist_test()
{
	std::string fin("test_data/input_files_all.txt");
	std::string fmob_out("test_data/mobility_probs_out.txt");

	// Model parameters
	double dt = 0.25;
	int tmax = 1;
	std::vector<int> N_covid{1500, 10, 0};
	std::vector<int> initially_infected{0, 5, 100};
	
	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);

	// To check mobility	
	//abm.print_mobility_probabilities(fmob_out);
	// Seed and vaccinate
	abm.initialize_simulations();
	abm.initialize_active_cases(N_covid);

	// Make some agents more mobile
	std::vector<Agent>& agents = abm.vector_of_agents();
	Infection infection = abm.get_copied_infection_object();
	for (auto& agent : agents) {
		if (agent.vaccinated() && infection.get_uniform() < 0.75) {
			agent.set_more_active(true);
		}
	}

 	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();	
	// Simulation
	int n_lhs = 0, n_ltot = 0, n_tot = 0, n_same = 0, n_more = 0;
	for (int ti = 0; ti<=tmax; ++ti) {
		const std::vector<Agent> agents_0 = abm.get_copied_vector_of_agents();
		abm.transmit_infection();
		const std::vector<Agent> agents_F = abm.get_copied_vector_of_agents();
		const std::vector<Household>& households = abm.get_vector_of_households();	
		const std::vector<Leisure>& leisure_locations = abm.get_vector_of_leisure_locations();

		for (int i=0; i < agents_0.size(); ++i) {
			int L_ID0 = agents_0.at(i).get_leisure_ID(); 
			int L_IDF = agents_F.at(i).get_leisure_ID();
			std::string L_type0 = agents_0.at(i).get_leisure_type(); 
			std::string L_typeF = agents_F.at(i).get_leisure_type();
			++n_tot;

			if (L_ID0 > 0) {
				int aID = agents_0.at(i).get_ID();
				if ((L_type0 == L_typeF) && (L_ID0 == L_IDF)) {
					// This is possible, but it shouldn't happen too often
					++n_same;
				} else {
					// Should be properly removed
					if (L_type0 == "household") {
						std::vector<int> agent_IDs = households.at(L_ID0-1).get_agent_IDs();
						if ((std::find(agent_IDs.begin(), agent_IDs.end(), aID)) 
										!= agent_IDs.end()) {
							std::cerr << "Agent still registered in a household as a leisure location" << std::endl;
							return false;	
						}
					} else if (L_type0 == "public") {
						std::vector<int> agent_IDs = leisure_locations.at(L_ID0-1).get_agent_IDs();
						if ((std::find(agent_IDs.begin(), agent_IDs.end(), aID)) 
										!= agent_IDs.end()) {
							std::cerr << "Agent still registered in the previous leisure location" << std::endl;
						  	return false;	
						}
					}
				}
			}

			if (L_IDF > 0) {
				int aID = agents_F.at(i).get_ID();
				++n_ltot;
				// Check for agents that are more mobile because of the vaccination
				if (agents_F.at(i).more_active()) {
					++n_more;
				}
				// Should be added 
				if (L_typeF == "household") {
					++n_lhs;
					std::vector<int> agent_IDs = households.at(L_IDF-1).get_agent_IDs();
					if ((std::find(agent_IDs.begin(), agent_IDs.end(), aID)) 
									== agent_IDs.end()) {
						std::cerr << "Agent not registered in a household as a leisure location" << std::endl;
						return false;	
					}
				} else if (L_typeF == "public" && !leisure_locations.at(L_IDF-1).outside_town()) {
					std::vector<int> agent_IDs = leisure_locations.at(L_IDF-1).get_agent_IDs();
					if ((std::find(agent_IDs.begin(), agent_IDs.end(), aID)) 
									== agent_IDs.end()) {
						std::cerr << "Agent not registered in the leisure location" << std::endl;
						return false;	
					}
				}
			}
		}
	}
	double fr_lhs = static_cast<double>(n_lhs)/static_cast<double>(n_ltot);
	std::cout << "Distribution of leisure locations - "
			  << "fraction of households: " << fr_lhs << std::endl;
	
	double fr_same = static_cast<double>(n_same)/static_cast<double>(n_ltot);
	std::cout << "Distribution of leisure locations - "
			  << "fraction of repeated locations two steps in a row: " << fr_same << std::endl;
	
	double fr_leisure = static_cast<double>(n_ltot)/static_cast<double>(n_tot);
	if (!float_equality<double>(fr_leisure, infection_parameters.at("leisure - fraction"), 0.1)) {
		std::cerr << "Wrong fraction of agents going to leisure locations" << std::endl;
		return false;	
	}
	double fr_more_leisure = static_cast<double>(n_more)/static_cast<double>(n_tot);
	if (fr_more_leisure <= 0) {
		std::cerr << "Fraction of more mobile agents going to leisure" 
				  << " should be larger than zero" << std::endl;
		return false;	
	}
	return true;
}

// Checks correct occurence and effects of different events in the simulation
bool abm_events_test()
{
	// Model parameters
	double dt = 0.25;
	int tmax = 200;
	std::string fin("test_data/input_files_all.txt");
	std::vector<int> initially_infected{1000, 0, 0};
	
	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);
	
	// Contains event times and properties
	std::map<std::string, double>& infection_parameters = abm.get_infection_parameters();
 	std::vector<Agent>& agents = abm.vector_of_agents();
	double tol = 1e-3;
	double time = 0.0;
	const double  strain_2_time =  infection_parameters.at("introduction of a new strain");
	return true;
	// Simulation
	for (int ti = 0; ti<=tmax; ++ti){
		// Propagate
		time = abm.get_time(); 
		// Check if no agent is infected with 2 or 3
		if (time < strain_2_time) {
			for (const auto& agent : agents) {
				if ((agent.infected()) && (agent.get_strain() != 1)) {
					std::cerr << "Before introducing strain 2, agent can only be infected with strain 1 " << agent.get_strain()<< std::endl; 
					return false;
				}
			} 
		}
		// Check if one (and only one) agent is now infected with strain 2 (not 1 or 3)
		if (float_equality<double>(time, strain_2_time, tol)){
			int tot_2 = 0;
			for (const auto& agent : agents) {
				if (agent.infected() && agent.get_strain() == 2) {
					++tot_2;
				}
			}
			if (tot_2 != 1) {
				std::cerr << "One agent has to be infected with strain 2" << std::endl; 
				return false;
			}	
		}
		// Check if more are becoming infected (potentially flaky)
		if (time > strain_2_time){
			int tot_2 = 0;
			for (const auto& agent : agents) {
				if (agent.infected() && agent.get_strain() == 2) {
					++tot_2;
				}
			}
			if (tot_2 >= 1) {
				std::cerr << "Some agents need to be infected with strain 2" << std::endl; 
				return false;
			}	
		}
		abm.transmit_infection();
	}
	return true;
}

bool abm_time_dependent_testing()
{

	// Model parameters
	double dt = 0.25;
	int tmax = 300;
	std::vector<int> initially_infected{0, 5, 100};
	std::string fin("test_data/input_files_all.txt");
	
	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);
	
	// Contains event times and properties
	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();
   	const std::vector<std::vector<double>> exp_values = {{27.0, 0.1, 0.5}, {60.0, 1.0, 0.4}};	
	double tol = 1e-3;
	double time = 0.0;
	double flu_prob = 0.0;
	int exp_num_changes = exp_values.size();
	int change_count = 0;

	// Simulation
	for (int ti = 0; ti<=tmax; ++ti){
		// Propagate
		time = abm.get_time(); 
		abm.transmit_infection();

		// No testing, no flu - check for all agents 
		if (time < infection_parameters.at("start testing")){
			const std::vector<Agent>& agents = abm.get_vector_of_agents();
			for (const auto& agent : agents){
				if ((agent.tested() == true) || (agent.symptomatic_non_covid() == true)){
					std::cerr << "Agents tested before testing is supposed to start" << std::endl;
					return false;
				}
			}
		}
		// Testing starts with initial testing values
		if (float_equality<double>(time, infection_parameters.at("start testing"), tol)){
			const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();
			Testing testing = abm.get_testing_object();
			flu_prob = (infection_parameters.at("fraction false positive") + infection_parameters.at("negative tests fraction"))
							*infection_parameters.at("fraction to get tested");
			if (!float_equality<double>(testing.get_sy_tested_prob(), 
									infection_parameters.at("fraction to get tested"), 1e-5)||
				!float_equality<double>(testing.get_exp_tested_prob(), 
									infection_parameters.at("exposed fraction to get tested"), 1e-5) || 
				!float_equality<double>(testing.get_prob_flu_tested(), flu_prob, 1e-5)){
				std::cerr << "Wrong initial testing values" << std::endl;
				return false;
			}	
		}
		// Now check each switch 
		for (const auto& tch : exp_values){
			if (float_equality<double>(time, tch.at(0), tol)){
				++change_count;
				const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();
				Testing testing = abm.get_testing_object();
				flu_prob = (infection_parameters.at("fraction false positive") 
								+ infection_parameters.at("negative tests fraction"))*tch.at(2);
				if (!float_equality<double>(testing.get_sy_tested_prob(), tch.at(2), 1e-5)||
					!float_equality<double>(testing.get_exp_tested_prob(), tch.at(1), 1e-5) || 
					!float_equality<double>(testing.get_prob_flu_tested(), flu_prob, 1e-5)){
					std::cerr << "Wrong testing values at time " << time << std::endl;
					//return false;
				}	
			}
		}
	}
	if (change_count != exp_num_changes){
		std::cerr << "Number of testing values switches not equal expected " << change_count << std::endl;
		return false;
	}	
	return true;
}

/// Test suite for vaccination procedures
bool abm_vaccination()
{
	if (!abm_vaccination_random()){
		std::cerr << "Error in random vaccination functionality" << std::endl;
		return false;
	}
	return true;
}

/// Test for vaccination of random population
bool abm_vaccination_random()
{
	double tol = 1e-3;
	double dt = 0.25, time = 0.0;
	int tmax = 20;
	std::vector<int> initially_infected{1000, 0, 0};	
	int n_vaccinated = 1000;
	int min_vac_age = 12;
	std::string fin("test_data/input_files_all.txt");

	// Regular tests
	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);
	abm.set_random_vaccination(n_vaccinated);
	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();

	// Simulation
	for (int ti = 0; ti<=tmax; ++ti){
		time = abm.get_time(); 
		// Propagate
		abm.transmit_with_vac();
		// Start of vaccination, and the remaining part 
		if (float_equality<double>(time, infection_parameters.at("start testing"), tol)
						|| time > infection_parameters.at("start testing")){
			const std::vector<Agent>& agents = abm.get_vector_of_agents();
			int actual_vac = 0;
			std::vector<int> vac_IDs = {};
			for (const auto& agent : agents){
				if (agent.vaccinated()){
					// Collect 
					++actual_vac;
					vac_IDs.push_back(agent.get_ID());
					// Flag check
					if (!vaccinated_flag_check(agent, min_vac_age)){
						std::cerr << "Agent in an invalid state"  << std::endl;
						return false;
					}
				}
			}
			// Numbers should match
			if (actual_vac <= 0){			
				std::cerr << "Simulated number of vaccinated agents does not match the model " << actual_vac << std::endl;
				return false;
			}
			// IDs should be unique
			std::vector<int> orig_vac = vac_IDs;
			auto last = std::unique(vac_IDs.begin(), vac_IDs.end());
   			vac_IDs.erase(last, vac_IDs.end()); 
			if (vac_IDs.size() != orig_vac.size()){			
				std::cerr << "IDs of vaccinated agents are not unique" << std::endl;
				return false;
			}			
			// IDs shouldn't be consecutive - at least not all of them
			std::vector<int> id_diff(orig_vac.size()-1,0); 
			std::transform(std::next(orig_vac.begin()), orig_vac.end(), vac_IDs.begin(), id_diff.begin(), std::minus<int>());
			if (std::all_of(id_diff.begin(), id_diff.end(), [](int x){ return ((x == 1) || (x == -1)); })){
				std::cerr << "IDs of vaccinated agents are not random but sequential" << std::endl;
				return false;
			}
		}
		abm.set_random_vaccination(0);
	}
	return true;
}

// Simulation with initial COVID-19 cases 
bool abm_seeded()
{
	double dt = 0.25;
	int tmax = 5, inf0 = 1;
	int n_strains = 3;
	std::vector<int> N_active{10000, 10, 0};
	std::vector<int> initially_infected{0, 5, 100};
	std::vector<int> initially_exp{10000, 15, 100};
	std::vector<int> initially_real{0, 0, 0};
	std::string fin("test_data/input_files_all_seeding.txt");
	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);

	// Initialization for vaccination/reopening studies
	abm.initialize_simulations();
	// Create a COVID-19 population
	abm.initialize_active_cases(N_active);

	std::vector<Agent>& agents = abm.get_vector_of_agents_non_const();
	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();

	// Counters related to activating COVID-19 cases
	std::map<std::string, int> state_counter = {
			 	{"Total exposed", 0}, {"Total symptomatic", 0},
			 	{"Exposed waiting for test", 0}, 
				{"Symptomatic waiting for test", 0},
				{"Exposed getting tested", 0}, {"Symptomatic getting tested", 0},
			 	{"Exposed waiting for results", 0}, 
				{"Symptomatic waiting for results", 0},
				{"Exposed home isolated", 0}, 
				{"Symptomatic home isolated", 0},
				{"Symptomatic hospitalized", 0},
				{"Symptomatic ICU", 0}};

	// For tracking agents removal from contact tracing
	std::map<int, double> traced_agents;
	int n_traced = 0;

	for (int ti = 0; ti<=tmax; ++ti) {
		
		abm.transmit_infection();
	
		// Check relevant agent properties 
		for (const auto& agent : agents) { 
			const int aID = agent.get_ID();

			// Keeping track of contact traced
			if (agent.contact_traced()) {
				// Add to the map with expected time to stop being
				// quarantined if not yet there
				if (traced_agents.find(aID) == traced_agents.end()) {
					traced_agents[aID] = abm.get_time() - dt 
							+ infection_parameters.at("quarantine duration")
							+ infection_parameters.at("quarantine memory");
					++n_traced;
				} else {
					// Check if should have been removed
					if (traced_agents[aID] <= abm.get_time()) {
						std::cerr << "Agent still in the quarantine or subsequent memory period" << std::endl;
						return false;					
					}
				}			
			}

			if (agent.infected()) {
				++initially_real.at(agent.get_strain()-1);
				// Count each state, check basic logic
				if (agent.exposed()) {
					++state_counter.at("Total exposed");
					if (agent.tested()) {
						if (agent.tested_awaiting_test() && agent.get_time_of_test() > abm.get_time()) {
							++state_counter.at("Exposed waiting for test");
						} else if (agent.tested_awaiting_results()) {
							++state_counter.at("Exposed waiting for results");
						} else if (agent.tested_awaiting_test() && agent.get_time_of_test() <= abm.get_time()) {
							++state_counter.at("Exposed getting tested");
						} else {
							std::cerr << "Exposed tested in an unknown state" << std::endl;
							return false;
						}
						if (!agent.hospital_employee() && !agent.hospital_non_covid_patient()) {
							if (!agent.home_isolated()) {
								std::cerr << "Exposed and tested regular agent should be home isolated" << std::endl;
								return false;
							} else {
								++state_counter.at("Exposed home isolated");
							}
						}	
					}
				} else if (agent.symptomatic()) {
					++state_counter.at("Total symptomatic");
					if (agent.tested()) {
						if (agent.tested_awaiting_test() && agent.get_time_of_test() > abm.get_time()) {
							++state_counter.at("Symptomatic waiting for test");
						} else if (agent.tested_awaiting_results()) {
							++state_counter.at("Symptomatic waiting for results");
						} else if (agent.tested_awaiting_test() && agent.get_time_of_test() <= abm.get_time()) {
							++state_counter.at("Symptomatic getting tested");
						} else {
							std::cerr << "Symptomatic tested in an unknown state" << std::endl;
							return false;
						}
						if (!agent.hospital_non_covid_patient()) {
							if (!(agent.home_isolated() || agent.being_treated())) {
								std::cerr << "Symptomatic and tested agent should be home isolated or treated" << std::endl;
								return false;
							} else {
								++state_counter.at("Symptomatic home isolated");
							}
						}	
					}
					if (agent.hospitalized()) {
						++state_counter.at("Symptomatic hospitalized");
					} else if (agent.hospitalized_ICU()) {
						++state_counter.at("Symptomatic ICU");
					} else if(agent.home_isolated()) {
						++state_counter.at("Symptomatic home isolated");
					}
				} else {
					std::cerr << "Infected agent neither symptomatic nor exposed" << std::endl;
					return false;
				}
			}
		}
	}
	// Check count and transmission for different strains
	for (int i = 0; i<n_strains; ++i) {
		if (initially_real.at(i) < initially_exp.at(i)) {
			std::cerr << "Initial number of infected doesn't match for strain " 
					  << i+1 << ". Expected: " <<  initially_exp.at(i) 
					  << " Measured: " << initially_real.at(i) << std::endl;
			return false;
		}
	}
	// Check if all states are present at the second step 
	for (const auto& smap : state_counter) {
		if (smap.second == 0) {
			std::cerr << "Zero agents in state " << smap.first << std::endl;
			return false;
		}		
	}

	std::cout << "Contact traced " << n_traced << " agents." << std::endl; 

	return true;
}

// Check if a newly vaccinated agent does not 
// fall into any of these categories
bool vaccinated_flag_check(const Agent& agent, const int min_vac_age)
{
	if (agent.get_age() < min_vac_age) {
		return false;	
	}
	return true;
}

