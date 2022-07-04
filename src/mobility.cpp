#include "../include/mobility.h"

/***************************************************** 
 * class: Mobility 
 * 
 * Attributes and functionality for agent mobility 
 * 
 *****************************************************/

// Compute and store visiting probabilities for public leisure locations
void Mobility::construct_public_probabilities(const std::vector<Household>& households,
				   						const std::vector<Leisure>& leisure_locations)
{
	// First check for parameters
	// Use default if not set and warn the user
	double tol = 1e-8;
	if (equal_floats(dr0, 0.0, tol) && equal_floats(beta, 0.0, tol) 
					&& equal_floats(kappa, 0.0, tol)) {
		std::cout << "Mobility: Probablity expression constants are not set and 0.0. Using defaults." << std::endl;
		dr0 = 1.5;
		kappa = 400.0;
		beta = 1.75;
	}
	// Compute the ditances and probabilities for all locations
	double dij = 0.0, pij = 0.0;
	for (const auto& house : households) {
		std::vector<double> probs = {};
		for (const auto& leisure : leisure_locations) {
			dij = compute_distance(house, leisure);
			pij = compute_probability(dij);
			probs.push_back(pij);	
		}
		// Cumulative sum for the CDF
		std::partial_sum(probs.begin(), probs.end(), probs.begin());
		// Normalize
		double max_p = probs.back();
		if (max_p > 0.0){
			std::for_each(probs.begin(), probs.end(), [&max_p](double &x) { x /= max_p; });
		}
		public_probabilities.push_back(probs);
	}	
}

// Computes distances between two locations based
// on their GIS coordinates
double Mobility::compute_distance(const Place& loc1, const Place& loc2)
{
	// Returns distance in km according to Haversine formula,
	// http://en.wikipedia.org/wiki/Haversine_formula

	const double radius = 6371.0; 
	const double lat1 = loc1.get_x()*pi/180.0;
	const double lon1 = loc1.get_y()*pi/180.0;
	const double lat2 = loc2.get_x()*pi/180.0;
	const double lon2 = loc2.get_y()*pi/180.0;
	const double deltaLat = lat2 - lat1;
	const double deltaLon = lon2 - lon1;

	const double a = std::sin(deltaLat/2.0)*std::sin(deltaLat/2.0) 
			+ std::cos(lat1)*std::cos(lat2)
			*std::sin(deltaLon/2.0)*std::sin(deltaLon/2.0);
	
	const double c = 2.0*std::atan2(std::sqrt(a), std::sqrt(1-a));
	const double d1km = radius*c;

	return d1km;
}

// Computes and returns probabilities based on distance
double Mobility::compute_probability(const double dist)
{
	const double pdij = std::pow((dist + dr0), -beta)*std::exp(-dist/kappa); 
	return pdij;
}

// Assign a leisure location - public or residential 
int Mobility::assign_leisure_location(Infection& infection, const int& house_ID, 
				bool& in_household, bool& in_public, const double& household_prob)
{
	in_household = false;
	in_public = false;
	int pub_ID = 0, guest_ID = 0;

	// Determine if the location will be private of public
	if (household_prob <= infection.get_uniform()) {	
		// If a household, randomly select the ID that is not one of current agents
		guest_ID = house_ID;
		while (guest_ID == house_ID) {
			guest_ID = infection.get_random_household_ID(public_probabilities.size());
		}	
		in_household = true;
		return guest_ID;
	} else {
		// If a public location - assign based on the probabilities
		int pub_ID = 0;
		in_public = true;
		const double prob = infection.get_uniform();
		const std::vector<double>& a_house = public_probabilities.at(house_ID-1);
			
		// Iterator to the first element with probability >= to prob, 
		// or one past last if no such element
		const auto& iter = std::find_if(a_house.cbegin(), a_house.cend(), 
						[&prob](const double x) { return x >= prob; });
		// Find and return the ID
		pub_ID = std::distance(a_house.cbegin(), iter) + 1;
		in_public = true;
		return pub_ID;
	}
	return 0;
}

// Save the matrix of probabilities to file	
void Mobility::print_probabilities(const std::string fname)
{
	// AbmIO settings
	std::string delim(" ");
	bool sflag = true;
	std::vector<size_t> dims = {0,0,0};	

	// Write data to file
	AbmIO abm_io(fname, delim, sflag, dims);
	abm_io.write_vector<double>(public_probabilities);
}
