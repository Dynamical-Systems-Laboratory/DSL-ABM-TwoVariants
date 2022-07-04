#include "../../include/places/place.h"

/***************************************************** 
 * class: Place
 * 
 * Base class that defines a place  
 * 
 *****************************************************/

//
// Initialization and update
//

// Add an agent
void Place::register_agent(const int agent_ID)
{
	// Store ID
	agent_IDs.push_back(agent_ID);
	// Update total
	++num_tot;
}

//
// Infection related computations
//

// Calculates and stores probability contribution 
// from exposed and symptoamtic agents if any 
void Place::compute_infected_contribution()
{
	num_tot = agent_IDs.size();
	
	if (num_tot == 0){
		std::fill(lambda_tot.begin(), lambda_tot.end(), 0.0);
	}else{
		std::transform(lambda_sum.begin(), lambda_sum.end(), lambda_tot.begin(), 
			[this](const double& lsum){ return lsum/(static_cast<double>(this->num_tot)); });
	}	
}

//
// I/O
//

// Save information about a Place object
void Place::print_basic(std::ostream& where) const
{
	where << ID << " " << x << " " << y << " "
		  << num_tot << " " << ck;	
}

//
// Initialization and update
//

// Remove an agent from this place
void Place::remove_agent(const int index)
{
	std::vector<int> new_agent_IDs = {};
	std::remove_copy(agent_IDs.begin(), agent_IDs.end(),
					std::back_insert_iterator<std::vector<int>>(new_agent_IDs), index);
	std::swap(new_agent_IDs, agent_IDs);
}

//
// Supporting functions
//

// Overloaded ostream operator for I/O
std::ostream& operator<< (std::ostream& out, const Place& place) 
{
	place.print_basic(out);
	return out;
}


