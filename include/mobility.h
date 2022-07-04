#ifndef MOBILITY_H
#define MOBILITY_H

#include <cmath>
#include "io_operations/abm_io.h"
#include "io_operations/load_parameters.h"
#include "places/place.h"
#include "places/household.h"
#include "places/leisure.h"
#include "infection.h"
#include "common.h"
#include "utils.h"

/***************************************************** 
 * class: Mobility 
 * 
 * Attributes and functionality for agent mobility 
 * 
 *****************************************************/

class Mobility {
public:
	
	//
	// Constructors
	//

	/**
	 * \brief Creates a Mobility object with default attributes
	 */
	Mobility() = default;

	//
	// Initialization
	//

	/**
	 * \brief Compute and store visiting probabilities for public leisure locations
	 *
	 * @param households - vector of household objects
	 * @param leisure_locations - vector of public leisure locations
	 */
	void construct_public_probabilities(const std::vector<Household>& households,
				   						const std::vector<Leisure>& leisure_locations);
	
	//
	// Main functionality 
	//

	/**
	 * \brief Assign a leisure location - public or residential 
	 *
	 * @param infection - instance of the Infection class
	 * @param house_ID - ID of the household to be assigned to
	 * @param in_household - in/out - assigned leisure location is a household 
	 * @param in_public - in/out - assigned leisure location is public 
	 * @param household_prob - optional parameter, probability agent will visit a private household 
	 */
	int assign_leisure_location(Infection& infection, const int& house_ID, bool& in_household, bool& in_public, const double& household_prob = 0.5);



	/**
	 * \brief Computes distances between two locations 
	 * \details Uses the the GIS coordinates and expects x in the locations
	 * 		is the latitude and y is the longitude
 	 *
	 * @param loc1 - object of type Place 
	 * @param loc2 - object of type Place 
	 * @return Distance between loc1 and loc2 in kilometers
	 */
	double compute_distance(const Place& loc1, const Place& loc2);

	//
	// Setters
	//

	/// Set the parameters of the distance-based probability	
	void set_probability_parameters(const double _dr0, const double _beta, const double _kappa)
		{ dr0 = _dr0; beta = _beta; kappa = _kappa; }

	//
	// Getters
	//
	
	std::vector<std::vector<double>> get_public_probabilities()
		{ return public_probabilities; }

	//
	// IO
	//

	/// Save the matrix of probabilities to file	
	void print_probabilities(const std::string fname);

private:

	// Probabilities of each household viting 
	// a given public leisure location
	// Outer vector: households, inner: public leisure location
	std::vector<std::vector<double>> public_probabilities;

	// Parameters for the probability model
	double dr0 = 0.0, beta = 0.0, kappa = 0.0;

	// Computes and returns probabilities based on distance
	double compute_probability(double dist);

	// Just Pi
	double pi = 3.14159265358979323846;
};

#endif
