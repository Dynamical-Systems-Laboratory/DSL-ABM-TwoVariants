#include "../common/test_utils.h"
#include <string>
#include "../../include/io_operations/load_parameters.h"

/*************************************************************** 
 * Suite for testing LoadParameters class 
 **************************************************************/

// Tests
bool read_parameters_test();
bool read_strings_test();
bool read_age_dependent_distribution_test();
bool read_tables_test();
bool read_tables_bad_format_test();

// Supporting functions
bool equal_maps(std::map<std::string, double>&, std::map<std::string, double>&);
bool equal_maps(std::map<std::string, std::string>& expected, std::map<std::string, std::string>& loaded);
bool equal_maps(std::map<std::string, std::vector<std::vector<double>>>& expected, 
				std::map<std::string, std::vector<std::vector<double>>>& loaded);

int main()
{
	test_pass(read_parameters_test(), "Load infection parameters");
	test_pass(read_strings_test(), "Load parameters that are strings");
	test_pass(read_age_dependent_distribution_test(), "Load age-dependent distributions");
	test_pass(read_tables_test(), "Load tables from files");
	test_pass(read_tables_bad_format_test(), "Formatting error in the file with a table");
}

/// Test for loading infection parameters
bool read_parameters_test()
{
	std::map<std::string, double> expected = 
			{{"days in December", 31.0}, {"feet in meters", 3.280}, 
			 {"hours", 1.5}};
	std::map<std::string, double> loaded = {};

	LoadParameters ldp;
	loaded = ldp.load_parameter_map<double>("test_data/ldp_input.txt");

	return equal_maps(expected, loaded);
}

/// Test for loading string parameters
bool read_strings_test()
{
	std::map<std::string, std::string> expected = 
			{{"days in December", "thirtyone"}, {"feet in meters", "dunno"}, 
			 {"hours", "noon"}};
	std::map<std::string, std::string> loaded = {};

	LoadParameters ldp;
	loaded = ldp.load_parameter_map<std::string>("test_data/ldp_string_input.txt");

	return equal_maps(expected, loaded);
}

/// Test for loading infection parameters
bool read_age_dependent_distribution_test()
{
	std::map<std::string, double> expected = 
			{{"0-9", 0.001}, {"10-19", 0.003}, 
			 {"20-29", 0.012}, {"30-39", 0.032}};
	std::map<std::string, double> loaded = {};

	LoadParameters ldp;
	loaded = ldp.load_age_dependent("test_data/age_dependent_dist.txt");

	return equal_maps(expected, loaded);
}

/// Test for loading tabular data
bool read_tables_test()
{
	std::map<std::string, std::vector<std::vector<double>>> loaded = {};
	std::map<std::string, std::vector<std::vector<double>>> expected = 
			{{"number", {{1.0, 2}, {5.012, 10.89}, {2, 3.5890}}},
			 {"grade",  {{1, 5}, {2, 6}, {3, 7}}}, 
			{"grade_excellent", {{4, 8.0009}, {5.001, 9}, {5.999, 10.123545}}}};
	LoadParameters ldp;
	loaded = ldp.load_table("test_data/table_example.txt");

	// To print (optional)
/*	for (const auto& one_row : loaded) {
		std::cout << one_row.first << ": ";
		for (const auto& values : one_row.second) {
			for (const auto& val : values) {
				std::cout << val << " ";
			}
		}
		std::cout << "\n";
	}*/

	return equal_maps(expected, loaded);
}

/// Test for loading tabular data with a bad format (should throw)
bool read_tables_bad_format_test()
{
	LoadParameters ldp;
	// Expected error type
	std::invalid_argument arg_err("Wrong input file format");
	// For information from the exception wrapper
	bool verbose = true;
	if (!exception_test(verbose, &arg_err, &LoadParameters::load_table, ldp, "test_data/table_invalid.txt")){
		std::cerr << "Did not throw a bad format exception" << std::endl;
		return false;
	}
	return true;
}

/// \brief Test two maps for equality
bool equal_maps(std::map<std::string, double>& expected, std::map<std::string, double>& loaded)
{
	if (expected.size() != loaded.size()) {
		return false;
	}

	for (const auto& entry : loaded){
		const auto iter = expected.find(entry.first);
		if (iter == expected.end()){
			return false;
		} else {
			if (!float_equality<double>(iter->second, entry.second, 1e-5)) {
				return false;
			}
		}
	}

	return true;		
}

/// \brief Test two maps for equality
bool equal_maps(std::map<std::string, std::string>& expected, 
						std::map<std::string, std::string>& loaded)
{
	if (expected.size() != loaded.size()) {
		return false;
	}

	for (const auto& entry : loaded){
		const auto iter = expected.find(entry.first);
		if (iter == expected.end()){
			return false;
		} else {
			if (iter->second != entry.second) {
				return false;
			}
		}
	}

	return true;		
}

/// \brief Test two maps for equality
bool equal_maps(std::map<std::string, std::vector<std::vector<double>>>& expected, 
				std::map<std::string, std::vector<std::vector<double>>>& loaded)
{
	if (expected.size() != loaded.size()) {
		return false;
	}
	for (const auto& entry : loaded){
		const auto iter = expected.find(entry.first);
		if (iter == expected.end()){
			return false;
		} else {
			size_t ien = 0;
			for (const auto& value : entry.second) {
				if (!float_equality<double>(iter->second.at(ien).at(0), value.at(0), 1e-5)) {
					return false;
				}
				if (!float_equality<double>(iter->second.at(ien).at(1), value.at(1), 1e-5)) {
					return false;
				}
				++ien;
			}
		}
	}
	return true;		
}


