#include "../../include/contact_tracing.h" 
#include "../../include/abm.h"
#include "../common/test_utils.h"

/***************************************************** 
 *
 * Test suite for Contact_tracing class
 *
 ******************************************************/

// Tests
bool tracking_visits_tests();
bool quarantining_household_test();
bool quarantining_visits_test();
bool quarantining_workplaces_test();
bool quarantining_hospitals_test();
bool quarantining_rh_test();
bool quarantining_schools_test();
bool quarantining_carpools_test();

int main()
{
	test_pass(tracking_visits_tests(), "Collecting visited households");
	test_pass(quarantining_household_test(), "Quarantining a household");
	test_pass(quarantining_visits_test(), "Quarantining guest households");
	test_pass(quarantining_workplaces_test(), "Quarantining workplaces");
	test_pass(quarantining_hospitals_test(), "Quarantining hospitals");
	test_pass(quarantining_rh_test(), "Quarantining retirement homes");
	test_pass(quarantining_schools_test(), "Quarantining schools");
	test_pass(quarantining_carpools_test(), "Quarantining carpools");
}

bool tracking_visits_tests()
{
	int n_agents = 100, n_houses = 1000, max_visits = 3, dt = 1;
	Contact_tracing contact_tracing(n_agents, n_houses, max_visits);

	// Add some house IDs (third argument is time of visit)
	contact_tracing.add_household(1, 1, 10);	
	contact_tracing.add_household(1, 45, 2);
	contact_tracing.add_household(45, 41, 11);
	contact_tracing.add_household(4, 789, 30);
	contact_tracing.add_household(4, 73, 500);
	contact_tracing.add_household(4, 999, 1);

	std::vector<std::deque<std::vector<int>>> locations = contact_tracing.get_private_leisure();

	int aID = 1;
	for (auto& loc : locations) {
		if (loc.empty()) {
			if (aID == 1 || aID == 45 || aID == 4) {
				std::cerr << "Select agents should not have their visits log empty" << std::endl;
				return false; 
			}
		}
		if (aID == 1) {
			if (loc.size() != 2 || loc.front().at(0) != 1 || loc.back().at(0) != 45) {
				std::cerr << "Wrong number of households or household ID for aID = 1" << std::endl;
				return false;
			}
		}
 		if (aID == 4) {
			if (loc.size() != 3) {
				std::cerr << "Wrong number of households for aID = 4" << std::endl;
				return false;
			}
			if (loc.front().at(0) != 789) {
				std::cerr << "Wrong first member for aID = 4" << std::endl;
				return false;
			}
			loc.pop_front();
			if (loc.front().at(0) != 73) {
				std::cerr << "Wrong second member for aID = 4" << std::endl;
				return false;
			}
			loc.pop_front();
			if (loc.front().at(0) != 999) {
				std::cerr << "Wrong third member for aID = 4" << std::endl;
				return false;
			}	
		}
		if (aID == 45) {
			if (loc.size() !=1 || loc.front().at(0) != 41) {
				std::cerr << "Wrong member for aID = 44" << std::endl;
				return false;
			}
		}
		++aID;
	}
 
	// Test adding past the limit
	contact_tracing.add_household(4, 879, 4);
	locations = contact_tracing.get_private_leisure();
	std::deque<std::vector<int>> loc = locations.at(3);
	if (loc.size() != 3) {
		std::cerr << "Wrong number of households for aID = 4 after adding new" << std::endl;
		return false;
	}
	if (loc.front().at(0) != 73) {
		std::cerr << "Wrong first member for aID = 4 after adding new" << std::endl;
		return false;
	}
	loc.pop_front();
	if (loc.front().at(0) != 999) {
		std::cerr << "Wrong second member for aID = 4 after adding new" << std::endl;
		return false;
	}
	loc.pop_front();
	if (loc.front().at(0) != 879) {
		std::cerr << "Wrong third member for aID = 4 after adding new" << std::endl;
		return false;
	}
	
	return true;
}

// Test for agent's household isolation
bool quarantining_household_test()
{
	// Model parameters and output
	double dt = 2.0;
	std::string fin("test_data/input_files_all.txt");
	std::vector<int> N_active{30000, 0, 0}, N_vac{10000, 0, 0}, N_boost{1000, 0, 0};
	bool vaccinate = true;
	int n_agents = 80000, n_houses = 29645, max_visits = 3;
	std::vector<int> initially_infected{0, 5, 100};
	Contact_tracing contact_tracing(n_agents, n_houses, max_visits);

	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);

	// Initialization for vaccination/reopening studies
	abm.initialize_simulations();
	// Create a COVID-19 population
	abm.initialize_active_cases(N_active, vaccinate, N_vac, N_boost);	
	Infection infection = abm.get_copied_infection_object();
	const std::vector<Household>& houses = abm.get_vector_of_households();
	const std::vector<Agent>& agents = abm.get_vector_of_agents();
	std::vector<int> traced;		
	for (const auto& hs : houses) {
		if (infection.get_uniform() < 0.4) {
			std::vector<int> agentIDs = hs.get_agent_IDs();
			if (agentIDs.empty()) {
				continue;
			}
			int na = (agentIDs.size()-1)/2;  
			traced = contact_tracing.isolate_household(agentIDs.at(na), hs);
			// Check properties
			if (std::find(traced.begin(), traced.end(), agentIDs.at(na)) != traced.end()) {
				std::cerr << "Contact traced agent should not be present "
						  << "in the return vector" << std::endl;
				return false;
			}
			for (auto& aID : traced) {
				Agent qagent = agents.at(aID-1);
				if (qagent.hospitalized() || qagent.hospitalized_ICU() 
						|| qagent.removed_dead()) {
					std::cerr << "Isolated agent is in a wrong state" << std::endl;
					return false;						
				}
			}
			if (contact_tracing.house_is_isolated(hs.get_ID())) {
				std::cerr << "This household should NOT be isolated" << std::endl;
				return false;						
			}
		}
	}
	return true;
}

// Test for agent's visited households isolation
bool quarantining_visits_test()
{
	// Model parameters and output
	double dt = 0.5;
	std::string fin("test_data/input_files_all.txt");
	std::vector<int> N_active{30000, 0, 0}, N_vac{10000, 0, 0}, N_boost{1000, 0, 0};
	bool vaccinate = true;
	int n_agents = 80000, n_houses = 29645, max_visits = 10;
	double compliance = 0.8;
	int iso_count = 0;
	std::vector<int> initially_infected{0, 5, 100};
	std::vector<std::deque<std::vector<int>>> locations;
	std::deque<std::vector<int>> cur_agent;
	// 10 max_visits and dt = 0.5 means tracking 5 days back
	int days_to_track = static_cast<int>(max_visits*dt);
	Contact_tracing contact_tracing(n_agents, n_houses, max_visits);

	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);
	// Initialization for vaccination/reopening studies
	abm.initialize_simulations();
	// Create a COVID-19 population
	abm.initialize_active_cases(N_active, vaccinate, N_vac, N_boost);	

	Infection infection = abm.get_copied_infection_object();
	std::vector<Household>& houses = abm.vector_of_households();
	std::vector<Agent>& agents = abm.vector_of_agents();
	std::vector<int> traced;

	// Run the simulation to make sure households are visited
	for (int ti=0; ti<3*max_visits; ++ti) {		
		abm.transmit_infection();
		// Add guest visits
		const std::vector<Agent>& agents = abm.get_vector_of_agents();
		for (const auto& agent : agents) {
			if (infection.get_uniform() < 0.7) {
				int hID = infection.get_int(1,n_houses);
				if (hID == agent.get_household_ID()) {
					if (hID > 1) {
						hID -= 1; 
					} else {
						hID += 1;
					}
				}
				contact_tracing.add_household(agent.get_ID(), hID, static_cast<int>(abm.get_time()));
			}
			// Isolate
			if (ti > max_visits) {
				if (infection.get_uniform() < 0.47) {
					traced = contact_tracing.isolate_visited_households(agent.get_ID(), houses, 
						compliance, infection, static_cast<int>(abm.get_time()), dt);
					// Check if agent is there
					if (std::find(traced.begin(), traced.end(), agent.get_ID()) != traced.end()) {
						std::cerr << "Contact traced agent should not be present "
								  << "in the return vector" << std::endl;
						return false;
					}
					++iso_count;	
				}
			}
		}
	}
	if (iso_count == 0) {
		std::cerr << "At least some households should end up isolated" << std::endl;
		return false;
	}
	return true;
}

// Test for agent's coworkers isolation
bool quarantining_workplaces_test()
{
	// Model parameters and output
	double dt = 2.0;
	std::string fin("test_data/input_files_all.txt");
	std::vector<int> N_active{30000, 0, 0}, N_vac{10000, 0, 0}, N_boost{1000, 0, 0};
	bool vaccinate = true;
	int n_agents = 80000, n_houses = 29645, max_visits = 3;
	std::vector<int> initially_infected{0, 5, 100};
	int n_contacts = 20;
	Contact_tracing contact_tracing(n_agents, n_houses, max_visits);

	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);
	// Initialization for vaccination/reopening studies
	abm.initialize_simulations();
	// Create a COVID-19 population
	abm.initialize_active_cases(N_active, vaccinate, N_vac, N_boost);	

	Infection infection = abm.get_copied_infection_object();
	const std::vector<Workplace>& workplaces = abm.get_vector_of_workplaces();
	const std::vector<Agent>& agents = abm.get_vector_of_agents();
	std::vector<int> traced;
	for (const auto& wk : workplaces) {
		if (wk.outside_town()) {
			continue;
		}
		if (infection.get_uniform() < 0.45) {
			std::vector<int> agentIDs = wk.get_agent_IDs();
			if (agentIDs.empty() || agentIDs.size() == 1) {
				continue;
			}
			int na = (agentIDs.size()-1)/2;  
			traced = contact_tracing.isolate_workplace(agentIDs.at(na),agents, 
						wk, n_contacts, infection);
			// Check properties
			if (traced.size() > n_contacts) {
				std::cerr << "Wrong number of coworkers isolated" << std::endl;
				return false;
			}
			if (std::find(traced.begin(), traced.end(), agentIDs.at(na)) != traced.end()) {
				std::cerr << "Contact traced agent should not be present "
						  << "in the return vector" << std::endl;
				return false;
			}
			for (auto& aID : traced) {
				Agent qagent = agents.at(aID-1);
				if (qagent.hospitalized() || qagent.hospitalized_ICU() 
						|| qagent.removed_dead()) {
					std::cerr << "Isolated agent is in a wrong state" << std::endl;
					return false;						
				}
				if (qagent.get_work_ID() != wk.get_ID()) {
					std::cerr << "Isolated agent is not in this workplace" << std::endl;
					return false;						
				}
			}
		}
	}
	return true;
}

// Test for agent's - hospital employee's - coworkers isolation
bool quarantining_hospitals_test()
{
	// Model parameters and output
	double dt = 2.0;
	std::string fin("test_data/input_files_all.txt");
	std::vector<int> N_active{30000, 0, 0}, N_vac{10000, 0, 0}, N_boost{1000, 0, 0};
	bool vaccinate = true;
	int n_agents = 80000, n_houses = 29645, max_visits = 3;
	std::vector<int> initially_infected{0, 5, 100};
	int n_contacts = 5;
	Contact_tracing contact_tracing(n_agents, n_houses, max_visits);

	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);
	// Initialization for vaccination/reopening studies
	abm.initialize_simulations();
	// Create a COVID-19 population
	abm.initialize_active_cases(N_active, vaccinate, N_vac, N_boost);	

	Infection infection = abm.get_copied_infection_object();
	const std::vector<Hospital>& hospitals = abm.get_vector_of_hospitals();
	const std::vector<Agent>& agents = abm.get_vector_of_agents();
	std::vector<int> traced;
	for (const auto& hsp : hospitals) {
		if (infection.get_uniform() < 0.45) {
			std::vector<int> agentIDs = hsp.get_agent_IDs();
			if (agentIDs.empty() || agentIDs.size() == 1) {
				continue;
			}
			int na = (agentIDs.size()-1)/2;  
			traced = contact_tracing.isolate_hospital(agentIDs.at(na),agents, 
						hsp, n_contacts, infection);
			// Check properties
			if (traced.size() > n_contacts) {
				std::cerr << "Wrong number of coworkers isolated " << std::endl;
				return false;
			}
			if (std::find(traced.begin(), traced.end(), agentIDs.at(na)) != traced.end()) {
				std::cerr << "Contact traced agent should not be present "
						  << "in the return vector" << std::endl;
				return false;
			}
			for (auto& aID : traced) {
				Agent qagent = agents.at(aID-1);
				if (qagent.hospitalized() || qagent.hospitalized_ICU() 
						|| qagent.removed_dead() || !qagent.hospital_employee()) {
					std::cerr << "Isolated agent is in a wrong state" << std::endl;
					return false;						
				}
				if (qagent.get_hospital_ID() != hsp.get_ID()) {
					std::cerr << "Isolated agent is not in this workplace" << std::endl;
					return false;						
				}
			}
		}
	}
	return true;
}

// Test for agent's - retirement home employee's - coworkers isolation
bool quarantining_rh_test()
{
	// Model parameters and output
	double dt = 2.0;
	std::string fin("test_data/input_files_all.txt");
	std::vector<int> N_active{30000, 0, 0}, N_vac{10000, 0, 0}, N_boost{1000, 0, 0};
	bool vaccinate = true;
	int n_agents = 80000, n_houses = 29645, max_visits = 3;
	std::vector<int> initially_infected{0, 5, 100};
	int n_emp = 2, n_res = 5;
	Contact_tracing contact_tracing(n_agents, n_houses, max_visits);

	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);
	// Initialization for vaccination/reopening studies
	abm.initialize_simulations();
	// Create a COVID-19 population
	abm.initialize_active_cases(N_active, vaccinate, N_vac, N_boost);	

	Infection infection = abm.get_copied_infection_object();
	const std::vector<RetirementHome>& retirement_homes = abm.get_vector_of_retirement_homes();
	const std::vector<Agent>& agents = abm.get_vector_of_agents();
	std::vector<int> traced;
	for (const auto& rh : retirement_homes) {
		if (infection.get_uniform() < 0.45) {
			std::vector<int> agentIDs = rh.get_agent_IDs();
			if (agentIDs.empty() || agentIDs.size() == 1) {
				continue;
			}
			int na = (agentIDs.size()-1)/2;  
			traced = contact_tracing.isolate_retirement_home(agentIDs.at(na), agents, 
						rh, n_emp, n_res, infection);
			// Check properties
			if (traced.size() > n_emp + n_res) {
				std::cerr << "Wrong number of coworkers and residents isolated " << std::endl;
				return false;
			}
			if (std::find(traced.begin(), traced.end(), agentIDs.at(na)) != traced.end()) {
				std::cerr << "Contact traced agent should not be present "
						  << "in the return vector" << std::endl;
				return false;
			}
			for (auto& aID : traced) {
				Agent qagent = agents.at(aID-1);
				if (qagent.hospitalized() || qagent.hospitalized_ICU() 
						|| qagent.removed_dead()) {
					std::cerr << "Isolated agent is in a wrong state" << std::endl;
					return false;						
				}
				if (qagent.retirement_home_employee() && qagent.get_work_ID() != rh.get_ID()) {
					std::cerr << "Isolated agent is not in this workplace" << std::endl;
					return false;						
				}
				if (qagent.retirement_home_resident() && qagent.get_household_ID() != rh.get_ID()) {
					std::cerr << "Isolated agent is not in this retirement home" << std::endl;
					return false;						
				}
			}
		}
	}
	return true;
}

// Test for agent's - students and teachers - isolation
bool quarantining_schools_test()
{
	// Model parameters and output
	double dt = 2.0;
	std::string fin("test_data/input_files_all.txt");
	std::vector<int> N_active{30000, 0, 0}, N_vac{10000, 0, 0}, N_boost{1000, 0, 0};
	bool vaccinate = true;
	int n_agents = 80000, n_houses = 29645, max_visits = 3;
	std::vector<int> initially_infected{0, 5, 100};
	int n_students = 5;
	Contact_tracing contact_tracing(n_agents, n_houses, max_visits);

	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);
	// Initialization for vaccination/reopening studies
	abm.initialize_simulations();
	// Create a COVID-19 population
	abm.initialize_active_cases(N_active, vaccinate, N_vac, N_boost);	

	Infection infection = abm.get_copied_infection_object();
	const std::vector<School>& schools = abm.get_vector_of_schools();
	std::vector<Agent>& agents = abm.vector_of_agents();
	std::vector<int> traced;
	for (const auto& sch : schools) {
		if (infection.get_uniform() < 0.75) {
			std::vector<int> agentIDs = sch.get_agent_IDs();
			if (agentIDs.empty() || agentIDs.size() == 1) {
				continue;
			}
			int na = (agentIDs.size()-1)/2;  
			traced = contact_tracing.isolate_school(agentIDs.at(na), agents, 
						sch, n_students, infection);
			// Check properties
			int age = 0;
			if (agents.at(agentIDs.at(na)-1).student() &&
					(agents.at(agentIDs.at(na)-1).get_school_ID() == sch.get_ID())) {
				if (traced.size() > n_students + 1) {
					std::cerr << "Wrong number of students and teachers isolated" << std::endl;
					return false;
				}
				age = agents.at(agentIDs.at(na)-1).get_age();
				int not_age = 0;
				for (auto& aID : traced) {
					if (agents.at(aID-1).get_age() != age) {
						++not_age;
					}
				}
				if (not_age != 1 && not_age > 0) {
					std::cerr << "Only one agent should have an age different than " << age  << " " << not_age<< std::endl;
					return false;
				}	
			} else {
				if (traced.size() > n_students) {
					std::cerr << "Wrong number of students isolated " << std::endl;
					return false;
				}
				age = agents.at(traced.at(0)-1).get_age();
				for (auto& aID : traced) {
					if (agents.at(aID-1).get_age() != age) {
						std::cerr << "All traced should be students of same age" << std::endl;
						return false;
					}
				}
			}
			if (std::find(traced.begin(), traced.end(), agentIDs.at(na)) != traced.end()) {
				std::cerr << "Contact traced agent should not be present "
						  << "in the return vector" << std::endl;
				return false;
			}
			for (auto& aID : traced) {
				Agent qagent = agents.at(aID-1);
				if (qagent.hospitalized() || qagent.hospitalized_ICU() 
						|| qagent.removed_dead() 
						|| (!qagent.student() && !qagent.school_employee())) {
					std::cerr << "Isolated agent is in a wrong state" << std::endl;
					return false;						
				}
			}
		}
	}
	return true;
}

// Test for agent's carpool isolation
bool quarantining_carpools_test()
{
	// Model parameters and output
	double dt = 2.0;
	std::string fin("test_data/input_files_all.txt");
	std::vector<int> N_active{30000, 0, 0}, N_vac{10000, 0, 0}, N_boost{1000, 0, 0};
	bool vaccinate = true;
	int n_agents = 80000, n_houses = 29645, max_visits = 3;
	std::vector<int> initially_infected{0, 5, 100};
	Contact_tracing contact_tracing(n_agents, n_houses, max_visits);

	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);
	// Initialization for vaccination/reopening studies
	abm.initialize_simulations();
	// Create a COVID-19 population
	abm.initialize_active_cases(N_active, vaccinate, N_vac, N_boost);	

	Infection infection = abm.get_copied_infection_object();
	const std::vector<Transit>& carpools = abm.get_vector_of_carpools();
	const std::vector<Agent>& agents = abm.get_vector_of_agents();
	std::vector<int> traced;		
	for (const auto& cp : carpools) {
		if (infection.get_uniform() < 0.7) {
			std::vector<int> agentIDs = cp.get_agent_IDs();
			if (agentIDs.empty()) {
				continue;
			}
			int na = (agentIDs.size()-1)/2;  
			traced = contact_tracing.isolate_carpools(agentIDs.at(na), agents, cp);
			// Check properties
			if (std::find(traced.begin(), traced.end(), agentIDs.at(na)) != traced.end()) {
				std::cerr << "Contact traced agent should not be present "
						  << "in the return vector" << std::endl;
				return false;
			}
			for (auto& aID : traced) {
				Agent qagent = agents.at(aID-1);
				if (qagent.hospitalized() || qagent.hospitalized_ICU() 
						|| qagent.removed_dead()) {
					std::cerr << "Isolated agent is in a wrong state" << std::endl;
					return false;						
				}
				if (qagent.get_carpool_ID() != cp.get_ID()) {
					std::cerr << "Isolated agent is not part of this carpool" << std::endl;
					return false;						
				}
			}

		}
	}
	return true;
}

