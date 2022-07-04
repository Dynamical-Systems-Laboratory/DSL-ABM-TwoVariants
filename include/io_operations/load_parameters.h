#ifndef LOAD_PARAMETERS_H
#define LOAD_PARAMETERS_H

#include "FileHandler.h"
#include "../common.h"

/***************************************************** 
 * class: LoadParameters 
 * 
 * Class for reading and storing parameters from file 
 * 
 *****************************************************/

class LoadParameters
{
public:
		
	LoadParameters() = default; 

	/**
	 * \brief Read parameters/tokens from file, store the as a map
	 * \details In the file parameter name can be multiple words
	 * 		but it needs to be in a separate line with a // comment
	 * 		type as the first element of the line; it is followed
	 * 		by a value; currently available for doubles and strings   
	 *
	 * @param infile - input file with parameters
	 */
	template<typename T>
	std::map<std::string, T> load_parameter_map(const std::string& infile);

	/**
	 * \brief Reads parameters in a tabular form 
	 * \details Returns a map where each key is the first entry in the
	 *		table (1st column) followed by pairs of datapoints; 
	 *		The datapoints are in the form x1,y1 x2,y2 - no space 
	 *		in the pair, spaces between pairs (any whitespaces)   
	 *
	 * @param infile - input file with the table
	 */
	std::map<std::string, std::vector<std::vector<double>>> load_table(const std::string& infile);

	/** 
	 * \brief Read age-dependent distributions as map, store the as a map
	 * \details Keys are age ranges as strings, in a specific form like "40-43"
	 *
	 * @param infile - input file with parameters
	 */
	std::map<std::string, double> load_age_dependent(const std::string& infile);

};

#endif
