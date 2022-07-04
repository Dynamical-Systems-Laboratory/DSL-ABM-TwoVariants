#include "../../include/places/hospital.h"

/***************************************************** 
 * class: Hospital
 * 
 * Defines and stores attributes of a single hospital 
 * 
 *****************************************************/

//
// Infection related computations
//

// Calculates and stores probability contribution 
// from exposedi and symptoamtic agents if any 
void Hospital::compute_infected_contribution()
{
	num_tot = agent_IDs.size() + n_tested;
	if (num_tot == 0) {
		std::fill(lambda_tot.begin(), lambda_tot.end(), 0.0);
	} else {
		std::transform(lambda_sum.begin(), lambda_sum.end(), lambda_tot.begin(), 
			[this](const double& lsum){ return lsum/(static_cast<double>(this->num_tot)); });
	}
}

//
// I/O
//

// Save information about a Hospital object
void Hospital::print_basic(std::ostream& where) const
{
	where << ID << " " << x << " " << y << " "
		  << num_tot << " " << ck;	
}


