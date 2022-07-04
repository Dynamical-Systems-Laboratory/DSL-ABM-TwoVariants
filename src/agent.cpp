#include "../include/agent.h"

/***************************************************** 
 * class: Agent
 * 
 * Defines and stores attributes of a single agent
 * 
 *****************************************************/

//
// Transmission
//

// Setup vectors with vaccination effects based on the target number of strains
void Agent::initialize_benefits()
{
	vaccinated_for_strains.resize(n_strains, 0);
	tpf_effectiveness.resize(n_strains, ThreePartFunction(0.0));
	tpf_asymptomatic.resize(n_strains, ThreePartFunction(1.0));
   	tpf_transmission.resize(n_strains, ThreePartFunction(1.0));
	tpf_severe.resize(n_strains, ThreePartFunction(1.0));
	tpf_death.resize(n_strains, ThreePartFunction(1.0));
	fpf_effectiveness.resize(n_strains, FourPartFunction(0.0));
	fpf_asymptomatic.resize(n_strains, FourPartFunction(1.0));
	fpf_transmission.resize(n_strains, FourPartFunction(1.0));
	fpf_severe.resize(n_strains, FourPartFunction(1.0));
	fpf_death.resize(n_strains, FourPartFunction(1.0));
	vaccine_type.resize(n_strains, "one_dose");
	vaccine_subtype.resize(n_strains, "one_dose");
	is_removed_recovered.resize(n_strains, 0);
}

//
// I/O
//

// Print Agent information 
void Agent::print_basic(std::ostream& where) const
{
	where << ID << " " << is_student << " " << is_working  
		  << " " << age << " " << x << " " << y << " "
		  << house_ID << " " << is_non_covid_patient << " " << school_ID 
		  << " " << work_ID << " " << works_at_hospital 
		  << " " << hospital_ID << " " << worksRH 
		  << " " << worksSch << " " << livesRH << " "<< is_infected;	
}

// Workplace transmissions for out-of-town
void Agent::set_occupation_transmission()
{
	for (const auto& strain_map : transmission_rates) {
		occupation_transmission_rates.push_back(strain_map.at("workplace transmission rate"));
	}
}

//
// Supporting functions
//

// Overloaded ostream operator for I/O
std::ostream& operator<< (std::ostream& out, const Agent& agent) 
{
	agent.print_basic(out);
	return out;
}


