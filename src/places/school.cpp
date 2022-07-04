#include "../../include/places/school.h"

/***************************************************** 
 * class: School
 * 
 * Defines and stores attributes of a single school 
 * 
 *****************************************************/

//
// I/O
//

// Save information about a School object
void School::print_basic(std::ostream& where) const
{
	Place::print_basic(where);
	where << " " << psi_emp << " " << psi_j;	
}


