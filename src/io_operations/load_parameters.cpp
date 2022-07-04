#include "../../include/io_operations/load_parameters.h"

/***************************************************** 
 * class: LoadParameters 
 * 
 * Class for reading and storing parameters from file 
 * 
 *****************************************************/

// Read parameters from file, store them as a map
template<>
std::map<std::string, double> LoadParameters::load_parameter_map<double>(const std::string& infile)
{
	FileHandler file(infile);
	std::fstream &in = file.get_stream();
	std::string line;

	std::map<std::string, double> parameters;

	std::string tag = {}, word = {};

	while (std::getline(in, line)){
		
		std::istringstream data_row(line);

		// If a comment with a tag, save all words
		if (line.find("//") != std::string::npos){
			// Skip the // entry and blank line, 
			// then collect the rest
			data_row >> word;
			while (data_row >> word)
				tag += (word + " ");
			// Remove the extra blank character
			if (!tag.empty())
				tag.pop_back();
			continue;
		}

		// If not - collect the number, make a map entry,
		// and reset the tag
		data_row >> word;
		parameters[tag] = std::stod(word);
		tag.clear();
	}
	return parameters;
}

// Read parameters/tokens from file, store the as a map
template<>
std::map<std::string, std::string> LoadParameters::load_parameter_map(const std::string& infile)
{
	FileHandler file(infile);
	std::fstream &in = file.get_stream();
	std::string line;

	std::map<std::string, std::string> parameters;
	std::string tag = {}, word = {};
	while (std::getline(in, line)){
		
		std::istringstream data_row(line);

		// If a comment with a tag, save all words
		if (line.find("//") != std::string::npos){
			// Skip the // entry and a blank, 
			// then collect the rest
			data_row >> word;
			while (data_row >> word) {
				tag += (word + " ");
			}
			// Remove the extra blank character
			if (!tag.empty()) {
				tag.pop_back();
			}
			continue;
		}

		// If not - collect the entry and reset the tag
		data_row >> word;
		parameters[tag] = word;
		tag.clear();
	}
	return parameters;
}

// Read age-dependent distributions as map, store the as a map
std::map<std::string, double> LoadParameters::load_age_dependent(const std::string& infile)
{
	FileHandler file(infile);
	std::fstream &in = file.get_stream();
	std::string line;

	std::map<std::string, double> age_distribution;

	std::string tag = {}, word = {};
	double value = 0.0;

	while (std::getline(in, line)) {
		std::istringstream data_row(line);

		// Read the key (age range of some form)
		data_row >> word;
		tag = word;

		// And the value (value for that age range)
		data_row >> word;
		value = std::stod(word);

		// Assign
		age_distribution[tag] = value;
	}

	return age_distribution;
}

// Reads parameters in a tabular form 
std::map<std::string, std::vector<std::vector<double>>> LoadParameters::load_table(const std::string& infile)
{
	FileHandler file(infile);
	std::fstream &in = file.get_stream();
	std::string line;
	std::string tag, pair;		
	std::string::size_type ind = 0;
	std::map<std::string, std::vector<std::vector<double>>> whole_table;

	while (std::getline(in, line)) {
		// Stores the whole row of values (corresponding to that tag)
		std::vector<std::vector<double>> values_row;
		std::istringstream data_row(line);

		// Read the key (attribute name)
		data_row >> tag;
		// And all the values
		while ( data_row >> pair) {
			// Stores x,y pair of values (one column)
			std::vector<double> temp(2,0.0);
			// Parse the pair based on the comma
			// ind is now the index of the comma
			ind = pair.find(",");
			if (ind != std::string::npos) {
				// Independent variable before ind
				temp.at(0) = std::stod(pair.substr(0,ind));
				// Dependent variable after ind
				temp.at(1) = std::stod(pair.substr(ind+1));
			} else {
				throw std::invalid_argument("Table input format is not correct"); 
			}
			values_row.push_back(temp);
		}
	
		// Store the row under correct attribute
		whole_table[tag] = values_row;	
	}
	return whole_table;
}
