#include "../../include/mobility.h"
#include "../common/test_utils.h"

/***************************************************** 
 *
 * Test suite for functionality of the Mobility class 
 *
 *****************************************************/

// Tests
bool distance_computation_test();
bool constructing_probabilities_test();
bool constructing_probabilities_default_test();
bool assigning_locations_test();

int main()
{
	test_pass(distance_computation_test(), "Computing distances between locations of type Place");
	test_pass(constructing_probabilities_test(), "Constructing probabilities");
	test_pass(constructing_probabilities_default_test(), "Constructing probabilities - default settings");
	test_pass(assigning_locations_test(), "Assigning locations");
}

bool distance_computation_test()
{
	Mobility mobility;
	double tol = 1e-5;

	// First set
	double lat1 = -43.0, lon1 = 172.0, lat2 = -44.0, lon2 = 171.0;
	double exp_dist = 137.365669065197, dist = 0.0;	
	int strain_no = 2;	

	Household house_1(1, lat1, lon1, 0.8, 2.0, strain_no);
	Leisure leisure_1(1, lat2, lon2, 2.0, "Kinoteka", strain_no);

	dist = mobility.compute_distance(house_1, leisure_1);

	if (!float_equality<double>(exp_dist, dist, tol)){
		std::cerr << "Wrong distance between locations" << std::endl;
		return false;
	}

	// Second set
	lat2 = 20.0; 
	lon2 = -108.0;
	exp_dist = 10734.8931427602;	
	
	Leisure leisure_2(2, lat2, lon2, 2.0, "Ada Ciganlija", strain_no);
	dist = mobility.compute_distance(house_1, leisure_2);

	if (!float_equality<double>(exp_dist, dist, tol)){
		std::cerr << "Wrong distance between locations - second set" << std::endl;
		return false;
	}

	return true;
}

bool constructing_probabilities_test()
{
	double tol = 1e-5;
	int n_households = 4, n_leisure = 3; 
	double dr0 = 1.5, beta = 0.001, kappa = 400.0;
	// In lats and lons first n_households refer to households
	std::vector<double> lats = {80.0280, 14.1886, 42.1761, 91.5736, 79.2207, 95.9492, 65.5741};
	std::vector<double> lons = {5.3568, 127.3694, 140.0990, 101.8103, 113.6610, 111.4699, 58.8341};
	std::vector<std::vector<double>> exp_probs = {{9.3121e-03, 6.0285e-02, 3.7611e-03}, 
											  {1.2752e-08, 1.4194e-10, 5.6409e-09}, 
											  {2.2917e-05, 3.8474e-07, 1.2505e-06},
											  {3.2290e-02, 2.9201e-01, 8.0557e-04}};
	std::vector<std::vector<double>> probs;
	int strain_no = 3;	

	for (auto& ve : exp_probs) {
		std::partial_sum(ve.begin(), ve.end(), ve.begin());
		double max_ve = ve.back();
		std::for_each(ve.begin(), ve.end(), [&max_ve](double& x) { x /= max_ve; });
	}

	// Construct objects
	Mobility mobility;
	mobility.set_probability_parameters(dr0, beta, kappa);

	std::vector<Household> households;
	std::vector<Leisure> leisure_locations;
	for (int i=0; i<n_households; ++i) {
		households.push_back(Household(i+1, lats.at(i), lons.at(i), 0.7, 2.0, strain_no));		
	}
	int ind  = 0;
	for (int i=n_households; i<n_households + n_leisure; ++i) {
		leisure_locations.push_back(Leisure(++ind, lats.at(i), lons.at(i), 2.0, "SKC", strain_no));		
	}
	
	// Compute probabilities 
	mobility.construct_public_probabilities(households, leisure_locations);
	probs = mobility.get_public_probabilities();

	if (!is_equal_floats<double>(probs, exp_probs, tol)) {
		std::cerr << "Computed probabilities not equal expected" << std::endl;
		return false;
	}

	// To see the sampling (put print statements in mobility class) 
/*	Infection infection(0.1);
	bool is_house = false, is_public = false; 
	for (const auto& house : households) {
		std::cout << "House ID: " << house.get_ID() << std::endl;
		for (int i=0; i<3; ++i){
			mobility.assign_leisure_location(infection, house.get_ID(), is_house, 
				is_public);
			std::cout << "House: " << is_house << std::endl;
		}
	}
*/
	return true;
}

bool constructing_probabilities_default_test()
{
	double tol = 1e-5;
	int n_households = 4, n_leisure = 3; 
	// In lats and lons first n_households refer to households
	std::vector<double> lats = {80.0280, 14.1886, 42.1761, 91.5736, 79.2207, 95.9492, 65.5741};
	std::vector<double> lons = {5.3568, 127.3694, 140.0990, 101.8103, 113.6610, 111.4699, 58.8341};
	std::vector<std::vector<double>> exp_probs = {{1.7659e-08, 2.7903e-07, 5.2310e-09}, 
											  {2.2482e-15, 1.6998e-17, 9.2104e-16}, 
											  {1.0239e-11, 9.7529e-14, 3.6663e-13},
											  {1.0518e-07, 5.7290e-06, 7.3137e-10}};
	std::vector<std::vector<double>> probs;
	int strain_no = 3;
	
	for (auto& ve : exp_probs) {
		std::partial_sum(ve.begin(), ve.end(), ve.begin());
		double max_ve = ve.back();
		std::for_each(ve.begin(), ve.end(), [&max_ve](double& x) { x /= max_ve; });
	}


	// Construct objects
	Mobility mobility;

	std::vector<Household> households;
	std::vector<Leisure> leisure_locations;
	for (int i=0; i<n_households; ++i) {
		households.push_back(Household(i+1, lats.at(i), lons.at(i), 0.7, 2.0, strain_no));		
	}
	int ind  = 0;
	for (int i=n_households; i<n_households + n_leisure; ++i) {
		leisure_locations.push_back(Leisure(++ind, lats.at(i), lons.at(i), 2.0, "Strand", strain_no));		
	}
	
	// Compute probabilities 
	mobility.construct_public_probabilities(households, leisure_locations);
	probs = mobility.get_public_probabilities();

	if (!is_equal_floats<double>(probs, exp_probs, tol)) {
		std::cerr << "Computed probabilities not equal expected" << std::endl;
		return false;
	}

	return true;
}

bool assigning_locations_test()
{
	double tol = 1e-5;
	int n_households = 4, n_leisure = 3; 
	double dr0 = 1.5, beta = 0.001, kappa = 400.0;
	// In lats and lons first n_households refer to households
	std::vector<double> lats = {80.0280, 14.1886, 42.1761, 91.5736, 79.2207, 95.9492, 65.5741};
	std::vector<double> lons = {5.3568, 127.3694, 140.0990, 101.8103, 113.6610, 111.4699, 58.8341};
	std::vector<std::vector<double>> exp_probs = {{9.3121e-03, 6.0285e-02, 3.7611e-03}, 
											  {1.2752e-08, 1.4194e-10, 5.6409e-09}, 
											  {2.2917e-05, 3.8474e-07, 1.2505e-06},
											  {3.2290e-02, 2.9201e-01, 8.0557e-04}};
	std::vector<std::vector<double>> probs;
	int strain_no = 5;

	for (auto& ve : exp_probs) {
		std::sort(ve.begin(), ve.end());
	}

	// Construct objects
	Mobility mobility;
	mobility.set_probability_parameters(dr0, beta, kappa);

	std::vector<Household> households;
	std::vector<Leisure> leisure_locations;
	for (int i=0; i<n_households; ++i) {
		households.push_back(Household(i+1, lats.at(i), lons.at(i), 0.7, 2.0, strain_no));		
	}
	int ind  = 0;
	for (int i=n_households; i<n_households + n_leisure; ++i) {
		leisure_locations.push_back(Leisure(++ind, lats.at(i), lons.at(i), 2.0, "DzKC", strain_no));		
	}
	
	// Compute probabilities 
	mobility.construct_public_probabilities(households, leisure_locations);
	probs = mobility.get_public_probabilities();

	// Check correct assignments
	Infection infection;
	int max_iter = 1000000;
	int picked_ID = 0, house_ID = 0;
  	bool is_public = false;
	bool is_household = false;
	for (int i=0; i<max_iter; ++i) {
		house_ID = infection.get_random_household_ID(households.size());
		picked_ID = mobility.assign_leisure_location(infection, house_ID, is_household, is_public);	
		if ((is_public == false) && (is_household == false)) {
			std::cerr << "Both flags set to false after assignment" << std::endl;
			return false;
		}
		if ((picked_ID == 0) || ((is_household) && (picked_ID > households.size()))
					|| ((is_public) && (picked_ID > leisure_locations.size()))) {
			std::cerr << "Assigned ID is not valid" << std::endl;
			return false;
		}
	}
	return true;
}
