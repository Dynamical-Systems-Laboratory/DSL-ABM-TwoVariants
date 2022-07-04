#include "../../include/places/transit.h"

/***************************************************** 
 * class: Transit 
 * 
 * Defines and stores attributes of a single transit 
 * mode object 
 * 
 *****************************************************/

//
// I/O
//

// Save information about a Transit object
void Transit::print_basic(std::ostream& where) const
{
	Place::print_basic(where);
	where << " " << psi_j << " " << type;	
}


