#include "places_tests.h"

/***************************************************** 
 *
 * Test suite for functionality of the Place class
 * 	 and its derived classes
 *
 *****************************************************/

// Tests
bool place_test();
bool school_test();
bool retirement_home_test();
bool workplace_test();
bool household_test();
bool hospital_test();
bool transit_test();
bool leisure_test();
bool workplace_transmission_changes();

// Tests for contributions
bool contribution_test_hospitals();
bool contribution_test_general_place();
bool contribution_test_workplace();
bool contribution_test_school();
bool contribution_test_household();
bool contribution_test_retirement_home();
bool contribution_test_transit();
bool contribution_test_leisure();

// Supporting functions
bool general_place_test(Place&, const int, const double,
							const double, const double, const double = -0.1,
							const double = -0.1, const std::string& = "");
bool general_contribution_test(Place& place, const double inf_var, const int tot_strains,
				const std::vector<double>& betas, const std::vector<int>& strains,
				const std::vector<int>& n_exp, const std::vector<int>& n_sym, 
				const int n_tot, const std::vector<double>& exp_lambda);
bool household_contribution_test(Household& place, const double inf_var, const int tot_strains,
				const std::vector<double>& betas, const std::vector<int>& strains,
				const std::vector<int>& n_exp, const std::vector<int>& n_sym, 
				const int n_tot, const std::vector<double>& exp_lambda);

int main()
{
	test_pass(place_test(), "Place class functionality");
	test_pass(contribution_test_general_place(), "Contribution test for places");

	test_pass(school_test(), "School class functionality");
	test_pass(contribution_test_school(), "Contribution test for schools");

	test_pass(workplace_test(), "Workplace class functionality");
	test_pass(contribution_test_workplace(), "Contribution test for workplace");

	test_pass(hospital_test(), "Hospital class functionality");
	test_pass(contribution_test_hospitals(), "Contribution test for hospitals");
	
	test_pass(household_test(), "Household class functionality");
	test_pass(contribution_test_household(), "Contribution test for household");
	
	test_pass(retirement_home_test(), "Retirement home class functionality");
	test_pass(contribution_test_retirement_home(), "Contribution test for retirement home");
	
	test_pass(workplace_transmission_changes(), "School and workplaces transmission parameters modifications");

	test_pass(transit_test(), "Transit class functionality");
	test_pass(contribution_test_transit(), "Contribution test for transit");

	test_pass(leisure_test(), "Leisure class functionality");
  	test_pass(contribution_test_leisure(), "Contribution test for leisure");

}

/// Tests all public functions from the Place class  
bool place_test()
{
	int pID = 1030; 
	double xi = 0.5, yi = 100.1;
	double severity_cor = 2.0;
	int no_strains = 2;

	Place place(pID, xi, yi, severity_cor, no_strains);

	// Check coordinate getters
	if (!float_equality<double>(xi, place.get_x(), 1e-5)){
		std::cerr << "x coordinate does not match the getter" << std::endl;
		return false;
	}
	if (!float_equality<double>(yi, place.get_y(), 1e-5)){
		std::cerr << "y coordinate does not match the getter" << std::endl;
		return false;
	}

	// Check everything else
	if (!general_place_test(place, pID, xi, yi, severity_cor)){
		return false;
	}

	// Outside town check (should always be false for now, 
	// except for workplaces that are outside)
	if (place.outside_town() == true) {
		std::cerr << "All places except for some workplaces and leisure locations should be in-town" << std::endl;
		return false;
	}

	return true;
}

/// Test contribution computation
bool contribution_test_general_place()
{
	int pID = 1030; 
	double xi = 0.5, yi = 100.1;
	double severity_cor = 2.0, inf_var = 0.9;
	std::vector<double> betas{0.47, 0.047};
	int no_strains = 2;
	std::vector<int> strains{1, 2};
	std::vector<int> n_exp{3, 3}, n_sym{6, 6}; 
	int n_tot = 30;
	std::vector<double> exp_lambda{0.0, 0.0};
	
	// Compute the expected contributions
	for (int i=0; i<strains.size(); ++i) {
		exp_lambda.at(i) = (inf_var*betas.at(i)*n_exp.at(i)
							+ severity_cor*inf_var*betas.at(i)*n_sym.at(i))/n_tot;
	}

	Place place(pID, xi, yi, severity_cor, no_strains);

	if (!general_contribution_test(place, inf_var, no_strains, 
			betas, strains, n_exp, n_sym, n_tot, exp_lambda)) {
		return false;
	}

	return true;
}

/// Tests all public functions from the School class  
bool school_test()
{
	int pID = 130; 
	double xi = 0.05, yi = 0.134;
	double severity_cor = 2.0, beta_emp = 0.9, beta = 0.47;
	double psi_emp = 0.1, psi_s = 0.25;
	int no_strains = 3;

	School school(pID, xi, yi, severity_cor, psi_emp, psi_s, no_strains);

	int ntot = 0;
	int test_pID = 0, test_ntot = 0;

	int test_ID = 0; 
	double test_xi = 0.0, test_yi = 0.0;
	double test_ck = 0.0;
	double test_psi_emp = 0.0, test_psi_s = 0.0;

	std::stringstream place_buff;
	place_buff << school;
	std::istringstream res(place_buff.str());

	res >> test_pID >> test_xi >> test_yi >> test_ntot >> test_ck >> test_psi_emp >> test_psi_s;

	// Check each value
	if (pID != test_pID || ntot != test_ntot){
		std::cerr << "ID or agent count different in output than expected" << std::endl;
		return false;
	}
	if (!float_equality<double>(xi, test_xi, 1e-5)){
		std::cerr << "x coordinates different in output than expected" << std::endl;
		return false;
	}
	if (!float_equality<double>(yi, test_yi, 1e-5)){
		std::cerr << "y coordinates different in output than expected" << std::endl;
		return false;
	}
	if (!float_equality<double>(severity_cor, test_ck, 1e-5)){
		std::cerr << "Severity correction different in output than expected" << std::endl;
		return false;
	}
	if (!float_equality<double>(psi_emp, test_psi_emp, 1e-5) && !float_equality<double>(psi_s, test_psi_s, 1e-5)){
		std::cerr << "Absenteeism correction different in output than expected" << std::endl;
		return false;
	}

	//
	// Test with agents
	//
	
	std::vector<int> agents = {909, 1, 10005};
	ntot = 3;
	for (int i=0; i<agents.size(); ++i) {
		school.register_agent(agents.at(i));
	}

	// Check if correctly registered (hardcoded)
	std::vector<int> reg_agents = school.get_agent_IDs();
	if (agents != reg_agents) {
		std::cerr << "Expected agent IDs don't match registered" << std::endl;
		return false;
	}

	//
	// Test adding and removing specific agents
	//
	
	// Addition
	int index = 201;
	school.add_agent(index);
	agents.push_back(index);
	
	reg_agents = school.get_agent_IDs();
	if (agents != reg_agents){
		std::cerr << "Error adding an agent" << std::endl;
		return false;
	}

	// Removal
	index = 1;
	std::vector<int> new_agents = {909, 10005, 201};
	school.remove_agent(index);

	reg_agents = school.get_agent_IDs();
	if (new_agents != reg_agents){
		std::cerr << "Error removing an agent" << std::endl;
		return false;
	}

	// Making sure removal when removed already doesn't change
	// anything (it is assumed not to change anything in the code)
	school.remove_agent(index);
	reg_agents = school.get_agent_IDs();
	if (new_agents != reg_agents){
		std::cerr << "Error removing an agent that was already removed" << std::endl;
		return false;
	}

	return true;
}

/// Test contribution computation
bool contribution_test_school()
{
	int pID = 130, type = 2; 
	double xi = 0.05, yi = 0.134;
	double severity_cor = 2.0, beta_emp = 0.1, beta = 0.4;
	double psi_emp = 0.3, psi_s = 0.25, inf_var = 0.9;
	int no_strains = 3;
	int n_tot = 50;

	// Student, strain 1, employee, strain 1, etc.
	std::vector<std::vector<double>> betas{{0.49, 1.5}, {0.1, 2.0}};
	std::vector<int> strains{1,2};
	std::vector<int> n_exp{3, 1}, n_sym{6, 3};
	std::vector<int> n_emp_exp{3, 1}, n_emp_sym{0, 3};

	std::vector<double> exp_lambda{0.0, 0.0};

	School school(pID, xi, yi, severity_cor, psi_emp, psi_s, no_strains);

	std::vector<double> lambda = {0.0, 0.0};

	// Compute the expected contributions
	for (int i=0; i<strains.size(); ++i) {
		exp_lambda.at(i) = (inf_var*betas.at(i).at(0)*n_exp.at(i)
							+ severity_cor*psi_s*inf_var*betas.at(i).at(0)*n_sym.at(i)
						    + inf_var*betas.at(i).at(1)*n_emp_exp.at(i)
							+ severity_cor*psi_emp*inf_var*betas.at(i).at(1)*n_emp_sym.at(i))/n_tot;
	}

	for (int i=0; i<n_tot; ++i) {
		school.add_agent(i+1);
	}

	for (int ist = 0; ist < strains.size(); ++ist) {
		int strain = strains.at(ist);
		// Students
		for (int i=0; i<(n_exp.at(ist) + n_sym.at(ist)); ++i){
			if (i < n_exp.at(ist)){
				school.add_exposed(inf_var, betas.at(ist).at(0), strain);
			}else if (i >= n_exp.at(ist)){
				school.add_symptomatic_student(inf_var, betas.at(ist).at(0), strain);
			}	
		}
		// Employees
		for (int i=0; i<(n_emp_exp.at(ist) + n_emp_sym.at(ist)); ++i){
			if (i < n_emp_exp.at(ist)){
				school.add_exposed_employee(inf_var, betas.at(ist).at(1), strain);
			}else if (i >= n_emp_exp.at(ist)){
				school.add_symptomatic_employee(inf_var, betas.at(ist).at(1), strain);
			}	
		}
	}

	school.compute_infected_contribution();
	lambda = school.get_infected_contribution();
	for (int ist = 0; ist < strains.size(); ++ist) {
		int strain = strains.at(ist);
		if (!float_equality<double>(exp_lambda.at(ist), lambda.at(strain-1), 1e-3)){
			std::cerr << "Wrong infection contribution, exp:  " << exp_lambda.at(ist) 
					  << " computed: " << lambda.at(strain-1) << std::endl;
			return false;
		}
	}

	// All but the target strains need to be zero
	for (int i=0; i<no_strains; ++i) {
		if (std::find(strains.begin(), strains.end(), i+1) == strains.end()) {
			if (!float_equality<double>(0.0, lambda.at(i), 1e-5)) {
				std::cerr << "Infection contribution for strain " << i 
						  << " should be 0.0" << std::endl;
				return false;
			}
		}	
	}

	// Zeroing
	school.reset_contributions();
	lambda = school.get_infected_contribution();
	for (int i=0; i<no_strains; ++i) {
		if (!float_equality<double>(0.0, lambda.at(i), 1e-5)){
			std::cerr << "Infection contribution not reset properly" << std::endl;
			return false;
		}
	}

	return true;
}

/// Tests all public functions from the Workplace class  
bool workplace_test()
{
	int pID = 1076; 
	double xi = 50.901, yi = 1001.675;
	double severity_cor = 2.5;
	double psi_w = 0.2, lam_out = 1.34, lam_out_new = 2.68;
	int no_strains = 3;
	std::string type = "outside";

	Workplace work(pID, xi, yi, severity_cor, psi_w, type, no_strains);

	if (!general_place_test(work, pID, xi, yi, severity_cor, -1.0, psi_w, type)){
		return false;
	}

	// Special conditions for an outside workplace
	work.set_outside_infected(lam_out);
	if (!float_equality<double>(work.get_outside_infected(), lam_out, 1e-5)){
		std::cerr << "Wrong outside lambda" << std::endl;
		return false;
	}
	work.adjust_outside_infected(2.0);
	if (!float_equality<double>(work.get_outside_infected(), lam_out_new, 1e-5)){
		std::cerr << "Wrong outside lambda after adjusting" << std::endl;
		return false;
	}
	if (work.outside_town() == false) {
		std::cerr << "Workplace should be out-of town" << std::endl;
		return false;
	}

	// And one in town
	Workplace in_town_work(pID+1, xi, yi, severity_cor, psi_w, "F", no_strains);
	if (in_town_work.outside_town() == true) {
		std::cerr << "Workplace wrongly labelled as out-of town" << std::endl;
		return false;
	}
	
	return true;
}

/// Test contribution computation
bool contribution_test_workplace()
{
	int pID = 130; 
	double xi = 0.05, yi = 0.134;
	double severity_cor = 2.5;
	double psi_w = 0.2, inf_var = 1.3;
	std::string wtype = "G";

	int no_strains = 3;
	std::vector<double> betas{0.49};
	std::vector<int> strains{3};
	std::vector<int> n_exp{3}, n_sym{6};
	std::vector<double> exp_lambda{0.0};
	int n_tot = 10;

	// Compute the expected contributions
	for (int i=0; i<strains.size(); ++i) {
		exp_lambda.at(i) = (inf_var*betas.at(i)*n_exp.at(i)
							+ severity_cor*psi_w*inf_var*betas.at(i)*n_sym.at(i))/n_tot;
	}

	Workplace work(pID, xi, yi, severity_cor, psi_w, wtype, no_strains);

	if (!general_contribution_test(work, inf_var, no_strains, 
			betas, strains, n_exp, n_sym, n_tot, exp_lambda)) {
		return false;
	}

	return true;
}

/// Tests all public functions from the Hospital class  
bool hospital_test()
{
	int pID = 10760; 
	double xi = 5.901, yi = 10010.675;
	double severity_cor = 2.5;
	int no_strains = 3;
	Hospital hospital(pID, xi, yi, severity_cor, no_strains);
	
	if (!general_place_test(hospital, pID, xi, yi, severity_cor)){
		return false;
	}

	return true;
}

/// Test computation of probability contribution in hospitals
bool contribution_test_hospitals()
{
	int pID = 10760; 
	double xi = 5.901, yi = 10010.675;
	int no_strains = 10;
	int n_tot = 500;

	// Employee, strain 1, non-covid patient, strain 1, 
	// covid patient, strain 1, icu patient, strain 1, tested, strain 1
	std::vector<std::vector<double>> betas{{0.49, 1.0, 2.0, 3.0, 0.1}, 
										{0.1, 2.0, 1.2, 0.9, 0.2}};
	std::vector<int> strains{3, 9};
	// Same for all for simplicity
	std::vector<int> n_exp{3, 1}, n_sym{6, 3};

	std::vector<double> exp_lambda{0.0, 0.0};
	std::vector<double> lambda = {0.0, 0.0};

	double severity_cor = 2.5;
	double inf_var = 0.2;
	
	// Check contributions
	// Set total number of agents
	Hospital hospital(pID, xi, yi, severity_cor, no_strains);

	// Compute the expected contributions
	for (int i=0; i<strains.size(); ++i) {
		exp_lambda.at(i) = (inf_var*betas.at(i).at(0)*n_exp.at(i)
							+ severity_cor*inf_var*betas.at(i).at(2)*n_sym.at(i)
						    + inf_var*betas.at(i).at(1)
							+ severity_cor*inf_var*betas.at(i).at(4)
							+ severity_cor*inf_var*betas.at(i).at(3))/n_tot;
	}

	for (int i=0; i<n_tot; ++i){
		hospital.add_agent(i+1);
	}

	for (int ist = 0; ist < strains.size(); ++ist) {
		int strain = strains.at(ist);
		for (int i=0; i<(n_exp.at(ist)+n_sym.at(ist)); ++i){
			if (i < n_exp.at(ist)){
				hospital.add_exposed(inf_var, betas.at(ist).at(0), strain);
			}else if (i >= n_exp.at(ist)){
				hospital.add_hospitalized(inf_var, betas.at(ist).at(2), strain);
			}	
		}
		// The rest has only one agent
		hospital.add_exposed_patient(inf_var, betas.at(ist).at(1), strain);
		hospital.add_hospital_tested(inf_var, betas.at(ist).at(4), strain);
		hospital.add_hospitalized_ICU(inf_var, betas.at(ist).at(3), strain);
	}

	hospital.compute_infected_contribution();
	lambda = hospital.get_infected_contribution();
	for (int ist = 0; ist < strains.size(); ++ist) {
		int strain = strains.at(ist);
		if (!float_equality<double>(exp_lambda.at(ist), lambda.at(strain-1), 1e-3)){
			std::cerr << "Wrong infection contribution, exp:  " << exp_lambda.at(ist) 
					  << " computed: " << lambda.at(strain-1) << std::endl;
			return false;
		}
	}
	// All but the target strains need to be zero
	for (int i=0; i<no_strains; ++i) {
		if (std::find(strains.begin(), strains.end(), i+1) == strains.end()) {
			if (!float_equality<double>(0.0, lambda.at(i), 1e-5)) {
				std::cerr << "Infection contribution for strain " << i 
						  << " should be 0.0" << std::endl;
				return false;
			}
		}	
	}
	// Zeroing
	hospital.reset_contributions();
	lambda = hospital.get_infected_contribution();
	for (int i=0; i<no_strains; ++i) {
		if (!float_equality<double>(0.0, lambda.at(i), 1e-5)){
			std::cerr << "Infection contribution not reset properly" << std::endl;
			return false;
		}
	}

	return true;
}

/// Tests all public functions from the Household class  
bool household_test()
{
	int pID = 176; 
	double xi = 5.95671, yi = 11.00675;
	double severity_cor = 3.5;
	double alpha = 0.7;
	int no_strains = 30;

	Household house(pID, xi, yi, alpha, severity_cor, no_strains);

	// All other tests including the default contribution
	if (!general_place_test(house, pID, xi, yi, severity_cor, alpha)){
		return false;
	}
	return true;
}

/// Test household infection contribution computation
bool contribution_test_household()
{
	int pID = 176; 
	double xi = 5.95671, yi = 11.00675;
	double severity_cor = 3.5;
	double alpha = 0.7, inf_var = 0.345;
	int no_strains = 30;
	std::vector<double> betas{0.47, 1.1};
	std::vector<int> strains{21, 30};
	std::vector<int> n_exp{3, 8}, n_sym{1, 6}; 
	int n_tot = 50;
	std::vector<double> exp_lambda{0.0, 0.0};
	
	// Compute the expected contributions
	for (int i=0; i<strains.size(); ++i) {
		exp_lambda.at(i) = (inf_var*betas.at(i)*n_exp.at(i)
							+ severity_cor*inf_var*betas.at(i)
								*n_sym.at(i)
							+ severity_cor*inf_var*betas.at(i))/std::pow(n_tot, alpha);
	}

	Household house(pID, xi, yi, alpha, severity_cor, no_strains);

	if (!household_contribution_test(house, inf_var, no_strains, 
			betas, strains, n_exp, n_sym, n_tot, exp_lambda)){
		return false;
	}

	return true;
}

/// Tests all public functions from the RetirementHome class  
bool retirement_home_test()
{
	int pID = 10760; 
	double xi = 5.901, yi = 10010.675;
	double severity_cor = 2.5;
	double psi_e = 0.3;
	int no_strains = 30;

	RetirementHome retirement_home(pID, xi, yi, severity_cor, psi_e, no_strains);
	
	if (!general_place_test(retirement_home, pID, xi, yi, severity_cor, -1.0, psi_e)) {
		return false;
	}

	return true;
}

/// Test retirement home contribution computation
bool contribution_test_retirement_home()
{
	int pID = 10760; 
	double xi = 5.901, yi = 10010.675;
	double severity_cor = 2.5;
	double inf_var = 0.345, psi = 0.1;
	int no_strains = 130;
	// Resident, employee, home isolated
	std::vector<std::vector<double>> betas = {{0.47, 1.1, 0.2}, 
									{4.7, 0.1, 1.2}, {0.7, 1.1, 2.2}};
	std::vector<int> strains{1, 101, 120};
	std::vector<int> n_exp{3, 18, 0}, n_sym{0, 16, 50}; 
	int n_tot = 300;
	std::vector<double> exp_lambda{0.0, 0.0, 0.0};
	std::vector<double> lambda;

	// Compute the expected contributions
	for (int i=0; i<strains.size(); ++i) {
		exp_lambda.at(i) = (inf_var*betas.at(i).at(0)*n_exp.at(i)
							+ severity_cor*inf_var*betas.at(i).at(0)*n_sym.at(i)
							+ inf_var*betas.at(i).at(2)*n_exp.at(i)
							+ severity_cor*inf_var*betas.at(i).at(2)*n_sym.at(i)
							+ inf_var*betas.at(i).at(1)*n_exp.at(i)
							+ severity_cor*inf_var*psi*betas.at(i).at(1)*n_sym.at(i))/n_tot;
	}
	
	RetirementHome retirement_home(pID, xi, yi, severity_cor, psi, no_strains);
	
	for (int i=0; i<n_tot; ++i){
		retirement_home.add_agent(i+1);
	}

	for (int ist = 0; ist < strains.size(); ++ist) {
		int strain = strains.at(ist);
		for (int i=0; i<(n_exp.at(ist) + n_sym.at(ist)); ++i){
			if (i < n_exp.at(ist)){
				retirement_home.add_exposed(inf_var, betas.at(ist).at(0), strain);
				retirement_home.add_exposed_employee(inf_var, betas.at(ist).at(1), strain);
				retirement_home.add_exposed_home_isolated(inf_var, betas.at(ist).at(2), strain);
			}else if (i >= n_exp.at(ist)){
				retirement_home.add_symptomatic(inf_var, betas.at(ist).at(0), strain);
				retirement_home.add_symptomatic_employee(inf_var, betas.at(ist).at(1), strain);
				retirement_home.add_symptomatic_home_isolated(inf_var, betas.at(ist).at(2), strain);
			}	
		}
	}

	retirement_home.compute_infected_contribution();
	lambda = retirement_home.get_infected_contribution();

	for (int ist = 0; ist < strains.size(); ++ist) {
		int strain = strains.at(ist);
		if (!float_equality<double>(exp_lambda.at(ist), lambda.at(strain-1), 1e-3)){
			std::cerr << "Wrong infection contribution, exp:  " << exp_lambda.at(ist) 
					  << " computed: " << lambda.at(strain-1) << std::endl;
			return false;
		}
	}

	// All but the target strains need to be zero
	for (int i=0; i<no_strains; ++i) {
		if (std::find(strains.begin(), strains.end(), i+1) == strains.end()) {
			if (!float_equality<double>(0.0, lambda.at(i), 1e-5)) {
				std::cerr << "Infection contribution for strain " << i 
						  << " should be 0.0" << std::endl;
				return false;
			}
		}	
	}

	// Zeroing
	retirement_home.reset_contributions();
	lambda = retirement_home.get_infected_contribution();
	for (int i=0; i<no_strains; ++i) {
		if (!float_equality<double>(0.0, lambda.at(i), 1e-5)){
			std::cerr << "Infection contribution not reset properly" << std::endl;
			return false;
		}
	}

	return true;
}

/// Tests all public functions from the Transit class  
bool transit_test()
{
	int pID = 1016; 
	double severity_cor = 1.5, psi = 0.25;
	int no_strains = 103;
	std::string type = "carpool";

	Transit transit(pID, severity_cor, psi, type, no_strains);

	if (!general_place_test(transit, pID, 0.0, 0.0, severity_cor, -1.0, psi, type)){
		return false;
	}

	return true;
}

/// Test contribution computation
bool contribution_test_transit()
{
	int pID = 130; 
	double xi = 0.0, yi = 0.0;
	double severity_cor = 2.25, psi = 0.5;
	double inf_var = 1.23;
	std::string type = "public";

	int no_strains = 103;	

	std::vector<double> betas{0.49, 0.9, 0.1};
	std::vector<int> strains{3, 100, 103};
	std::vector<int> n_exp{3, 1, 0}, n_sym{6, 5, 20};
	std::vector<double> exp_lambda{0.0, 0.0, 0.0};
	int n_tot = 100;

	// Compute the expected contributions
	for (int i=0; i<strains.size(); ++i) {
		exp_lambda.at(i) = (inf_var*betas.at(i)*n_exp.at(i)
							+ severity_cor*psi*inf_var*betas.at(i)*n_sym.at(i))/n_tot;
	}

	Transit transit(pID, severity_cor, psi, type, no_strains);

	if (!general_contribution_test(transit, inf_var, no_strains, 
			betas, strains, n_exp, n_sym, n_tot, exp_lambda)) {
		return false;
	}

	return true;
}

/// Tests all public functions from the Leisure class  
bool leisure_test()
{
	int pID = 1016;
	double xi = 0.0, yi = 10.0;
	double severity_cor = 1.5, lam_out = 1.34, lam_out_new = 2.68;
	int no_strains = 309;
	std::string type = "outside";

	Leisure leisure(pID, xi, yi, severity_cor, type, no_strains);

	if (!general_place_test(leisure, pID, xi, yi, severity_cor, -1.0, -1.0, type)){
		return false;
	}
	
	// Special conditions for an outside leisure location
	leisure.set_outside_infected(lam_out);
	if (!float_equality<double>(leisure.get_outside_infected(), lam_out, 1e-5)){
		std::cerr << "Wrong outside lambda" << std::endl;
		return false;
	}
	leisure.adjust_outside_infected(2.0);
	if (!float_equality<double>(leisure.get_outside_infected(), lam_out_new, 1e-5)){
		std::cerr << "Wrong outside lambda after adjusting" << std::endl;
		return false;
	}
	if (leisure.outside_town() == false) {
		std::cerr << "Leisure location should be out-of town" << std::endl;
		return false;
	}

	// And one in town
	Leisure in_town_lsr(pID+1, xi, yi, severity_cor, "F", no_strains);
	if (in_town_lsr.outside_town() == true) {
		std::cerr << "Leisure location wrongly labelled as out-of town" << std::endl;
		return false;
	}
	return true;
}

/// Test contribution computation
bool contribution_test_leisure()
{
	int pID = 130; 
	double xi = 0.0, yi = 0.0;
	double severity_cor = 2.25, beta = 1.49;
	double inf_var = 1.23;
	std::string type = "M";
	int no_strains = 309;

	std::vector<double> betas{1.49, 0.01};
	std::vector<int> strains{1, 308};
	std::vector<int> n_exp{30, 10}, n_sym{0, 1};
	std::vector<double> exp_lambda{0.0, 0.0};
	int n_tot = 1000;

	// Compute the expected contributions
	for (int i=0; i<strains.size(); ++i) {
		exp_lambda.at(i) = (inf_var*betas.at(i)*n_exp.at(i)
							+ severity_cor*inf_var*betas.at(i)*n_sym.at(i))/n_tot;
	}

	Leisure leisure(pID, xi, yi, severity_cor, type, no_strains);

	if (!general_contribution_test(leisure, inf_var, no_strains, 
			betas, strains, n_exp, n_sym, n_tot, exp_lambda)) {
		return false;
	}

	return true;
}

/** 
 * \brief Test for general contribution to infection probability
 * 
 * @param place - place object
 * @param inf_var - infection variability
 * @param n_exp - number of exposed for each non-zero strain
 * @param n_sym - number of symptomatic for each non-zero strain 
 * @param n_tot - total number of agents
 * @param exp_lambda - expected lambda for each non-zero strain
 */
bool general_contribution_test(Place& place, const double inf_var, const int tot_strains,
				const std::vector<double>& betas, const std::vector<int>& strains,
				const std::vector<int>& n_exp, const std::vector<int>& n_sym, 
				const int n_tot, const std::vector<double>& exp_lambda)
{
	std::vector<double> lambda;

	for (int i=0; i<n_tot; ++i) {
		place.add_agent(i+1);
	}

	for (int ist = 0; ist < strains.size(); ++ist) {
		int strain = strains.at(ist);
		for (int i=0; i<(n_exp.at(ist) + n_sym.at(ist)); ++i){
			if (i < n_exp.at(ist)){
				place.add_exposed(inf_var, betas.at(ist), strain);
			}else if (i >= n_exp.at(ist)){
				place.add_symptomatic(inf_var, betas.at(ist), strain);
			}	
		}
	}

	place.compute_infected_contribution();
	lambda = place.get_infected_contribution();
	for (int ist = 0; ist < strains.size(); ++ist) {
		int strain = strains.at(ist);
		if (!float_equality<double>(exp_lambda.at(ist), lambda.at(strain-1), 1e-3)){
			std::cerr << "Wrong infection contribution, exp:  " << exp_lambda.at(ist) 
					  << " computed: " << lambda.at(strain-1) << std::endl;
			return false;
		}
	}

	// All but the target strains need to be zero
	for (int i=0; i<tot_strains; ++i) {
		if (std::find(strains.begin(), strains.end(), i+1) == strains.end()) {
			if (!float_equality<double>(0.0, lambda.at(i), 1e-5)) {
				std::cerr << "Infection contribution for strain " << i 
						  << " should be 0.0" << std::endl;
				return false;
			}
		}	
	}

	// Zeroing
	place.reset_contributions();
	lambda = place.get_infected_contribution();
	for (int i=0; i<tot_strains; ++i) {
		if (!float_equality<double>(0.0, lambda.at(i), 1e-5)){
			std::cerr << "Infection contribution not reset properly" << std::endl;
			return false;
		}
	}

	return true;
}

/// Test for most classes derived from Place including itself
bool general_place_test(Place& place, const int pID, 
		const double xi, const double yi, const double ck, 
		const double alpha, const double psi, const std::string& type)
{
	//
	// Test if correct without agents
	// Uses streams to avoid going through files
	//
	
	int ntot = 0, ninf = 0;
	int test_pID = 0, test_ntot = 0; 
	double test_x = 0.0, test_y = 0.0;
	double test_ck = 0.0;
	double test_alpha = 0.0, test_psi = 0.0;
	std::string test_type;

	std::stringstream place_buff;
	place_buff << place;
	std::istringstream res(place_buff.str());

	// If not a house, school, or workplace
	if (alpha < 0.0 && psi < 0.0){
		res >> test_pID >> test_x >> test_y >> test_ntot >> test_ck;
		if (!type.empty()) {
			res >> test_type;
		}
	}
	// If a house
	if (alpha > 0.0) {
		res >> test_pID >> test_x >> test_y >> test_ntot >> test_ck >> test_alpha;
	}
	// If a workplace or a retirement home
	if (psi > 0.0){
		res >> test_pID >> test_x >> test_y >> test_ntot >> test_ck >> test_psi;
		if (!type.empty()) {
			res >> test_type;
		}
	}

	// Check each value
	if (pID != test_pID || ntot != test_ntot){
		std::cerr << "ID or agent count different in output than expected" << test_pID << " " << pID << std::endl;
		return false;
	}
	if (!float_equality<double>(xi, test_x, 1e-5)){
		std::cerr << "x coordinates different in output than expected" << std::endl;
		return false;
	}
	if (!float_equality<double>(yi, test_y, 1e-5)){
		std::cerr << "y coordinates different in output than expected" << std::endl;
		return false;
	}
	if (!float_equality<double>(ck, test_ck, 1e-5)){
		std::cerr << "Severity correction different in output than expected" << std::endl;
		return false;
	}
	// If a house
	if (alpha > 0.0 && !float_equality<double>(alpha, test_alpha, 1e-5)){
		std::cerr << "Household scaling factor different in output than expected" << std::endl;
		return false;
	}
	// If a school or workplace
	if (psi > 0.0 && !float_equality<double>(psi, test_psi, 1e-5)){
		std::cerr << "Absenteeism correction different in output than expected" << std::endl;
		return false;
	}
	// If has a type
	if (!type.empty()) {
		if (type != test_type) {
			std::cerr << "Type of tested object does not match expected type" << std::endl;
			return false;
		}
	}

	//
	// Test with agents
	//
	
	std::vector<int> agents = {909, 1, 10005};
	ntot = 3;
	for (int i=0; i<agents.size(); ++i) {
		place.register_agent(agents.at(i));
	}

	// Check if correctly registered (hardcoded)
	std::vector<int> reg_agents = place.get_agent_IDs();
	if (agents != reg_agents) {
		std::cerr << "Expected agent IDs don't match registered" << std::endl;
		return false;
	}		

	//
	// Test adding and removing specific agents
	//
	
	// Addition
	int index = 201;
	place.add_agent(index);
	agents.push_back(index);
	
	reg_agents = place.get_agent_IDs();
	if (agents != reg_agents){
		std::cerr << "Error adding an agent" << std::endl;
		return false;
	}

	// Removal
	index = 1;
	std::vector<int> new_agents = {909, 10005, 201};
	place.remove_agent(index);

	reg_agents = place.get_agent_IDs();
	if (new_agents != reg_agents){
		std::cerr << "Error removing an agent" << std::endl;
		return false;
	}

	// Making sure removal when removed already doesn't change
	// anything (it is assumed not to change anything in the code)
	place.remove_agent(index);
	reg_agents = place.get_agent_IDs();
	if (new_agents != reg_agents){
		std::cerr << "Error removing an agent that was already removed" << std::endl;
		return false;
	}

	return true;
}

/** 
 * \brief Test for household contribution to infection probability
 * 
 * @param place - place object
 * @param inf_var - infection variability
 * @param n_exp - number of exposed for each non-zero strain
 * @param n_sym - number of symptomatic for each non-zero strain 
 * @param n_tot - total number of agents
 * @param exp_lambda - expected lambda for each non-zero strain
 */
bool household_contribution_test(Household& household, const double inf_var, const int tot_strains,
				const std::vector<double>& betas, const std::vector<int>& strains,
				const std::vector<int>& n_exp, const std::vector<int>& n_sym, 
				const int n_tot, const std::vector<double>& exp_lambda)
{
	std::vector<double> lambda;

	for (int i=0; i<n_tot; ++i) {
		household.add_agent(i+1);
	}

	for (int ist = 0; ist < strains.size(); ++ist) {
		int strain = strains.at(ist);
		for (int i=0; i<(n_exp.at(ist) + n_sym.at(ist)); ++i){
			if (i < n_exp.at(ist)){
				household.add_exposed(inf_var, betas.at(ist), strain);
			}else if (i >= n_exp.at(ist)){
				household.add_symptomatic(inf_var, betas.at(ist), strain);
			}	
		}
		household.add_symptomatic_home_isolated(inf_var, betas.at(ist), strain);
	}

	household.compute_infected_contribution();
	lambda = household.get_infected_contribution();

	for (int ist = 0; ist < strains.size(); ++ist) {
		int strain = strains.at(ist);
		if (!float_equality<double>(exp_lambda.at(ist), lambda.at(strain-1), 1e-3)){
			std::cerr << "Wrong infection contribution, exp:  " << exp_lambda.at(ist) 
					  << " computed: " << lambda.at(strain-1) << std::endl;
			return false;
		}
	}

	// All but the target strains need to be zero
	for (int i=0; i<tot_strains; ++i) {
		if (std::find(strains.begin(), strains.end(), i+1) == strains.end()) {
			if (!float_equality<double>(0.0, lambda.at(i), 1e-5)) {
				std::cerr << "Infection contribution for strain " << i 
						  << " should be 0.0" << std::endl;
				return false;
			}
		}	
	}

	// Zeroing
	household.reset_contributions();
	lambda = household.get_infected_contribution();
	for (int i=0; i<tot_strains; ++i) {
		if (!float_equality<double>(0.0, lambda.at(i), 1e-5)){
			std::cerr << "Infection contribution not reset properly" << std::endl;
			return false;
		}
	}

	return true;
}

/// Tests if transmission parameters are properly modified
bool workplace_transmission_changes()
{
	int pID = 1076; 
	double xi = 50.901, yi = 1001.675;
	double severity_cor = 2.5;
	double psi_w = 0.2;
	double new_psi = 0.7; 
	std::string type = "K";
	int no_strains = 1001;

	Workplace work(pID, xi, yi, severity_cor, psi_w, type, no_strains);

	work.change_absenteeism_correction(new_psi);

	if (!float_equality<double>(work.get_absenteeism_correction(), new_psi, 1e-5)){
		std::cerr << "Workplace abseteeism correction not properly changed" << std::endl;
		return false;
	}

	return true;
}

