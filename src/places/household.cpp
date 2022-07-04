#include "../../include/places/household.h"

/***************************************************** 
 * class: Household
 * 
 * Defines and stores attributes of a single household
 * 
 *****************************************************/

//
// I/O
//

// Save information about a Household object
void Household::print_basic(std::ostream& where) const
{
	Place::print_basic(where);
	where << " " << alpha;	
}

// Calculates and stores fraction of infected agents if any 
void Household::compute_infected_contribution()
{
	num_tot = agent_IDs.size();
	
	if (num_tot == 0) {
		std::fill(lambda_tot.begin(), lambda_tot.end(), 0.0);
	} else {
		std::transform(lambda_sum.begin(), lambda_sum.end(), lambda_tot.begin(), 
			[this](const double& lsum){ return lsum/(std::pow(static_cast<double>(this->num_tot), this->alpha)); });
	}
}


