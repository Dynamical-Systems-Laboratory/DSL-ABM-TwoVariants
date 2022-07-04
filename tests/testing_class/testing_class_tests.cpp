#include "testing_class_tests.h"

/***************************************************** 
 *
 * Test suite for functionality of the Testing class
 *
 *****************************************************/

bool testing_time_dependence_test();

int main()
{
	test_pass(testing_time_dependence_test(), "Time dependence of testing - collective simulation setup");
}

bool testing_time_dependence_test()
{
	// Model parameters
	// Time in days, space in km
	double dt = 0.25;
	// Max number of steps to simulate
	int tmax = 400;
	// All strains
	std::vector<int> initially_infected{0, 5, 1};
	// Filenames and corresponding data
	std::string fin("test_data/input_files_all.txt");

	ABM abm(dt);
	abm.simulation_setup(fin, initially_infected);

	// Expected testing values
	std::vector<std::vector<double>> exp_testing = {
			{9, 0.5, 0.7, (0.2+0.89)*0.7},
			{15, 0.1, 0.5, (0.2+0.89)*0.5}, 
			{50, 0.7, 0.2, (0.2+0.89)*0.2}, 
			{70, 0.32, 0.25, (0.2+0.89)*0.25}};
	
	// Time and expected counter
	double time = 0.0;
	int ctn = 0;

	for (int ti = 0; ti<=tmax; ++ti){
		// Propagate 
		abm.transmit_infection();
		
		// Check the testing
		std::vector<double> test_values = exp_testing.at(ctn);
		Testing testing = abm.get_testing_object();

		// Check testing start
		if (!testing.started(time)){
			if (time >= exp_testing.at(ctn).at(0)){
				std::cerr << "Testing did not start when it should" << std::endl;
				return false;
			}
		}	

		// Check values
		if (testing.started(time)){
			int next = std::min(ctn+1, static_cast<int>(exp_testing.size()-1));
			if (float_equality<double>(time, exp_testing.at(next).at(0), 1e-5)){
				++ctn;
				test_values = exp_testing.at(ctn);
			}
		}
		time += dt;
	}
	return true;
}
