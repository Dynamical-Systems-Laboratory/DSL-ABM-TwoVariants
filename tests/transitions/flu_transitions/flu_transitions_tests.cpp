#include "flu_transitions_tests.h"

/***************************************************** 
 *
 * Test suite for FluTransitions class
 *
 ******************************************************/

// Tests
bool test_susceptible_transitions();

// Supporting functions
ABM create_abm(const double dt, int i0);
bool check_isolation(const Agent&, const std::vector<School>,
				 const std::vector<RetirementHome>, const std::vector<Workplace>,
				 const std::vector<Transit>, const std::vector<Transit>,
				 bool check_in_isolation = true);
bool test_leisure(const Agent&, const std::vector<Household>&, 
				const std::vector<RetirementHome>&, const double);
bool check_fractions(int, int, double, std::string, bool need_ge = false);

int main()
{
	test_pass(test_susceptible_transitions(), "Susceptible transitions");
}

/// Series of tests for transitions of a regular susceptible agent
bool test_susceptible_transitions()
{
	// Time in days
	double dt = 0.25;
	// Max number of steps to simulate
	int tmax = 200;
	double time = 0.0, t_exp = 0.0; 
	std::vector<int> initially_infected{10000, 5, 100};
	int n_strains = 3;	
	std::string fin("test_data/input_files_all.txt");
	
	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);

	// Retrieve necessary data
    std::vector<Agent>& agents = abm.get_vector_of_agents_non_const();
	std::vector<Household>& households = abm.vector_of_households();
    std::vector<School>& schools = abm.vector_of_schools();
    std::vector<Workplace>& workplaces = abm.vector_of_workplaces();
    std::vector<Hospital>& hospitals = abm.vector_of_hospitals();
	std::vector<RetirementHome>& retirement_homes = abm.vector_of_retirement_homes();
	std::vector<Transit>& carpools = abm.vector_of_carpools();
	std::vector<Transit>& public_transit = abm.vector_of_public_transit();
	std::vector<Leisure>& leisure_locations = abm.vector_of_leisure_locations();

	Infection& infection = abm.get_infection_object();
    const std::map<std::string, double> infection_parameters = abm.get_infection_parameters(); 
    std::vector<int> state_changes(4,0);

	FluTransitions flu_tr;
	Flu& flu = abm.get_flu_object();
	Testing& testing = abm.get_testing_object(); 

	// Testing-related variables
	int n_infected = 0, n_exposed = 0;
	int n_exposed_never_sy = 0, n_tested_exposed = 0;
	int n_hsp = 0, n_car = 0; 
	int n_flu_false_pos = 0, n_flu_negative = 0, n_tested_flu = 0;
	int n_waiting_for_res = 0, n_fpos_recovered = 0;

	// To initialize Flu agents
	testing.check_switch_time(0);
	abm.check_events(schools, workplaces);
	for (int ti = 0; ti<=tmax; ++ti){
		abm.compute_place_contributions();
    	
		households = abm.get_copied_vector_of_households();
    	schools = abm.get_copied_vector_of_schools();
    	workplaces = abm.get_copied_vector_of_workplaces();
    	hospitals = abm.get_copied_vector_of_hospitals();
  		carpools = abm.get_copied_vector_of_carpools();
  		public_transit = abm.get_copied_vector_of_public_transit();
  		leisure_locations = abm.get_copied_vector_of_leisure_locations();
		retirement_homes = abm.get_copied_vector_of_retirement_homes();

		for (auto& agent : agents){
			int aID = agent.get_ID();
			if (agent.symptomatic_non_covid() == false){
				continue;
			}

			if (!test_leisure(agent, households, retirement_homes, time)) {
				std::cerr << "This agent type should not have a leisure location assigned (or was not properly removed)" << std::endl; 
				return false;
			}
			
			if (agent.infected() == false){
				// Flu - should not be in susceptible anymore
				std::vector<int> flu_susceptible = flu.get_susceptible_IDs();
				if (std::any_of(flu_susceptible.begin(), flu_susceptible.end(), [aID](int& faid){ return faid == aID; } )){
					std::cerr << "Agent with flu should not be part of future flu poll" << std::endl; 
					return false;
				}
				// Should be part of the flu group 
				std::vector<int> flu_agents = flu.get_flu_IDs();
				if (std::none_of(flu_agents.begin(), flu_agents.end(), [aID](int& faid){ return faid == aID; } )){
					std::cerr << "Agent with flu not part of flu group" << std::endl;
				}
				state_changes = flu_tr.susceptible_transitions(agent, time, infection,
					households, schools, workplaces, hospitals, retirement_homes,
				   	carpools, public_transit, leisure_locations,	
					infection_parameters, agents, flu, testing, dt, n_strains);
			}
			if (state_changes.at(0) == 0){
				// Testing flags
				if (agent.tested()){
					if ((agent.tested()) && (agent.get_time_for_flu_isolation() <= time)
                 			&& (agent.tested_awaiting_test() || agent.tested_awaiting_results())){
						// Check home isolation
						if (!agent.home_isolated()){
							std::cerr << "Agent undergoing testing but not home isolated" << std::endl;
							return false;
						}
						if (!check_isolation(agent, schools, retirement_homes, workplaces, carpools, public_transit)){
							std::cerr << "Tested agent home isolated but still present in public places" << std::endl;
							return false;
						}
					}
					// Just tested
					if (state_changes.at(1)){
						++n_tested_flu;	
						// Should still be home isolated
						if (!agent.home_isolated()){
							std::cerr << "Agent waiting for test results not home isolated" << std::endl;
							return false;
						}
						if (!check_isolation(agent, schools, retirement_homes, workplaces, carpools, public_transit)){
							std::cerr << "Agent waiting for test results home isolated but still present in public places" << std::endl;
							return false;
						}
						// Needs to be waiting for results
						if (!agent.tested_awaiting_results()){
							std::cerr << "Agent just tested is not waiting for test results" << std::endl;
							return false;
						}
						// Check testing time (can be approximate within a dt)
						if (!float_equality<double>(time, agent.get_time_of_test(), 1.1*dt)){
							  std::cerr << "Wrong testing time:\n"
							  			<< "\t\tCurrent time: " << time << " Expected: " << agent.get_time_of_test() << std::endl; 
							return false;
						}
						++n_waiting_for_res;	
					}
				}

				// Just got results 
				// Confirmed negative 
				if (state_changes.at(2)){
					++n_flu_negative;
					// Should not be home isolated 
					if (agent.home_isolated()){
						std::cerr << "Agent confirmed negative still home isolated" << std::endl;
						return false;
					}
					if (check_isolation(agent, schools, retirement_homes, workplaces, carpools, public_transit, false)){
						std::cerr << "Agent confirmed negative and still not back in public places" << std::endl;
						return false;
					}
					// Correct flag
					if (!agent.tested_covid_negative()){
						std::cerr << "Agent confirmed negative doesn't have the flag set" << std::endl;
						return false;
					}
					// Check results time 
					if (!float_equality<double>(time, agent.get_time_of_results(), 1.1*dt)){
						  std::cerr << "Wrong test results time:\n"
						  			<< "\t\tCurrent time: " << time << " Expected: " << agent.get_time_of_results() << std::endl; 
						return false;
					}
					// Flu - should not be in susceptible anymore
					const std::vector<int> flu_susceptible = flu.get_susceptible_IDs();
					for (const auto& afs : flu_susceptible){
						if (afs == aID){
							std::cerr << "Agent confirmed negative should not be part of future flu poll" << std::endl; 
							return false;
						}
					}
					// And shouldn't be in flu either
					const std::vector<int> flu_agents = flu.get_flu_IDs();
					for (const auto& afs : flu_agents){
						if (afs == aID){
							std::cerr << "Agent confirmed negative should not be part of flu group" << std::endl; 
							return false;
						}
					}
				}

				// False positive 
				if (state_changes.at(3)){
					++n_flu_false_pos;
					// Should be home isolated 
					if (!agent.home_isolated()){
						std::cerr << "Agent false positive and not home isolated" << std::endl;
						return false;
					}
					if (!check_isolation(agent, schools, retirement_homes, workplaces, carpools, public_transit)){
						std::cerr << "Agent false positive present in public places" << std::endl;
						return false;
					}
					// Correct flag
					if (!agent.tested_false_positive()){
						std::cerr << "Agent false positive doesn't have the flag set" << std::endl;
						return false;
					}
					// Check results time 
					if (!float_equality<double>(time, agent.get_time_of_results(), 1.1*dt)){
						  std::cerr << "Wrong test results time:\n"
						  			<< "\t\tCurrent time: " << time << " Expected: " << agent.get_time_of_results() << std::endl; 
						return false;
					}
					// Check recovery time validity
					if (agent.get_recovery_time() <= time){
						std::cerr << "Symptomatic agent with invalid recovery time" << std::endl;
						return false;
					}
				}

				// "Recovery"
				if (agent.tested_false_positive() && agent.get_recovery_time() <= time){
					++n_fpos_recovered;
					// No home isolation
					// Should not be home isolated 
					if (agent.home_isolated()){
						std::cerr << "False positive agent recovered still home isolated" << std::endl;
						return false;
					}
					if (check_isolation(agent, schools, retirement_homes, workplaces, carpools, public_transit, false)){
						std::cerr << "False positive agent recovered and still not back in public places" << std::endl;
						return false;
					}
					// Flu - should not be in susceptible anymore
					const std::vector<int> flu_susceptible = flu.get_susceptible_IDs();
					for (const auto& afs : flu_susceptible){
						if (afs == aID){
							std::cerr << "False positive agent recovered should not be part of future flu poll" << std::endl; 
							return false;
						}
					}
					// And shouldn't be in flu either
					const std::vector<int> flu_agents = flu.get_flu_IDs();
					for (const auto& afs : flu_agents){
						if (afs == aID){
							std::cerr << "False positive agent recovered should not be part of flu group" << std::endl; 
							return false;
						}
					}
					if (!float_equality<double>(time, agent.get_recovery_time(), 1.1*dt)){
						std::cerr << "Wrong recovery time:\n"
								  << "\t\tCurrent time: " << time << " Expected: " << agent.get_recovery_time() << " " << dt<< std::endl; 
						return false;
					}
				}
			continue;
		}

			// If infected - test and collect
			++n_infected;
			++n_exposed;
			// Flu - should not be in susceptible anymore
			const std::vector<int> flu_susceptible = flu.get_susceptible_IDs();
			for (const auto& afs : flu_susceptible){
				if (afs == aID){
					std::cerr << "Infected agent should not be part of future flu poll" << std::endl; 
					return false;
				}
			}
			// And shouldn't be in flu either
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
					return false;
				}
			}
			if (agent.tested()){
				++n_tested_exposed;
				// Time to test
				t_exp = time + infection_parameters.at("time from decision to test");
				if (!float_equality<double>(t_exp, agent.get_time_of_test(), 1.1*dt)){
					std::cerr << "Wrong test time:\n"
							  << "\t\tComputed: " << agent.get_time_of_test() << " Expected: " << t_exp << std::endl; 
					return false;
				}
				// Isolation
				if (!agent.home_isolated()){
					std::cerr << "Exposed agent undergoing testing but not home isolated" << std::endl;
					return false;
				}
				if (!check_isolation(agent, schools, retirement_homes, workplaces, carpools, public_transit)){
					std::cerr << "Exposed agent home isolated but still present in public places" << std::endl;
					return false;
				}
				// Treatment type
				if (agent.tested_in_hospital()){
					++n_hsp;
					// Hospital ID for testing
					if (agent.get_hospital_ID() <= 0){
						std::cerr << "Hospital tested agent without hospital ID" << std::endl;
						return false;
					}
				} else if (agent.tested_in_car()){
					++n_car;
				} else {
					std::cerr << "Exposed agents testing location not specified" << std::endl;
					return false;
				}
			}
			// Infectiousness variability
			if (agent.get_inf_variability_factor() < 0){
				std::cerr << "Agent's infectiousness variability out of range: " << agent.get_inf_variability_factor() << std::endl;
				return false;
			}
			// Latency and non-infectious period
			if ((agent.get_infectiousness_start_time() > agent.get_latency_end_time()) || 
					(agent.get_latency_end_time() < 0)){
				std::cerr << "Agent's non-infectious period or latency out of range" << std::endl;
				return false;
			}
		}
		abm.reset_contributions();
		abm.advance_in_time();
		time += dt;	
	}		

	// Verify the outcomes
	// Non-zero number of infected (some must have gotten infected)
	if (n_infected == 0){
		std::cerr << "No newly infected in the simulation" << std::endl;
		return false;
	}
	// Fraction "recovered" false positive
	if (n_fpos_recovered == 0){
		std::cerr << "No recovered false positive in the simulation" << std::endl;
		return false;
	}
	// Other quantities
	if (n_flu_false_pos == 0 || n_flu_negative == 0 || n_waiting_for_res == 0 
			|| n_tested_flu == 0){
		std::cerr << "Quantity that should be zero is not zero" << std::endl;
		return false;
	}
	// Fraction false negative
	if (!check_fractions(n_flu_false_pos, n_tested_flu, infection_parameters.at("fraction false positive"), 
							"Wrong percentage of agents tested false positive")){
		return false;
	}
	// Fraction that never develops symptoms (higher or equal to expected due to vaccines)
	if (!check_fractions(n_exposed_never_sy, n_exposed, 0.01, 
							"Wrong percentage of exposed agents never developing symptoms", true)){
		return false;
	}
	// Fraction tested
	if (!check_fractions(n_tested_exposed, n_exposed, 0.01, "Wrong percentage of tested agents", true)){
		return false;
	}
	// Fraction tested in hospitals
	if (!check_fractions(n_hsp, n_tested_exposed, infection_parameters.at("fraction tested in hospitals"), 
							"Wrong percentage tested in hospitals")){
		return false;
	}
	return true;
}

/// Check if agent is checked out from all the public places
bool check_isolation(const Agent& agent, const std::vector<School> schools,
				const std::vector<RetirementHome> retirement_homes,
				const std::vector<Workplace> workplaces, 
				const std::vector<Transit> carpools, 
				const std::vector<Transit> public_transit,
				bool check_in_isolation)
{
	const int aID = agent.get_ID();
   	bool not_present_school = true, not_present_work = true;
	bool not_present_transit = true;	
	bool in_isolation = true, no_isolation = false;
	if (agent.student()){
		if (find_in_place<School>(schools, aID, agent.get_school_ID())){
			not_present_school = false;
		}
	}
	if (agent.works() && !agent.works_from_home()){
		if (agent.school_employee()){
			if (find_in_place<School>(schools, aID, agent.get_work_ID())){
				not_present_work = false;
			}
		}else if (agent.retirement_home_employee()){
			if (find_in_place<RetirementHome>(retirement_homes, aID, agent.get_work_ID())){
				not_present_work = false;
			}		
		} else {
			if (find_in_place<Workplace>(workplaces, aID, agent.get_work_ID())){
				not_present_work = false;
			}
		}
		if (agent.get_work_travel_mode() == "carpool") {
			if (find_in_place<Transit>(carpools, aID, agent.get_carpool_ID())){
				not_present_transit = false;
			}
		} else if (agent.get_work_travel_mode() == "public") {
			if (find_in_place<Transit>(public_transit, aID, agent.get_public_transit_ID())){
				not_present_transit = false;
			}
		}
	}

	if (agent.student() && agent.works() && !agent.works_from_home()){
		if (agent.get_work_travel_mode() == "carpool" 
						|| agent.get_work_travel_mode() == "public") {
			no_isolation = (not_present_school == false) && (not_present_transit == false)
							&& (not_present_work == false);
			in_isolation = (not_present_school == true) && (not_present_transit == true)
							&& (not_present_work == true);
		} else {
			no_isolation = (not_present_school == false) && (not_present_work == false);
			in_isolation = (not_present_school == true) && (not_present_work == true);	
		}	
	} else if (agent.student()){
		in_isolation =  not_present_school;
		no_isolation = !in_isolation;
	} else if (agent.works() && !agent.works_from_home()){
		if (agent.get_work_travel_mode() == "carpool" 
						|| agent.get_work_travel_mode() == "public") {
			no_isolation = (not_present_transit == false) && (not_present_work == false);
			in_isolation = (not_present_transit == true) && (not_present_work == true);
		} else {
			no_isolation = (not_present_work == false);
			in_isolation = (not_present_work == true);	
		}
	} else if ((!agent.student() && !agent.works())
					|| (!agent.student() && agent.works_from_home())) {
		if (check_in_isolation) {
			in_isolation = true;
		   	no_isolation = false;	
		} else {
			in_isolation = false;
			no_isolation = true;
		}
	}	
	return (check_in_isolation ? in_isolation : !no_isolation);
}

/// Checks if leisure locations are not assigned to special types of agents
bool test_leisure(const Agent& agent, const std::vector<Household>& households, 
				const std::vector<RetirementHome>& retirement_homes, const double time)
{
	int lID = agent.get_leisure_ID();
	// All states in which agent should not participate
	if (agent.home_isolated() || agent.being_treated() 
		 	|| agent.hospitalized() || agent.hospitalized_ICU() 
			|| agent.hospital_non_covid_patient()
			|| agent.retirement_home_resident()) 
	{
		if (lID != 0) {
			return false;
		}
	}
	// Time of testing
	if ((agent.tested()) && (agent.get_time_of_test() <= time)
					     && (agent.tested_awaiting_test() == true))
	{
		if (lID != 0) {
			return false;
		}
	}
	// Passed agent - indirectly
	if (agent.removed()) {
		if (agent.retirement_home_resident()) {
			if (!find_in_place<RetirementHome>(retirement_homes, agent.get_ID(), agent.get_household_ID())) {
				if (lID != 0) {
					return false;
				}
			}
		} else {
			if (!find_in_place<Household>(households, agent.get_ID(), agent.get_household_ID())) {
				if (lID != 0) {
					return false;
				}
			}
		}
	}
	return true;
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
