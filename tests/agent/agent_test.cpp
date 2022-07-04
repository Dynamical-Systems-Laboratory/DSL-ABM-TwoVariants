#include "agent_tests.h"

/***************************************************** 
 *
 * Test suite for functionality of the Agent class
 *
 *****************************************************/

// Tests
bool agent_constructor_getters_test();
bool agent_events_test();
bool agent_out_test();
bool agent_time_dependent_properties_test();

// Supporting functions
std::vector<std::vector<double>> read_correct(const std::string& fname);

int main()
{
	test_pass(agent_constructor_getters_test(), "Agent class constructor and getters");
	test_pass(agent_events_test(), "Agent class event scheduling and handling functionality");
	test_pass(agent_out_test(), "Agent class ostream operator");
	test_pass(agent_time_dependent_properties_test(), "Agent time dependent properties");
}

/// Tests Agent class constructor and most of existing getters 
// This checks only one agent type, more robust type checking 
// i.e. health state, works/studies combinations are in the
// ABM test suite  
bool agent_constructor_getters_test()
{
	bool student = true, works = false, infected = true;
	bool is_hospital_patient = false, works_at_hospital = true;
	bool res_rh = false, works_rh = false, works_school = true;
	bool works_from_home = false;
	
	int age = 25, hID = 3, sID = 305, wID = 0, hspID = 1;
	double xi = 7.009, yi = 100.5, wt_time = 10.1;
	int aID = 1, ptID = 3, cpID = 0;
	double inf_var = 0.2009;
	double cur_time = 4.0;
	std::string travel_mode("public");

	std::vector<std::map<std::string, double>> tr_rates = {{{"workplace transmission rate",2.0}, {"Home tr reate", 0.5}},
														{{"workplace transmission rate",0.3}, {"Home tr reate", 1.5}}};
	int strain_id = 1;
	int n_strains = 3;

	Agent agent(student, works, age, xi, yi, hID, is_hospital_patient, sID, 
					res_rh, works_rh, works_school, wID, works_at_hospital, hspID, infected,
					travel_mode, wt_time, cpID, ptID, works_from_home, tr_rates, n_strains);
	agent.set_ID(aID);
	agent.set_inf_variability_factor(inf_var);
	agent.set_strain(strain_id);
	
	// IDs
	if (aID != agent.get_ID() || hID != agent.get_household_ID() 
			|| sID != agent.get_school_ID() || wID != agent.get_work_ID() 
			|| hspID != agent.get_hospital_ID() || cpID != agent.get_carpool_ID()
			|| ptID != agent.get_public_transit_ID()) {
		std::cerr << "Error in one of the IDs" << std::endl;
		return false;
	}

	// Age
	if (age != agent.get_age()) {
		return false;
	}

	// Location
	if (!float_equality<double>(xi, agent.get_x_location(), 1e-5)
			|| !float_equality<double>(yi, agent.get_y_location(), 1e-5))
		return false;

	// Transmission rates
	const std::map<std::string, double>& current_rates = agent.curr_strain_tr_rates();
	const std::map<std::string, double>& exp_rates = tr_rates.at(strain_id-1);
	for (const auto& rate : current_rates) {
		if ((exp_rates.find(rate.first) == exp_rates.end())
			|| !float_equality<double>(exp_rates.at(rate.first), rate.second, 1e-5))
			return false;
	}

	// Time to work
	if (!float_equality<double>(wt_time, agent.get_work_travel_time(), 1e-5))
		return false;

	// State
	if (student != agent.student() || works != agent.works() 
			|| infected != agent.infected() 
			|| works_from_home != agent.works_from_home())
		return false;	

	// Hospital patient or healthcare professional status
	if (is_hospital_patient != agent.hospital_non_covid_patient() || 
			works_at_hospital != agent.hospital_employee())
		return false;	
		
	// Hospital patient or healthcare professional status
	if (res_rh != agent.retirement_home_resident() || 
			works_rh != agent.retirement_home_employee() ||
			works_school != agent.school_employee())
		return false;
	
	// Infectiousness variability
	if (!float_equality<double>(inf_var, agent.get_inf_variability_factor(), 1e-5))
		return false;

	// Transit mode
	if (travel_mode != agent.get_work_travel_mode())
		return false;

	// Occupation
	agent.set_occupation("C");
	agent.set_occupation_transmission();
	if (agent.get_occupation() != "C") {
		return false;
	}
	const std::vector<double>& occ_tr_rates = agent.get_occupation_transmission();
	if (!is_equal_floats<double>({{tr_rates.at(0).at("workplace transmission rate"), 
			tr_rates.at(1).at("workplace transmission rate")}}, 
			{agent.get_occupation_transmission()}, 1e-5)) {
		return false;
	}

	return true;
}

/// Check the correctness of the time-depending event handling
bool agent_events_test()
{
	bool student = true, works = false, infected = true;
	bool is_hospital_patient = false, works_at_hospital = true;
	bool res_rh = false, works_rh = false, works_school = true;
	bool works_from_home = false;
	int age = 25, hID = 3, sID = 305, wID = 0, hspID = 1;
	double xi = 7.009, yi = 100.5, wt_time = 10.1;
	int aID = 1, ptID = 3, cpID = 10;
	double inf_var = 0.2009;
	std::string travel_mode("carpool");
	int n_strains = 3;
	double cur_time = 4.0;
	double latency = 3.5, lat_end_time = cur_time + latency;
	double can_vaccinate_lag = cur_time + 11.0, recovered_to_susceptible = cur_time + 3.0; 
	double dt_inert = 5.0, inert = cur_time + dt_inert; 
	double otd = 1.2, time_of_death = cur_time + otd;
	double recovery = 10.0, time_of_recovery = cur_time + recovery; 
	double test_time = 0.5, time_of_test = cur_time + test_time;
	double test_res_time = 3.5, time_of_test_res = cur_time + test_res_time;
	double t_hsp_icu = cur_time + 2, t_hsp_ih = cur_time + 0.3, t_icu_hsp = cur_time + 11;
	double t_ih_icu = cur_time + 1, t_ih_hsp = cur_time + 3.1;  
	double vacc_end = cur_time + 5.7, active_start = cur_time + 14;

	std::vector<std::map<std::string, double>> tr_rates = {{{"Work tr rate",2.0}, {"Home tr reate", 0.5}},
														{{"Work tr rate",0.3}, {"Home tr reate", 1.5}}};

	Agent agent(student, works, age, xi, yi, hID, is_hospital_patient, sID, 
					res_rh, works_rh, works_school, wID, works_at_hospital, hspID, infected,
					travel_mode, wt_time, cpID, ptID, works_from_home, tr_rates, n_strains);

	agent.set_ID(aID);
	agent.set_inf_variability_factor(inf_var);
	
	agent.set_latency_duration(latency);
	agent.set_latency_end_time(cur_time);
	agent.set_infectiousness_start_time(cur_time, dt_inert);
	agent.set_time_recovered_can_vaccinate(can_vaccinate_lag);
	agent.set_time_recovered_to_susceptible(recovered_to_susceptible);
	agent.set_time_vaccine_effects_reduction(vacc_end);
	agent.set_time_mobility_increase(active_start);

	agent.set_time_to_death(otd);
	agent.set_death_time(cur_time);

	agent.set_recovery_duration(recovery);
	agent.set_recovery_time(cur_time);

  	agent.set_time_to_test(test_time);
  	agent.set_time_of_test(cur_time);

    agent.set_time_until_results(test_res_time); 
  	agent.set_time_of_results(cur_time);
   	
   	agent.set_time_hsp_to_icu(t_hsp_icu); 
 	agent.set_time_hsp_to_ih(t_hsp_ih); 
  	agent.set_time_icu_to_hsp(t_icu_hsp);
  	agent.set_time_ih_to_icu(t_ih_icu); 
	agent.set_time_ih_to_hsp(t_ih_hsp); 

	// Latency
	if (!float_equality<double>(lat_end_time, agent.get_latency_end_time(), 1e-5))
		return false;
	// Infectiousness 
	if (!float_equality<double>(inert, agent.get_infectiousness_start_time(), 1e-5))
		return false;

	// Death
	if (!float_equality<double>(time_of_death, agent.get_time_of_death(), 1e-5))
		return false;
	// Recovery
	if (!float_equality<double>(time_of_recovery, agent.get_recovery_time(), 1e-5))
		return false;

	// Eligible for vaccination after recovery
	if (!float_equality<double>(can_vaccinate_lag, agent.get_time_recovered_can_vaccinate(), 1e-5))
		return false;
	// Recovered losing immunity
	if (!float_equality<double>(recovered_to_susceptible, agent.get_time_recovered_to_susceptible(), 1e-5))
		return false;
	// Vaccine effects start dropping 
	if (!float_equality<double>(vacc_end, agent.get_time_vaccine_effects_reduction(), 1e-5))
		return false;
	// Agent becomes more mobile 
	if (!float_equality<double>(active_start, agent.get_time_mobility_increase(), 1e-5))
		return false;	
	
	// Testing
	if (!float_equality<double>(time_of_test, agent.get_time_of_test(), 1e-5))
		return false;
	if (!float_equality<double>(time_of_test_res, agent.get_time_of_results(), 1e-5))
		return false;
	
	// Treatment
	if (!float_equality<double>(t_hsp_icu, agent.get_time_hsp_to_icu(), 1e-5))
		return false;
	if (!float_equality<double>(t_hsp_ih, agent.get_time_hsp_to_ih(), 1e-5))
		return false;
	if (!float_equality<double>(t_icu_hsp, agent.get_time_icu_to_hsp(), 1e-5))
		return false;
	if (!float_equality<double>(t_ih_icu, agent.get_time_ih_to_icu(), 1e-5))
		return false;
	if (!float_equality<double>(t_ih_hsp, agent.get_time_ih_to_hsp(), 1e-5))
		return false;

	return true;
}


/// Tests Agent ostream operator overload/print capabilities
bool agent_out_test()
{
	bool student = true, works = false, infected = true;
	bool works_from_home = false;
	bool is_hospital_patient = false, works_at_hospital = true;
	bool res_rh = true, works_rh = false, works_school = true;
	int age = 25, hID = 3, sID = 305, wID = 0, hspID = 1;
	double xi = 7.009, yi = 100.5, wt_time = 10.1;
	int aID = 1, ptID = 3, cpID = 0;
	int n_strains = 3;
	double inf_var = 0.2009;
	std::string travel_mode("car");
	std::vector<std::map<std::string, double>> tr_rates = {{{"Work tr rate",2.0}, {"Home tr reate", 0.5}},
														{{"Work tr rate",0.3}, {"Home tr reate", 1.5}}};

	Agent agent(student, works, age, xi, yi, hID, is_hospital_patient, sID, 
					res_rh, works_rh, works_school, wID, works_at_hospital, hspID, infected,
					travel_mode, wt_time, cpID, ptID, works_from_home, tr_rates, n_strains);
	agent.set_ID(aID);

	// Get directly from the stream and compare
	std::stringstream agent_buff;
	agent_buff << agent;
	std::istringstream res(agent_buff.str());

	bool test_student = false, test_works = false, test_infected = false; 
	bool test_works_hospital = false, test_patient = false;
	bool test_rh_res = false, test_works_rh = false, test_works_school = false;
	int test_age = 0, test_hID = 0, test_sID = 0, test_wID = 0, test_hspID = 0;
	double test_xi = 0.0, test_yi = 0.0;
	int test_aID = 0;

	res >> test_aID >> test_student >> test_works >> test_age
		>> test_xi >> test_yi >> test_hID >> test_patient >> test_sID >> test_wID
		>> test_works_hospital >> test_hspID >> works_rh >> works_school >> test_rh_res >> test_infected;

	// IDs
	if (aID != test_aID || hID != test_hID || sID != test_sID || wID != test_wID || hspID != test_hspID)
		return false;
	
	// Location
	if (!float_equality<double>(xi, test_xi, 1e-5)
			|| !float_equality<double>(yi, test_yi, 1e-5))
		return false;

	// State
	if (student != test_student || works != test_works || infected != test_infected 
					|| works_at_hospital != test_works_hospital || is_hospital_patient != test_patient)
		return false;	

	return true;
}

/// Checks the correctness of time dependent functions for a single agent
bool agent_time_dependent_properties_test()
{
	bool student = true, works = false, infected = true;
	bool is_hospital_patient = false, works_at_hospital = true;
	bool res_rh = false, works_rh = false, works_school = true;
	bool works_from_home = false;

	int age = 25, hID = 3, sID = 305, wID = 0, hspID = 1;
	double xi = 7.009, yi = 100.5, wt_time = 10.1;
	int aID = 1, ptID = 3, cpID = 0;
	double inf_var = 0.2009;
	double cur_time = 4.0;
	std::string travel_mode("public");
	double tol = 1e-5;
	double time = 1.0;
	int n_strains = 3;

	std::vector<std::map<std::string, double>> tr_rates = {{{"Work tr rate",2.0}, {"Home tr reate", 0.5}},
														{{"Work tr rate",0.3}, {"Home tr reate", 1.5}}};

	Agent agent(student, works, age, xi, yi, hID, is_hospital_patient, sID, 
					res_rh, works_rh, works_school, wID, works_at_hospital, hspID, infected,
					travel_mode, wt_time, cpID, ptID, works_from_home, tr_rates, n_strains);
	agent.set_ID(aID);
	agent.set_inf_variability_factor(inf_var);

	// First check all the default values
	for (int i = 1; i<=n_strains; ++i) {
		if (agent.is_vaccinated_for_strain(i) != 0) {
			std::cout << "Initially agent should not be vaccinated for any strain" << std::endl;
			return false;
		}
		if (agent.get_vaccine_type(i) != "one_dose") {
			std::cout << "Initially agent should have their vaccine type set to one_dose" << std::endl;
			return false;
		}
		// One dose - three part functions
		agent.set_vaccine_type("one_dose", i);
		if (!float_equality<double>(agent.vaccine_effectiveness(time, i), 0.0, tol)) {
			return false;
		}
		if (!float_equality<double>(agent.asymptomatic_correction(time, i), 1.0, tol)) {
			return false;
		}
		if (!float_equality<double>(agent.transmission_correction(time, i), 1.0, tol)) {
			return false;
		}
		if (!float_equality<double>(agent.severe_correction(time, i), 1.0, tol)) {
			return false;
		}
		if (!float_equality<double>(agent.death_correction(time, i), 1.0, tol)) {
			return false;
		}
		// Two doses - default
		agent.set_vaccine_type("two_doses", i);	
		if (!float_equality<double>(agent.vaccine_effectiveness(time, i), 0.0, tol)) {
			return false;
		}
		if (!float_equality<double>(agent.asymptomatic_correction(time, i), 1.0, tol)) {
			return false;
		}
		if (!float_equality<double>(agent.transmission_correction(time, i), 1.0, tol)) {
			return false;
		}
		if (!float_equality<double>(agent.severe_correction(time, i), 1.0, tol)) {
			return false;
		}
		if (!float_equality<double>(agent.death_correction(time, i), 1.0, tol)) {
			return false;
		}
	}
	// Two doses - time dependence, only some properties for now;
	// Comprehensive testing in Vaccinations tests
	double y = 0.0;
	// No offset
	double offset = 0.0;
	const std::vector<std::vector<double>> points = {{0.0, 70.0}, {29.2929, 78.7879},
													 {49.4949, 90.0001}, {80.8081, 90.0001},
													 {100.0, 30.0004}};
	std::string file_solution("fpf_results.txt");
	std::vector<double> temp(2,0.0);
	FourPartFunction fpf(points, offset);
	// Assign a copy to this agent
	int strain_id = 2;
	agent.set_vaccine_effectiveness(fpf, strain_id);
	// Check values	
	std::vector<std::vector<double>> expected_values = read_correct(file_solution);
	for (const auto& xy_exp : expected_values) {
		// Compute
		y = agent.vaccine_effectiveness(xy_exp.at(0), strain_id);
		// Check
		if (!float_equality<double>(y, xy_exp.at(1), tol)) {
			std::cerr << "Computed function value " << y
					  << " not matching expected " << xy_exp.at(1) 
					  << " at " << xy_exp.at(0)<< std::endl;
			return false;	
		} 
	}

	// Test 2 - offset (same results as no offset)
	offset = 340.0;
	FourPartFunction fpf_offset(points, offset);
	// Assign a copy to this agent
	agent.set_vaccine_effectiveness(fpf_offset, strain_id);
	for (const auto& xy_exp : expected_values) {
		// Compute
		y = agent.vaccine_effectiveness(xy_exp.at(0)+offset, strain_id);
		// Check
		if (!float_equality<double>(y, xy_exp.at(1), tol)) {
			std::cerr << "Computed function value " << y
					  << " not matching expected " << xy_exp.at(1) 
					  << " at " << xy_exp.at(0) + offset << std::endl;
			return false;	
		}
	}

	// Test 3 - negative offset (same results as no offset)
	offset = -150.0;
	FourPartFunction fpf_neg_offset(points, offset);
	// Assign a copy to this agent
	agent.set_vaccine_effectiveness(fpf_neg_offset, strain_id);
	for (const auto& xy_exp : expected_values) {
		// Compute
		y = agent.vaccine_effectiveness(xy_exp.at(0)+offset, strain_id);
		// Check
		if (!float_equality<double>(y, xy_exp.at(1), tol)) {
			std::cerr << "Computed function value " << y
					  << " not matching expected " << xy_exp.at(1) 
					  << " at " << xy_exp.at(0) + offset << std::endl;
			return false;	
		}
	}
	
	// One dose - time, same things hold as for two doses testing
	strain_id = 1;
	agent.set_vaccine_type("one_dose", strain_id);
	file_solution = "tpf_results.txt";

	// Test 1 - no offset
	offset = 0.0;
	tol = 1e-2;
	const std::vector<std::vector<double>> tpf_points = {{0.0, 50.0}, {49.4949, 74.7475},
													 {73.7374, 74.7475}, {100.0, 73.0003}};
	ThreePartFunction tpf(tpf_points, offset);	
	expected_values = read_correct(file_solution);
	// This also tests overwriting
	agent.set_vaccine_effectiveness(tpf, strain_id);
	for (const auto& xy_exp : expected_values) {
		// Compute
		y = agent.vaccine_effectiveness(xy_exp.at(0), strain_id);
		// Check
		if (!float_equality<double>(y, xy_exp.at(1), tol)) {
			std::cerr << "Computed function value " << y
					  << " not matching expected " << xy_exp.at(1) 
					  << " at " << xy_exp.at(0)<< std::endl;
			return false;	
		} 
	}

	// Test 2 - offset (same results as no offset)
	offset = 340.0;
	ThreePartFunction tpf_offset(tpf_points, offset);
	agent.set_vaccine_effectiveness(tpf_offset, strain_id);
	for (const auto& xy_exp : expected_values) {
		// Compute
		y = agent.vaccine_effectiveness(xy_exp.at(0)+offset, strain_id);
		// Check
		if (!float_equality<double>(y, xy_exp.at(1), tol)) {
			std::cerr << "Computed function value " << y
					  << " not matching expected " << xy_exp.at(1) 
					  << " at " << xy_exp.at(0) + offset << std::endl;
			return false;	
		} 
	}

	// Test 3 - negative offset (same results as no offset)
	offset = -31.0;
	ThreePartFunction tpf_neg_offset(tpf_points, offset);
	agent.set_vaccine_effectiveness(tpf_neg_offset, strain_id);
	for (const auto& xy_exp : expected_values) {
		// Compute
		y = agent.vaccine_effectiveness(xy_exp.at(0)+offset, strain_id);
		// Check
		if (!float_equality<double>(y, xy_exp.at(1), tol)) {
			std::cerr << "Computed function value " << y
					  << " not matching expected " << xy_exp.at(1) 
					  << " at " << xy_exp.at(0) + offset << std::endl;
			return false;	
		} 
	}
	return true;
}

/// Load the correct solution from fname
std::vector<std::vector<double>> read_correct(const std::string& fname)
{
	// Unused options of the AbmIO object
	std::string delim(" ");
	bool one_file = true;
	std::vector<size_t> dims = {0,0,0};

	// Read and return the correct solution (x, y pairs)
	std::vector<std::vector<double>> solution;
	AbmIO io_int(fname, delim, one_file, dims);
	solution = io_int.read_vector<double>();

	return solution;
}
