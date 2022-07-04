#include "../include/contact_tracing.h"

/***************************************************** 
 * class: Contact_tracing
 * 
 * Manages the contact tracing functionality  
 * 
 *****************************************************/

// Assumptions:
// - Agents that are already isolated are removed from public places
// - Household members always isolate
// - Setting properties and removal of isolated agents is done through ABM

// Add guest household ID to agent aID
void Contact_tracing::add_household(const int aID, const int hID, const int time)
{
	std::deque<std::vector<int>>& visits = private_leisure.at(aID-1);
	if (visits.size() < max_num_hID) {
		visits.push_back(std::vector<int>{hID, time});
	} else {
		visits.pop_front();
		visits.push_back(std::vector<int>{hID, time});
	}
	assert(visits.size() <= max_num_hID); 
}

// Apply isolation to the household of agent aID
std::vector<int> Contact_tracing::isolate_household(const int aID, const Household& household)
{
	std::vector<int> traced;
	for (const auto& ag : household.get_agent_IDs()) {
		if (ag != aID) {
			traced.push_back(ag);
		}
	}
	// Abandoning for now
	//is_isolated.at(household.get_ID()-1) = true;
	return traced;
}

// Apply selective isolation to all guest households of a quarantined agent
std::vector<int> Contact_tracing::isolate_visited_households(const int aID,
									const std::vector<Household>& households, 
									const double compliance, Infection& infection,
									const int time, const double dt)
{
	std::vector<int> traced;
	std::deque<std::vector<int>>& visits = private_leisure.at(aID-1);
	while (!visits.empty()) {
		std::vector<int> hdata = visits.front();
		int hsID = hdata.at(0);
		int tvis = hdata.at(1);
		int del_tvis = time - tvis;
		// This is kind of hideous but will do for now
		// Will apply CT only to households visited within an input #days		
		if (del_tvis > static_cast<int>(max_num_hID*dt)) {
			visits.pop_front();
			continue;
		}
 		// Check if guest household will isolate (if not already isolated)
		if (!is_isolated.at(hsID-1) && infection.get_uniform() <= compliance) {
			for (const auto& ag : households.at(hsID-1).get_agent_IDs()) {
				if (ag != aID) {
					// In case a guest at this step
					traced.push_back(ag);
				}
			}
			is_isolated.at(hsID-1) = true;		
		} 
		visits.pop_front();
	}
	assert(visits.empty());
	return traced;
}

// Apply isolation to random num_contacts at a general workplace of agent aID
std::vector<int> Contact_tracing::isolate_workplace(const int aID, const std::vector<Agent>& agents, 
						const Workplace& workplace, const double num_contacts,
						Infection& infection)
{
	std::vector<int> traced;
	std::vector<int> coworkers = workplace.get_agent_IDs();
	if (!coworkers.empty()) {
		infection.vector_shuffle(coworkers);
		int num_coworkers = coworkers.size() >= num_contacts ? num_contacts : coworkers.size();
		for (int i=0; i<num_coworkers; ++i) {
			if (coworkers.at(i) == aID) {
				continue;
			} else {
				traced.push_back(coworkers.at(i));		
			}
		}
	}
	return traced; 	
}

// Apply isolation to random num_contacts at a hospital where agent aID works
std::vector<int> Contact_tracing::isolate_hospital(const int aID, const std::vector<Agent>& agents, 
							const Hospital& hospital, const double num_contacts,
							Infection& infection)
{
	std::vector<int> traced;
	std::vector<int> everyone = hospital.get_agent_IDs();
	if (!everyone.empty()) {
		infection.vector_shuffle(everyone);
		int num_coworkers = 0;
		for (const auto& ag : everyone) {
			const Agent& agent = agents.at(ag-1);
			if (ag == aID || !agent.hospital_employee()) {
				continue;
			} else {
				traced.push_back(ag);
				++num_coworkers;
			}
			if (num_coworkers >= num_contacts) {
				break;
			}
		}
	}
	return traced; 	
}

// Apply isolation to random num_contacts at a retirement homw where agent aID works
std::vector<int> Contact_tracing::isolate_retirement_home(const int aID, const std::vector<Agent>& agents, 
							const RetirementHome& retirement_home, const double num_emp, 
							const double num_res, Infection& infection)
{
	std::vector<int> traced;
	std::vector<int> everyone = retirement_home.get_agent_IDs();
	if (!everyone.empty()) {
		infection.vector_shuffle(everyone);
		int num_coworkers = 0, num_residents = 0;
		for (const auto& ag : everyone) {
			const Agent& agent = agents.at(ag-1);
			if (ag == aID) {
				continue;
			} else {
				if (agent.retirement_home_employee() && num_coworkers < num_emp) {
					++num_coworkers;
					traced.push_back(ag);
				} else if (agent.retirement_home_resident() && num_residents < num_res) {
					++num_residents;
					traced.push_back(ag);
				} 		
			}
			if (num_coworkers >= num_emp && num_residents >= num_res) {
				break;
			}
		}
	}
 	return traced;	
}

// Schools - pick a random age if not a student and select from that group
std::vector<int> Contact_tracing::isolate_school(const int aID, std::vector<Agent>& agents,
 							const School& school, const double n_students,
							Infection& infection)
{
	std::vector<int> traced;
	// Infected agent
	const Agent& agent = agents.at(aID-1);
	bool is_student = (agent.student() && (agent.get_school_ID() == school.get_ID()));
	// Students and staff
 	std::vector<int> everyone = school.get_agent_IDs();
	if (everyone.size() <= 1) {
		return traced;
	}
	// Select the age of the class
	int age = 0;
	if (is_student) {
		age = agent.get_age();
	} else {
		// Select all students
		std::vector<int> all_students;
		for (const auto& ag : everyone) {
			if (ag == aID) {
				// Skip self
				continue;
			}
			// Add potential student
			if (agents.at(ag-1).student() && (agents.at(ag-1).get_school_ID() == school.get_ID())) { 
				all_students.push_back(ag);				
			}
		}
		// Randomly select a student and their age
		if (!all_students.empty()) {
			int i = infection.get_int(0, all_students.size()-1);
			age = agents.at(all_students.at(i)-1).get_age();
		} else {
			return traced;
		}
	} 	
	// Class size counter
	int n_class = 0;
	// Isolate one teacher and n_student sized class
	// Fist a teacher if infected agent is a student
	std::vector<int> teachers;
	if (is_student) {
		for (const auto& ag : everyone) {
			if (ag == aID) {
				// Skip self
				continue;
			}
			// Add potential teacher
			if (agents.at(ag-1).school_employee() &&
					(agents.at(ag-1).get_work_ID() == school.get_ID())) { 
				teachers.push_back(ag);				
			}
		}
	}
	// Isolate an n_student sized class
	for (const auto& ag : everyone) {
		if (ag == aID) {
			// Skip self
			continue;
		}
		// Collect the class
		if ((agents.at(ag-1).get_age() == age) &&
				(agents.at(ag-1).get_school_ID() == school.get_ID())) {
			traced.push_back(ag);
			++n_class;	
		}
		if (n_class >= n_students) {
			// Done collecting a class, add a teacher if agent is a student
			if (is_student && !teachers.empty()) {
				infection.vector_shuffle(teachers);
				traced.push_back(teachers.at(0));
			}
			break;
		}
	}
	if (n_class < n_students && is_student && !teachers.empty()) {
		// Done collecting a class, add a teacher if agent is a student
		infection.vector_shuffle(teachers);
		traced.push_back(teachers.at(0));
	}
	return traced;
}

// Apply isolation to all agents that share a carpool 
std::vector<int> Contact_tracing::isolate_carpools(const int aID, const std::vector<Agent>& agents, 
							const Transit& carpool)
{
	std::vector<int> traced;
	for (const auto& ag : carpool.get_agent_IDs()) {
		if (ag != aID) {
			traced.push_back(ag);
		}
	}
	return traced;
}


