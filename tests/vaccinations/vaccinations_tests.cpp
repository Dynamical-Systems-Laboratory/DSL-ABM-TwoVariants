#include <set>
#include <fstream>
#include "../../include/vaccinations.h"
#include "../../include/infection.h"
#include "../common/test_utils.h"

/***************************************************** 
 *
 * Test suite for the Vaccinations class 
 *
 *****************************************************/

using setter = void (Agent::*)(const bool);
using getter = bool (Agent::*)() const;
using nested_maps = const std::map<std::string, std::map<std::string, std::vector<std::vector<double>>>>;
using one_map = const std::map<std::string, std::vector<std::vector<double>>>;

// Tests
bool check_random_vaccinations_functionality();
bool check_group_vaccinations_functionality();
bool check_random_vaccinations_neg_time_offset();
bool check_random_revaccinations();

// Supporting functions
bool check_agent_vaccination_attributes(Agent& agent, const double time, 
											nested_maps&, const double offset, int,
 											const bool other_strain, const int second_strain,
											const std::set<int>& revac_IDs = {}, std::ostream& out= std::cout);

int main()
{
	test_pass(check_random_vaccinations_functionality(), "Random vaccination functionality");
	test_pass(check_random_vaccinations_neg_time_offset(), "Random vaccination functionality - negative time offset");
	test_pass(check_random_revaccinations(), "Random re-vaccination functionality");
}

bool check_random_vaccinations_functionality()
{
	// Vaccination settings
	const std::string data_dir_1("test_data/strain_1/");
	const std::string data_dir_2("test_data/strain_2/");
	const std::string data_dir_3("test_data/strain_3/");
	const std::string fname_1("test_data/vaccination_parameters_strain_1.txt");
	const std::string fname_2("test_data/vaccination_parameters_strain_2.txt");
	const std::string fname_3("test_data/vaccination_parameters_strain_3.txt");
	// Initial number to vaccinate
	int n_vac_0 = 10000;
	std::vector<int> temp_vac;
	// Vaccinate this many each step
	int n_vac = 10;
	// Actual number of vaccinated
	int n_vac_cur_2 = 0;	
	// Maximum number to vaccinate
	int n_vac_max = 30000;
	// Max number of time steps
	int n_steps = 150;
	// Probability of state assignements
	double prob = 0.0;
	// Time and time step
	double time = 0.0, dt = 0.25;
	// Strains
	int n_strains = 3;
	// For random 
	Infection infection(dt);
	// Total number of vaccinated for strain 2
	int as_2_tot = 0;

	// Create agents
	// Number of agents
	int n_agents = 50000;
	// Default agent objects
	std::vector<Agent> agents(n_agents);
	// States some agents need to be in and their probabilities
	std::map<std::string, std::pair<double, setter>> agent_states = 
		{{"removed_dead", {0.1, &Agent::set_removed_dead}},
 		 {"tested_covid_positive", {0.25, &Agent::set_tested_covid_positive}},
		 {"removed_can_vaccinate", {0.53, &Agent::set_removed_can_vaccinate}},
		 {"former_suspected", {0.3, &Agent::set_former_suspected}},
		 {"symptomatic", {0.12, &Agent::set_symptomatic}},
		 {"symptomatic_non_covid", {0.18, &Agent::set_symptomatic_non_covid}},
		 {"home_isolated", {0.23, &Agent::set_home_isolated}},
		 {"needs_next_vaccination", {0.11, &Agent::set_needs_next_vaccination}}};	
	// Randomly assign age and different states
	int aID = 1;
	int tot_strains = 3;
	for (auto& agent : agents) {
		// Initialize vaccination characteristics
		agent.set_n_strains(tot_strains);
		agent.initialize_benefits();
		// Assign ID
		agent.set_ID(aID);	
		// Assign age
		agent.set_age(infection.get_int(0,100));
		// Assign other states
		prob = infection.get_uniform();
		for (const auto& state : agent_states) {
			if (prob <= state.second.first) {
				if (state.first == "removed_can_vaccinate") {
					/*agent.set_removed_recovered(true, 2);
					if (infection.get_uniform() < 0.5) {
						(agent.*state.second.second)(false);
					} else {
						(agent.*state.second.second)(true);
					}*/
				} else {
					(agent.*state.second.second)(true);
				}
			} else {
				(agent.*state.second.second)(false);
			}	
		}
		++aID;			
	}

	// State checking 
	std::map<std::string, getter> agent_states_check = 
		{{"removed_dead", &Agent::removed_dead},
 		 {"tested_covid_positive", &Agent::tested_covid_positive},
		 {"removed_can_vaccinate", &Agent::removed_can_vaccinate},
		 {"former_suspected", &Agent::former_suspected},
		 {"symptomatic", &Agent::symptomatic},
		 {"symptomatic_non_covid", &Agent::symptomatic_non_covid},
		 {"home_isolated", &Agent::home_isolated},
		 {"needs_next_vaccination", &Agent::needs_next_vaccination}};

	// Vaccination functionality
	Vaccinations vac_strain_1(fname_1, data_dir_1);
	Vaccinations vac_strain_2(fname_2, data_dir_2);
	Vaccinations vac_strain_3(fname_3, data_dir_3);

	// Vaccination data
	nested_maps& vac_data_map_1 = vac_strain_1.get_vaccination_data();
	nested_maps& vac_data_map_2 = vac_strain_2.get_vaccination_data();
	nested_maps& vac_data_map_3 = vac_strain_3.get_vaccination_data();
	// Vaccination parameters
	const std::map<std::string, double>& vac_params_2 = vac_strain_2.get_vaccination_parameters();
	const std::map<std::string, double>& vac_params_3 = vac_strain_3.get_vaccination_parameters();

	// Strain 2 first
	// Check max eligible
	int n_eligible = 0; 
	for (const auto& agent : agents) {
		bool not_eligible = false;
		if (agent.get_age() < vac_params_2.at("Minimum vaccination age")) {
			not_eligible = true;
		} else {
			for (const auto& state : agent_states_check) {
				if (state.first == "removed_can_vaccinate") {
					if ((agent.*state.second)() == false && agent.removed_recovered(2)) {
						not_eligible = false;
					}
				} else if ((agent.*state.second)() == true) {
					not_eligible = true;		
				}	
			}
		}
		if (not_eligible == false) {
			++n_eligible;				
		}		
	}
	// Should match max eligible at the begining 
	if (n_eligible != vac_strain_2.max_eligible_random(agents)) {
		std::cerr << "Wrong number of initially eligible to vaccinate" << std::endl;
		return false;
	}
	
	// Initially vaccinated
	temp_vac = vac_strain_2.vaccinate_random(agents, n_vac_0, 0, infection, time);	
	n_vac_cur_2 = temp_vac.at(0);
	as_2_tot += n_vac_cur_2;	

	// At this point they need to be the same (there is a sufficient number of agents)
	if (n_vac_cur_2 != n_vac_0) {
		std::cerr << "Wrong number of initially vaccinated" << std::endl;
		return false;
	}	

	// Check properties with time
	std::vector<double> offsets(n_agents, 0.0);
	for (int i=0; i<n_steps; ++i) {
		for (auto& agent : agents) {
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_2, offsets.at(agent.get_ID()-1), 2, false, 2)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents" << std::endl;
				return false;		
			}
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_2, offsets.at(agent.get_ID()-1), 2, true, 1)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents for second strain 1" << std::endl;
				return false;		
			}
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_2, offsets.at(agent.get_ID()-1), 2, true, 3)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents for second strain 3" << std::endl;
				return false;		
			}
			// Update offsets of all non-vaccinated for new vaccinations
			// If they get vaccinated, they will have correct offsets
			if (!agent.vaccinated()) {
				offsets.at(agent.get_ID()-1) = time;
				// Should have no vaccinations
				if (!check_agent_vaccination_attributes(agent, time, vac_data_map_1, offsets.at(agent.get_ID()-1), 1, false, 1)) {
					std::cerr << "Error in properties of vaccinated and not vaccinated agents" << std::endl;
					return false;		
				}
				if (!check_agent_vaccination_attributes(agent, time, vac_data_map_3, offsets.at(agent.get_ID()-1), 3, false, 3)) {
					std::cerr << "Error in properties of vaccinated and not vaccinated agents" << std::endl;
					return false;		
				}
			}		
		}
		// Vaccinate new batch and save offsets
		temp_vac = vac_strain_2.vaccinate_random(agents, n_vac, 0, infection, time);
		n_vac_cur_2 = temp_vac.at(0);
		as_2_tot += n_vac_cur_2;
		time += dt;
	}
	// Vaccinate some more, now with strain 3
	int n_3_0 = 10000;
	temp_vac = vac_strain_3.vaccinate_random(agents, n_3_0, 0, infection, time);
	int n_vac_cur_3 = temp_vac.at(0);
	if (n_vac_cur_3 != n_3_0) {
		std::cerr << "Wrong number of initially vaccinated for strain 3" << std::endl;
		return false;
	}
	int as_2 = 0;
	for (auto& agent : agents) {
		if (agent.is_vaccinated_for_strain(2) && !agent.is_vaccinated_for_strain(3)) {
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_2, offsets.at(agent.get_ID()-1), 2, false, 2)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents" << std::endl;
				return false;		
			}
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_2, offsets.at(agent.get_ID()-1), 2, true, 1)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents for second strain 1" << std::endl;
				return false;		
			}
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_2, offsets.at(agent.get_ID()-1), 2, true, 3)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents for second strain 3" << std::endl;
				return false;		
			}
			++as_2;		
		}
		if (agent.is_vaccinated_for_strain(3) && !agent.is_vaccinated_for_strain(2)) {
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_3, time, 3, false, 3)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents for main strain 3" << std::endl;
				return false;		
			}
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_3, time, 3, true, 1)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents for second strain 1" << std::endl;
				return false;		
			}
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_3, time, 3, true, 2)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents for second strain 2" << std::endl;
				return false;		
			}
		}
		if (agent.is_vaccinated_for_strain(2) && agent.is_vaccinated_for_strain(3)) {
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_2, offsets.at(agent.get_ID()-1), 2, false, 2)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents for first strain 2" << std::endl;
				return false;		
			}
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_3, time, 3, true, 1)) {
				std::cerr << "Error in properties for strain 1 if vaccinated for 2 and 3" << std::endl;
				return false;		
			}
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_3, time, 3, false, 3)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents for first strain 3 (when vaxed for 2 and 3)" << std::endl;
				return false;		
			}
			++as_2;		
		}
	}
	if (as_2 != as_2_tot) {
		std::cerr << "Wrong number of vaccinated for strain 2" << std::endl;
		return false;
	}	

	// Exceeding the limits
	n_eligible = vac_strain_2.max_eligible_random(agents);
	// Should vaccinate only n_eligible 
	n_vac = n_eligible + 1; 
	temp_vac = vac_strain_2.vaccinate_random(agents, n_vac, 0, infection, time);
	n_vac_cur_2 = temp_vac.at(0);
	if (n_vac_cur_2 != n_eligible) {
		std::cerr << "Wrong number of agents vaccinated after the limit was exceeded" << std::endl;
		return false;
	}
	// Now it shouldn't vaccinate at all
	n_vac = 1;
	temp_vac = vac_strain_2.vaccinate_random(agents, n_vac, 0, infection, time);
	n_vac_cur_2 = temp_vac.at(0);
	if (n_vac_cur_2 != 0) {
		std::cerr << "No agents should be vaccinated at this point " << n_vac_cur_2 << std::endl;
		return false;
	}

	return true;
}

bool check_random_vaccinations_neg_time_offset()
{
	// Vaccination settings
	const std::string data_dir_1("test_data/strain_1/");
	const std::string data_dir_2("test_data/strain_2/");
	const std::string data_dir_3("test_data/strain_3/");
	const std::string fname_1("test_data/vaccination_parameters_strain_1.txt");
	const std::string fname_2("test_data/vaccination_parameters_strain_2.txt");
	const std::string fname_3("test_data/vaccination_parameters_strain_3.txt");
	// Initial number to vaccinate
	int n_vac_0 = 10000;
	// Vaccinate this many each step
	int n_vac = 10;
	// Actual number of vaccinated
	int n_vac_cur_2 = 0;		
	// Maximum number to vaccinate
	int n_vac_max = 30000;
	// Max number of time steps
	int n_steps = 2;
	// Probability of state assignements
	double prob = 0.0;
	// Time and time step
	double time = 0.0, dt = 0.25;
	// Strains
	int n_strains = 3;
	// For random 
	Infection infection(dt);
	// Total number of vaccinated for strain 2
	int as_2_tot = 0;

	// Create agents
	// Number of agents
	int n_agents = 50000;
	// Default agent objects
	std::vector<Agent> agents(n_agents);
	// States some agents need to be in and their probabilities
	std::map<std::string, std::pair<double, setter>> agent_states = 
		{{"removed_dead", {0.1, &Agent::set_removed_dead}},
 		 {"tested_covid_positive", {0.25, &Agent::set_tested_covid_positive}},
		 {"removed_can_vaccinate", {0.53, &Agent::set_removed_can_vaccinate}},
		 {"former_suspected", {0.3, &Agent::set_former_suspected}},
		 {"symptomatic", {0.12, &Agent::set_symptomatic}},
		 {"symptomatic_non_covid", {0.18, &Agent::set_symptomatic_non_covid}},
		 {"home_isolated", {0.23, &Agent::set_home_isolated}},
		 {"needs_next_vaccination", {0.11, &Agent::set_needs_next_vaccination}}};	
	// Randomly assign age and different states
	int aID = 1;
	int tot_strains = 3;
	for (auto& agent : agents) {
		// Initialize vaccination characteristics
		agent.set_n_strains(tot_strains);
		agent.initialize_benefits();
		// Assign ID
		agent.set_ID(aID);	
		// Assign age
		agent.set_age(infection.get_int(0,100));
		// Assign other states
		prob = infection.get_uniform();
		for (const auto& state : agent_states) {
			if (prob <= state.second.first) {
				if (state.first == "removed_can_vaccinate") {
					agent.set_removed_recovered(true, 2);
					/*if (infection.get_uniform() < 0.5) {
						(agent.*state.second.second)(false);
					} else {
						(agent.*state.second.second)(true);
					}*/
				} else {
					(agent.*state.second.second)(true);
				}
			} else {
				(agent.*state.second.second)(false);
			}	
		}
		++aID;			
	}

	// State checking 
	std::map<std::string, getter> agent_states_check = 
		{{"removed_dead", &Agent::removed_dead},
 		 {"tested_covid_positive", &Agent::tested_covid_positive},
		 {"removed_can_vaccinate", &Agent::removed_can_vaccinate},
		 {"former_suspected", &Agent::former_suspected},
		 {"symptomatic", &Agent::symptomatic},
		 {"symptomatic_non_covid", &Agent::symptomatic_non_covid},
		 {"home_isolated", &Agent::home_isolated},
		 {"needs_next_vaccination", &Agent::needs_next_vaccination}};

	// Vaccination functionality
	Vaccinations vac_strain_1(fname_1, data_dir_1);
	Vaccinations vac_strain_2(fname_2, data_dir_2);
	Vaccinations vac_strain_3(fname_3, data_dir_3);

	// Vaccination data
	nested_maps& vac_data_map_1 = vac_strain_1.get_vaccination_data();
	nested_maps& vac_data_map_2 = vac_strain_2.get_vaccination_data();
	nested_maps& vac_data_map_3 = vac_strain_3.get_vaccination_data();
	// Vaccination parameters
	const std::map<std::string, double>& vac_params_2 = vac_strain_2.get_vaccination_parameters();
	const std::map<std::string, double>& vac_params_3 = vac_strain_3.get_vaccination_parameters();

	// Strain 2 first
	// Check max eligible
	int n_eligible = 0; 
	for (const auto& agent : agents) {
		bool not_eligible = false;
		if (agent.get_age() < vac_params_2.at("Minimum vaccination age")) {
			not_eligible = true;
		} else {
			for (const auto& state : agent_states_check) {
				if (state.first == "removed_can_vaccinate") {
					/*if ((agent.*state.second)() == false && agent.removed_recovered(2)) {
						not_eligible = false;
					}*/
				} else if ((agent.*state.second)() == true) {
					not_eligible = true;		
				}	
			}
		}
		if (not_eligible == false) {
			++n_eligible;				
		}		
	}
	// Should match max eligible at the begining 
	if (n_eligible != vac_strain_2.max_eligible_random(agents)) {
		std::cerr << "Wrong number of initially eligible to vaccinate" << std::endl;
		return false;
	}
	
	// Initially vaccinated
	n_vac_cur_2 = vac_strain_2.vaccinate_random_time_offset(agents, n_vac_0, 0, infection, time);	
	as_2_tot += n_vac_cur_2;

	// At this point they need to be the same (there is a sufficient number of agents)
	if (n_vac_cur_2 != n_vac_0) {
		std::cerr << "Wrong number of initially vaccinated" << std::endl;
		return false;
	}	

	// Check properties with time
	for (int i=0; i<n_steps; ++i) {
		for (auto& agent : agents) {
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_2, agent.get_vac_time_offset(), 2, false, 2)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents" << std::endl;
				return false;		
			}
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_2, agent.get_vac_time_offset(), 2, true, 1)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents for second strain 1" << std::endl;
				return false;		
			}
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_2, agent.get_vac_time_offset(), 2, true, 3)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents for second strain 3" << std::endl;
				return false;		
			}
		}
		// Vaccinate new batch and save offsets
		n_vac_cur_2 = vac_strain_2.vaccinate_random_time_offset(agents, n_vac, 0, infection, time);
		as_2_tot += n_vac_cur_2;
		time += dt;
	}
	// Exceeding the limits
	n_eligible = vac_strain_2.max_eligible_random(agents);
	// Should vaccinate only n_eligible 
	n_vac = n_eligible + 1; 
	n_vac_cur_2 = vac_strain_2.vaccinate_random_time_offset(agents, n_vac, 0, infection, time);
	if (n_vac_cur_2 != n_eligible) {
		std::cerr << "Wrong number of agents vaccinated after the limit was exceeded" << std::endl;
		return false;
	}
	// Now it shouldn't vaccinate at all
	n_vac = 1;
	n_vac_cur_2 = vac_strain_2.vaccinate_random_time_offset(agents, n_vac, 0, infection, time);
	if (n_vac_cur_2 != 0) {
		std::cerr << "No agents should be vaccinated at this point" << std::endl;
		return false;
	}

	return true;
}

/// Tests all the states and properties related to vaccinations
bool check_agent_vaccination_attributes(Agent& agent, const double time, 
											nested_maps& vac_data_map, const double offset, int strain_id,
											const bool other_strain, const int second_strain, 
											const std::set<int>& revac_IDs, std::ostream& fout)
{
	double tol = 1e-2;
	int n_strains = agent.get_n_strains();
	if (!agent.vaccinated()) {
		for (int i = 1; i <= n_strains; ++i) {
			// All vaccination-dependent properties should be at their default values
			if (!float_equality<double>(agent.vaccine_effectiveness(time, i), 0.0, tol)) {
				std::cerr << "Computed effectiveness not equal default." << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.asymptomatic_correction(time, i), 1.0, tol)) {
				std::cerr << "Computed probability of being asymptomatic not equal default." << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.transmission_correction(time, i), 1.0, tol)) {
				std::cerr << "Computed probability correction of transmission not equal default." << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.severe_correction(time, i), 1.0, tol)) {
				std::cerr << "Computed probability correction of developing severe disease not equal default." << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.death_correction(time, i), 1.0, tol)) {
				std::cerr << "Computed probability correction of dying not equal default." << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.get_time_vaccine_effects_reduction(), 0.0, tol)) {
				std::cerr << "Vaccine effects drop should be set to initial value (0.0) at this point" << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.get_time_mobility_increase(), 0.0, tol)) {
				std::cerr << "Time when mobility increases should be set to initial value (0.0) at this point" << std::endl;
				return false;	
			}
		}
	} else {
		// For re-vaccinations - just print and inspect
		if ((agent.got_booster()) && (!revac_IDs.empty())) {
			fout << time << " " << agent.get_ID() << " " << agent.get_vaccine_subtype(strain_id) << " " 
				 << agent.vaccine_effectiveness(time, strain_id) << " " << agent.asymptomatic_correction(time, strain_id) << " " 
				 << agent.transmission_correction(time, strain_id) << " " 
				 << agent.severe_correction(time, strain_id) << " " << agent.death_correction(time, strain_id) << std::endl;
			return true; 
		}
		// Get the property map for this agents tag
		std::string tag = agent.get_vaccine_subtype(strain_id);
		if (other_strain) {
			tag = tag + std::string(" other strain ") + std::to_string(second_strain);
		}
		one_map& prop_map = vac_data_map.at(tag);
		// Construct tpf or fpf for each property
		// Check for current time if all properties equal expeceted
		if (agent.get_vaccine_type(strain_id) == "one_dose") { 
			strain_id = other_strain ? second_strain : strain_id;
			ThreePartFunction tpf_eff(prop_map.at("effectiveness"), offset);
			ThreePartFunction tpf_asm(prop_map.at("asymptomatic"), offset);
			ThreePartFunction tpf_tra(prop_map.at("transmission"), offset);
			ThreePartFunction tpf_sev(prop_map.at("severe"), offset);
			ThreePartFunction tpf_dth(prop_map.at("death"), offset);
		
			// For debugging
		//	std::cout << "One dose: " << tag << " " << agent.vaccine_effectiveness(time, strain_id) << " " <<  tpf_eff(time) << std::endl;
	
			if (!float_equality<double>(agent.vaccine_effectiveness(time, strain_id), tpf_eff(time), tol)) {
				std::cerr << "Computed effectiveness not equal expected" << std::endl;
				std::cout << "One dose: " << tag << " " << agent.vaccine_effectiveness(time, strain_id) << " " <<  tpf_eff(time) << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.asymptomatic_correction(time, strain_id), tpf_asm(time), tol)) {
				std::cerr << "Computed probability of being asymptomatic not equal expected" << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.transmission_correction(time, strain_id), tpf_tra(time), tol)) {
				std::cerr << "Computed probability correction of transmission not equal expected" << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.severe_correction(time, strain_id), tpf_sev(time), tol)) {
				std::cerr << "Computed probability correction of developing severe disease not equal expected" << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.death_correction(time, strain_id), tpf_dth(time), tol)) {
				std::cerr << "Computed probability correction of dying not equal expected" << std::endl;
				return false;	
			} 
		} else {
			strain_id = other_strain ? second_strain : strain_id;
		 	FourPartFunction fpf_eff(prop_map.at("effectiveness"), offset);
			FourPartFunction fpf_asm(prop_map.at("asymptomatic"), offset);
			FourPartFunction fpf_tra(prop_map.at("transmission"), offset);
			FourPartFunction fpf_sev(prop_map.at("severe"), offset);
			FourPartFunction fpf_dth(prop_map.at("death"), offset);

			// For debugging
		//	std::cout << "Two dose: " << tag << " " << agent.vaccine_effectiveness(time, strain_id) << " " <<  fpf_eff(time) << std::endl;
	
			if (!float_equality<double>(agent.vaccine_effectiveness(time, strain_id), fpf_eff(time), tol)) {
				std::cerr << "Computed effectiveness not equal expected " << std::endl;
				std::cout << "Two dose: " << tag << " " << agent.vaccine_effectiveness(time, strain_id) << " " <<  fpf_eff(time) << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.asymptomatic_correction(time, strain_id), fpf_asm(time), tol)) {
				std::cerr << "Computed probability of being asymptomatic not equal expected" << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.transmission_correction(time, strain_id), fpf_tra(time), tol)) {
				std::cerr << "Computed probability correction of transmission not equal expected" << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.severe_correction(time, strain_id), fpf_sev(time), tol)) {
				std::cerr << "Computed probability correction of developing severe disease not equal expected" << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.death_correction(time, strain_id), fpf_dth(time), tol)) {
				std::cerr << "Computed probability correction of dying not equal expected" << std::endl;
				return false;	
			}
		}
		// General properties
		if (agent.get_time_vaccine_effects_reduction() < 0.0 && offset >= 0) {
			std::cerr << "Vaccine effects drop not set after vaccination" << std::endl;
			return false;	
		}
		if (agent.get_time_mobility_increase() < 0.0 && offset >= 0) {
			std::cerr << "Time when agent's mobility increases not set after vaccination" << std::endl;
			return false;	
		}
	}
	return true;
}

bool check_random_revaccinations()
{
	// Vaccination settings
	const std::string data_dir_1("test_data/strain_1/");
	const std::string data_dir_2("test_data/strain_2/");
	const std::string data_dir_3("test_data/strain_3/");
	const std::string fname_1("test_data/vaccination_parameters_strain_1.txt");
	const std::string fname_2("test_data/vaccination_parameters_strain_2.txt");
	const std::string fname_3("test_data/vaccination_parameters_strain_3.txt");
	// Initial number to vaccinate
	int n_vac_0 = 10000;
	// Vaccinate this many each step
	int n_vac = 10;
	// Actual number of vaccinated
	int n_vac_cur_2 = 0;	
	// Maximum number to vaccinate
	int n_vac_max = 30000;
	// Max number of time steps
	int n_steps = 2;
	// Probability of state assignements
	double prob = 0.0;
	// Time and time step
	double time = 0.0, dt = 0.25;
	// Strains
	int n_strains = 3;
	// For random 
	Infection infection(dt);
	// Total number of vaccinated for strain 2
	int as_2_tot = 0;
	// For saving properties of re-vaccinated
	std::ofstream fout("test_data/revac_stats.txt");

	// Create agents
	// Number of agents
	int n_agents = 50000;
	// Default agent objects
	std::vector<Agent> agents(n_agents);
	// States some agents need to be in and their probabilities
	std::map<std::string, std::pair<double, setter>> agent_states = 
		{{"removed_dead", {0.1, &Agent::set_removed_dead}},
 		 {"tested_covid_positive", {0.25, &Agent::set_tested_covid_positive}},
		 {"removed_can_vaccinate", {0.53, &Agent::set_removed_can_vaccinate}},
		 {"former_suspected", {0.3, &Agent::set_former_suspected}},
		 {"symptomatic", {0.12, &Agent::set_symptomatic}},
		 {"symptomatic_non_covid", {0.18, &Agent::set_symptomatic_non_covid}},
		 {"home_isolated", {0.23, &Agent::set_home_isolated}},
		 {"needs_next_vaccination", {0.71, &Agent::set_needs_next_vaccination}}};	

	// Vaccination functionality
	Vaccinations vac_strain_1(fname_1, data_dir_1);
	Vaccinations vac_strain_2(fname_2, data_dir_2);
	Vaccinations vac_strain_3(fname_3, data_dir_3);

	// Randomly assign age and different states
	int aID = 1;
	// Initially re-vaccinated
	std::set<int> revaccinated;
	int tot_strains = 3;
	for (auto& agent : agents) {
		// Initialize vaccination characteristics
		agent.set_n_strains(tot_strains);
		agent.initialize_benefits();
		// Assign ID
		agent.set_ID(aID);	
		// Assign age
		agent.set_age(infection.get_int(0,100));
		// Assign other states
		prob = infection.get_uniform();
		for (const auto& state : agent_states) {
			if (prob <= state.second.first) {
				if (state.first == "removed_can_vaccinate") {
					agent.set_removed_recovered(true, 2);
					if (infection.get_uniform() < 0.5) {
						(agent.*state.second.second)(false);
					} else {
						(agent.*state.second.second)(true);
					}
				} else if (state.first == "needs_next_vaccination") {
					// Vaccinate just one
					std::vector<int> temp(1, agent.get_ID()); 
					vac_strain_2.vaccinate_and_setup_time_offset(agents, 
						temp, infection, time, 0);
					// Then reset these	
					(agent.*state.second.second)(true);
					agent.set_vaccinated(true);
					revaccinated.emplace(agent.get_ID());
					// And re-vaccinate
					// Check for 0.0 and non-zero (here 10) time
					vac_strain_2.vaccinate_and_setup_time_offset(agents, 
						temp, infection, time+10.0, 1);	
					++n_vac_max;
				} else {
					(agent.*state.second.second)(true);
				}
			} else {
				(agent.*state.second.second)(false);
			}	
		}
		++aID;			
	}

	// State checking 
	std::map<std::string, getter> agent_states_check = 
		{{"removed_dead", &Agent::removed_dead},
 		 {"tested_covid_positive", &Agent::tested_covid_positive},
		 {"removed_can_vaccinate", &Agent::removed_can_vaccinate},
		 {"former_suspected", &Agent::former_suspected},
		 {"symptomatic", &Agent::symptomatic},
		 {"symptomatic_non_covid", &Agent::symptomatic_non_covid},
		 {"home_isolated", &Agent::home_isolated},
		 {"needs_next_vaccination", &Agent::needs_next_vaccination}};

	// Vaccination data
	nested_maps& vac_data_map_1 = vac_strain_1.get_vaccination_data();
	nested_maps& vac_data_map_2 = vac_strain_2.get_vaccination_data();
	nested_maps& vac_data_map_3 = vac_strain_3.get_vaccination_data();
	// Vaccination parameters
	const std::map<std::string, double>& vac_params_2 = vac_strain_2.get_vaccination_parameters();
	const std::map<std::string, double>& vac_params_3 = vac_strain_3.get_vaccination_parameters();

	// Check max eligible
	int n_eligible = vac_strain_2.max_eligible_random(agents); 
	// Initially vaccinated
	n_vac_cur_2 = vac_strain_2.vaccinate_random_time_offset(agents, n_vac_0/2, n_vac_0/2, infection, time);	
	// At this point they need to be the same (there is a sufficient number of agents)
	if (n_vac_cur_2 != n_vac_0/2) {
		std::cerr << "Wrong number of initially vaccinated" << std::endl;
		return false;
	}	
	// Check properties with time
	for (int i=0; i<n_steps; ++i) {
		for (auto& agent : agents) {
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_2, agent.get_vac_time_offset(), 2, false, 2, revaccinated, fout)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents" << std::endl;
				return false;		
			}
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_2, agent.get_vac_time_offset(), 2, true, 1, revaccinated, fout)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents for second strain 1" << std::endl;
				return false;		
			}
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map_2, agent.get_vac_time_offset(), 2, true, 3, revaccinated, fout)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents for second strain 3" << std::endl;
				return false;		
			}
		}
		// Vaccinate new batch and save offsets
		n_vac_cur_2 = vac_strain_2.vaccinate_random_time_offset(agents, n_vac/2, n_vac/2, infection, time);
		time += dt;
	}
	// Exceeding the limits
	n_eligible = vac_strain_2.max_eligible_random(agents);
	// Should vaccinate only n_eligible 
	n_vac = n_eligible + 1; 
	n_vac_cur_2 = vac_strain_2.vaccinate_random_time_offset(agents, n_vac, 1, infection, time);
	if (n_vac_cur_2 != n_eligible) {
		std::cerr << "Wrong number of agents vaccinated after the limit was exceeded" << std::endl;
		return false;
	}
	return true;
}

