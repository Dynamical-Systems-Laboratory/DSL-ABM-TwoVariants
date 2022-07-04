#include "../../include/places/leisure.h"

/***************************************************** 
 * class: Leisure 
 * 
 * Defines and stores attributes of a single leisure 
 * location 
 * 
 *****************************************************/

//
// Infection related computations
//


// Calculates and stores probability contribution of infected agents if any 
void Leisure::compute_infected_contribution()
{
	// Different treatment for locations outside the modeled town 
	if (type == "outside") {
		std::copy(frac_inf_out.begin(), frac_inf_out.end(), lambda_tot.begin());
	} else {
		Place::compute_infected_contribution();
	}
}

//
// I/O
//

// Save information about a Workplace object
void Leisure::print_basic(std::ostream& where) const
{
	Place::print_basic(where);
	where << " " << type;	
}


