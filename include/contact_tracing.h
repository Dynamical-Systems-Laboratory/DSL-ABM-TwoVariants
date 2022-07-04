#ifndef CONTACT_TRACING_H
#define CONTACT_TRACING_H

#include <cassert>
#include <deque>
#include "common.h"
#include "places/place.h"
#include "places/household.h"
#include "places/retirement_home.h"
#include "places/school.h"
#include "places/workplace.h"
#include "places/hospital.h"
#include "places/transit.h"
#include "places/leisure.h"
#include "agent.h"
#include "infection.h"


/***************************************************** 
 * class: Contact_tracing
 * 
 * Manages the contact tracing functionality  
 * 
 *****************************************************/

class Contact_tracing {
public:

	// 
	// Constructors
	//

	Contact_tracing() = default;

	/** 
	 * \brief Create a Contact_tracing object
	 * @param na - number of agents
	 * @param nhs - number of households
	 * @param mn - maximum number of visited household IDs stored per agent
	 */
	Contact_tracing(const int na, const int nhs, const int mn) : 
		num_agents(na), num_hs(nhs), max_num_hID(mn) 
			{ private_leisure.resize(na); is_isolated.resize(num_hs, false); }
	
	//
	// Main functionality 
	//
		
	/// Add guest household ID to agent aID
	void add_household(const int aID, const int hID, const int);

	/// Return true if a household is fully quarantined
	bool house_is_isolated(const int hID) const { return is_isolated.at(hID-1); }

	/// Lift household quarantine
	void reset_house_isolation(const int hID) { is_isolated.at(hID-1) = false; }

	/// Apply isolation to the household of agent aID
	std::vector<int> isolate_household(const int aID, const Household& household);

	/// Apply selective isolation to all guest household of a quarantined agent
	std::vector<int> isolate_visited_households(const int aID, 
										const std::vector<Household>& households, 
										const double compliance, Infection& infection,
										const int, const double);

	/// Apply isolation to random num_contacts at a general workplace of agent aID
	std::vector<int> isolate_workplace(const int aID, const std::vector<Agent>& agents, 
							const Workplace& workplace, const double num_contacts,
							Infection& infection);
	/// Apply isolation to random num_contacts at a hospital where agent aID works
	std::vector<int> isolate_hospital(const int aID, const std::vector<Agent>& agents, 
							const Hospital& hospital, const double num_contacts,
							Infection& infection);
	/// Apply isolation to random num_contacts at a retirement homw where agent aID works
	std::vector<int> isolate_retirement_home(const int aID, const std::vector<Agent>& agents, 
							const RetirementHome& retirement_home, const double num_contacts,
							const double num_res, Infection& infection);
	/// Apply isolation to school where agent aID works or is a student
	std::vector<int> isolate_school(const int aID, std::vector<Agent>& agents, 
							const School& school, const double n_students,
							Infection& infection);
	/// Apply isolation to all agents that share a carpool 
	std::vector<int> isolate_carpools(const int aID, const std::vector<Agent>& agents, 
							const Transit& carpool);

	//
	// Getters
	//

	// Records of visited households
	std::vector<std::deque<std::vector<int>>> get_private_leisure() const { return private_leisure; }

private:
	// Number of agents
	int num_agents = 0;
	// Number of households
	int num_hs = 0;
	// Max number of private contacts to store
	int max_num_hID = 0;

	// Deque of vectors with the following elements:
	// 0) House IDs from private visits of each agent
	// 1) Times of visit floored to an integer day
	std::vector<std::deque<std::vector<int>>> private_leisure;
	// Households isolation flags
	std::vector<bool> is_isolated;
};

#endif
