#include "../../include/four_part_function.h"
#include "../../include/io_operations/abm_io.h"
#include "../common/test_utils.h"

/***************************************************** 
 *
 * Test suite for the FourPartFunction class 
 *
 *****************************************************/

// Tests
bool check_the_values();
bool past_the_limit();
bool one_value();

// Supporting functions
std::vector<std::vector<double>> read_correct(const std::string& fname);
void print_solution(const std::vector<std::vector<double>>& vec, 
						const std::string& fname);

int main()
{
	test_pass(check_the_values(), "Correct solutions - with and without the offset");
	test_pass(past_the_limit(), "Correct behavior after reaching y = 0");
	test_pass(one_value(), "Returning a constant value at all times");
}

bool check_the_values()
{
	std::string file_solution("fpf_results.txt");

	double tol = 1e-3;
	double y = 0.0;
	
	// Test 1 - no offset
	double offset = 0.0;
	const std::vector<std::vector<double>> points = {{0.0, 70.0}, {29.2929, 78.7879},
													 {49.4949, 90.0001}, {80.8081, 90.0001},
													 {100.0, 30.0004}};
	std::vector<double> temp(2,0.0);
	FourPartFunction fpf(points, offset);	
	std::vector<std::vector<double>> expected_values = read_correct(file_solution);
	std::vector<std::vector<double>> computed_values;
	for (const auto& xy_exp : expected_values) {
		// Compute
		y = fpf(xy_exp.at(0));
		// Save for printing
		temp.at(0) = xy_exp.at(0);
		temp.at(1) = y;
		computed_values.push_back(temp);
		// Check
		if (!float_equality<double>(y, xy_exp.at(1), tol)) {
			std::cerr << "Computed function value " << y
					  << " not matching expected " << xy_exp.at(1) 
					  << " at " << xy_exp.at(0)<< std::endl;
			return false;	
		} 
	}
	
	// Optionally, print the computed values
	print_solution(computed_values, "fpf_computed_no_offset.txt");		

	// Test 2 - offset (same results as no offset)
	offset = 340.0;
	FourPartFunction fpf_offset(points, offset);	
	std::vector<std::vector<double>> computed_values_offset;
	for (const auto& xy_exp : expected_values) {
		// Compute
		y = fpf_offset(xy_exp.at(0)+offset);
		// Save for printing
		temp.at(0) = xy_exp.at(0) + offset;
		temp.at(1) = y;
		computed_values_offset.push_back(temp);
		// Check
		if (!float_equality<double>(y, xy_exp.at(1), tol)) {
			std::cerr << "Computed function value " << y
					  << " not matching expected " << xy_exp.at(1) 
					  << " at " << xy_exp.at(0) + offset << std::endl;
			return false;	
		}
	}
		
	// Test 3 - negative offset (same results as no offset)
	offset = -30.0;
	FourPartFunction fpf_neg_offset(points, offset);	
	std::vector<std::vector<double>> computed_values_neg_offset;
	for (const auto& xy_exp : expected_values) {
		// Compute
		y = fpf_neg_offset(xy_exp.at(0)+offset);
		// Save for printing
		temp.at(0) = xy_exp.at(0) + offset;
		temp.at(1) = y;
		computed_values_neg_offset.push_back(temp);
		// Check
		if (!float_equality<double>(y, xy_exp.at(1), tol)) {
			std::cerr << "Computed function value " << y
					  << " not matching expected " << xy_exp.at(1) 
					  << " at " << xy_exp.at(0) + offset << std::endl;
			return false;	
		}
	}

	// Optionally, print the computed values
	print_solution(computed_values_neg_offset, "fpf_computed_with_negative_offset.txt");		

	return true;
}

/// Verifies if the behavior is correct for negative function values
bool past_the_limit()
{
	std::string file_solution("fpf_results_past_limits.txt");
	double y = 0.0;
	double offset = 340.0;
	const std::vector<std::vector<double>> points = {{0.0, 70.0}, {29.2929, 78.7879},
													 {49.4949, 90.0001}, {80.8081, 90.0001},
													 {100.0, 30.0004}};
	std::vector<double> x_values(10000, 0.0);
	std::generate(x_values.begin(), x_values.end(), [offset] () mutable { return 0.9*offset++; });	

	std::vector<double> temp(2,0.0);
	FourPartFunction fpf(points, offset);	
	std::vector<std::vector<double>> computed_values;
	for (const auto& x : x_values) {
		// Compute
		y = fpf(x);
		// Save for printing
		temp.at(0) = x;
		temp.at(1) = y;
		computed_values.push_back(temp);
		// Check - should always be larger or equal to 0.0
		if (y < 0.0) {
			std::cerr << "Function value should not be lower than 0.0 " 
					  << y << std::endl;
			return false;	
		} 
	}
	
	// Optionally, print the computed values
	print_solution(computed_values, "fpf_computed_past_limit.txt");		

	return true;
}

/// Tests the case where the part-function should always return a single value
bool one_value()
{
	const double tol = 1e-5;
	const double val = 2.57;
	double y = 0;
	FourPartFunction tpf(val);
	for (int i=0; i<1000; ++i) {
		y = tpf(static_cast<double>(i));
		if (!float_equality<double>(val, y, tol)) {
			std::cerr << "Computed function value " << y
					  << " not matching expected " << val
					  << " at " << i << std::endl;
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

/// Print current output to fname
void print_solution(const std::vector<std::vector<double>>& vec, 
						const std::string& fname)
{
	// Unused options of the AbmIO object
	std::string delim(" ");
	bool one_file = true;
	std::vector<size_t> dims = {0,0,0};
	// Save the current solution 
	AbmIO io_int(fname, delim, one_file, dims);
	io_int.write_vector<double>(vec);
}


