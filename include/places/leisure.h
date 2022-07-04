#ifndef LEISURE_H
#define LEISURE_H

#include "place.h"

class Place;

/***************************************************** 
 * class: Leisure 
 * 
 * Defines and stores attributes of a single leisure 
 * location 
 * 
 *****************************************************/

class Leisure : public Place {
public:

	//
	// Constructors
	//

	/**
	 * \brief Creates a Leisure object with default attributes
	 */
	Leisure() = default;

	/**
	 * \brief Creates a Leisure object 
	 * \details Leisure with custom ID, location, and infection parameters
	 *
	 * @param leisure_ID - ID of the leisure location 
	 * @param xi - x coordinate of the leisure location 
	 * @param yi - y coordinate of the leisure location
	 * @param severity_cor - severity correction for symptomatic
	 * @param ltype - leisure location type
	 * @param strain_no - number of strains
	 */
	Leisure(const int leisure_ID, const double xi, const double yi,
			 const double severity_cor, const std::string ltype, const int strain_no) : 
			type(ltype), Place(leisure_ID, xi, yi, severity_cor, strain_no)
				{ frac_inf_out.resize(strain_no); }

	//
	// Infection related computations
	//

	/// Calculates and stores probability contribution of infected agents if any 
	void compute_infected_contribution() override;
	
	//
	// Setters
	//

	/// Assign total contribution at a leisure location outside the modeled town
	void set_outside_infected(const double val, const int strain_id) 
		{ frac_inf_out.at(strain_id-1) = val; }

	/// Separate modification due to closures
	//void adjust_outside_infected(const double mod) { frac_inf_out *= mod; }
	
	//
	// Getters
	//
	
	/// True if the leisure location is outside current town
	bool outside_town() const override 
		{ return (type == "outside")?(true) : (false); }

	/// Lambda for a leisure location outside the modeled town
	//double get_outside_infected() const { return frac_inf_out; }
	
	//
 	// I/O
	//

	/**
	 * \brief Save information about a Leisure object
	 * \details Saves to a file, everything but detailed agent 
	 * 		information; order is ID | x | y | number of agents | 
	 * 		 ck | type 
	 * 		Delimiter is a space.
	 * 	@param where - output stream
	 */
	void print_basic(std::ostream& where) const override;

private:
	// Leisure location type 
	std::string type = "none";
	// Lambda of an outside leisure location
	std::vector<double> frac_inf_out;
};
#endif
