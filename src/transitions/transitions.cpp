#include "../../include/transitions/transitions.h"

/***************************************************** 
 * class: Transitions 
 *
 * Interface for computing of transitioning 
 * between different agents states 
 * 
 ******************************************************/

// Transitions related to quarantining an agent as part of contact tracing
void Transitions::new_quarantined(Agent& agent, const double time,  
				const double dt, Infection& infection,
				std::vector<Household>& households, std::vector<School>& schools,
				std::vector<Workplace>& workplaces, std::vector<Hospital>& hospitals,
				std::vector<RetirementHome>& retirement_homes,
				std::vector<Transit>& carpools, std::vector<Transit>& public_transit,
				const std::map<std::string, double>& infection_parameters)
{
	// Set the main flag
	agent.set_contact_traced(true);
	// Set time to end memory of being quarantined
	agent.set_memory_duration(time + infection_parameters.at("quarantine duration")
						+ infection_parameters.at("quarantine memory"));	
	// Set vaccination lag
	agent.set_suspected_can_vaccinate(false);
	agent.set_former_suspected(true);
	agent.set_time_recovered_can_vaccinate(time + infection_parameters.at("quarantine duration")					   
  						+ infection_parameters.at("quarantine memory")
						+ infection_parameters.at("Post-infection vaccination lag"));
	// Extempt from quarantining (recovered from any strain)
	if (!agent.removed() || !agent.is_up_to_date()) {
		return;
	} 
	// Removal from all the public places (except leisure - in the next step anyway)
	agent.set_quarantine_duration(time + infection_parameters.at("quarantine duration"));
	int agent_ID = agent.get_ID();
	if (agent.student()) {
		schools.at(agent.get_school_ID()-1).remove_agent(agent_ID);
	}
	if (agent.works()){
		if (agent.works_from_home()) {
			return;
		}
		if (agent.retirement_home_employee()){
			retirement_homes.at(agent.get_work_ID()-1).remove_agent(agent_ID);
		} else if (agent.school_employee()){
			schools.at(agent.get_work_ID()-1).remove_agent(agent_ID);
		} else {
			workplaces.at(agent.get_work_ID()-1).remove_agent(agent_ID);
		}
		if (agent.get_work_travel_mode() == "carpool") {
			carpools.at(agent.get_carpool_ID()-1).remove_agent(agent_ID);
		}
		if (agent.get_work_travel_mode() == "public") {
			public_transit.at(agent.get_public_transit_ID()-1).remove_agent(agent_ID);
		}
	}
	if (agent.hospital_employee()) {
		hospitals.at(agent.get_hospital_ID()-1).remove_agent(agent_ID);
		if (agent.get_work_travel_mode() == "carpool") {
			carpools.at(agent.get_carpool_ID()-1).remove_agent(agent_ID);
		}
		if (agent.get_work_travel_mode() == "public") {
			public_transit.at(agent.get_public_transit_ID()-1).remove_agent(agent_ID);
		}
	}
	if (agent.symptomatic() && !agent.hospital_employee() && !agent.hospital_non_covid_patient()
			&& !agent.being_treated() && !agent.tested()) {
		// Cancel all testing and assign treatment
		regular_tr.set_all_testing_states(agent, false);
		regular_tr.select_initial_treatment(agent, time, dt, infection, households, 
						schools, workplaces, hospitals, retirement_homes, 
						carpools, public_transit, infection_parameters);
		// Recompute the probability of dying
		regular_tr.recovery_status(agent, infection, time, infection_parameters);
	} else {
		// Set quarantine 
		agent.set_being_treated(true);
		agent.set_home_isolated(true);
	}	
}

// Implement transitions that hold for all the agents
bool Transitions::common_transitions(Agent& agent, const double time, 
										std::vector<School>& schools,
										std::vector<Workplace>& workplaces, std::vector<Hospital>& hospitals,
										std::vector<RetirementHome>& retirement_homes,
										std::vector<Transit>& carpools, 
										std::vector<Transit>& public_transit, 
										Contact_tracing& contact_tracing,
				const std::map<std::string, double>& infection_parameters)
{
	bool re_vaccinating = false;
	int n_strains = static_cast<int>(infection_parameters.at("number of strains"));
	for (int ist = 1; ist<=n_strains; ++ist) {
		// Recovered becoming susceptible again
		if (agent.removed_recovered(ist) && agent.get_time_recovered_to_susceptible() <= time) {
			agent.set_removed_recovered(false, ist);
		}
		// Can vaccinate after recovery 
		if (agent.removed_recovered(ist) && !agent.removed_can_vaccinate() && 
				agent.get_time_recovered_can_vaccinate() <= time) {
			agent.set_removed_can_vaccinate(true);
		}
	}
	// Elligible for re-vaccination
	if (agent.vaccinated() && !agent.needs_next_vaccination() && 
				agent.get_time_vaccine_effects_reduction() <= time) {
		agent.set_needs_next_vaccination(true);
		re_vaccinating = true;
	}
	// Can vaccinate after suspected COVID-19 (through flu, contact tracing)
	if (agent.former_suspected() && !agent.suspected_can_vaccinate() && 
			agent.get_time_recovered_can_vaccinate() <= time) {
		agent.set_former_suspected(false);
		agent.set_suspected_can_vaccinate(true);
	}
	// Time mobility increases
/*	if (agent.vaccinated() && !agent.more_active() && 
				agent.get_time_mobility_increase() <= time) {
		agent.set_more_active(true);
	}*/
	// Returns from quarantine (only if not confirmed - including false)
	if (agent.contact_traced() && agent.get_quarantine_duration() <= time && agent.home_isolated()) {
		return_from_quarantine(agent, schools, workplaces, hospitals, 
								retirement_homes, carpools, public_transit, contact_tracing);
	}
	// Reset memory		
	if (agent.contact_traced() && agent.get_memory_duration() <= time) {
		agent.set_contact_traced(false);
	}
	return re_vaccinating;
}

// Lift this agents quarantine, add back to public places
void Transitions::return_from_quarantine(Agent& agent, std::vector<School>& schools,
				std::vector<Workplace>& workplaces, std::vector<Hospital>& hospitals,
				std::vector<RetirementHome>& retirement_homes,
				std::vector<Transit>& carpools, std::vector<Transit>& public_transit,
				Contact_tracing& contact_tracing)
{
	// Different treatment for symptomatic - assuming they will stay quarantined
	// Same goes for any agent that is currently undergoing testing or
	// for an agent that has flu (COVID-19-like symptoms)
	if (agent.symptomatic() || agent.tested() || agent.symptomatic_non_covid()) {
		return;
	}
	agent.set_being_treated(false);
	agent.set_home_isolated(false);

	// Reset the household flag
	contact_tracing.reset_house_isolation(agent.get_household_ID());

	int agent_ID = agent.get_ID();	
	// Add back to all places
	if (agent.student()) {
		schools.at(agent.get_school_ID()-1).add_agent(agent_ID);
	}
	if (agent.works()){
		if (agent.works_from_home()) {
			return;
		}
		if (agent.retirement_home_employee()){
			retirement_homes.at(agent.get_work_ID()-1).add_agent(agent_ID);
		} else if (agent.school_employee()){
			schools.at(agent.get_work_ID()-1).add_agent(agent_ID);
		} else {
			workplaces.at(agent.get_work_ID()-1).add_agent(agent_ID);
		}
		if (agent.get_work_travel_mode() == "carpool") {
			carpools.at(agent.get_carpool_ID()-1).add_agent(agent_ID);
		}
		if (agent.get_work_travel_mode() == "public") {
			public_transit.at(agent.get_public_transit_ID()-1).add_agent(agent_ID);
		}
	}
	if (agent.hospital_employee()) {
		hospitals.at(agent.get_hospital_ID()-1).add_agent(agent_ID);
		if (agent.get_work_travel_mode() == "carpool") {
			carpools.at(agent.get_carpool_ID()-1).add_agent(agent_ID);
		}
		if (agent.get_work_travel_mode() == "public") {
			public_transit.at(agent.get_public_transit_ID()-1).add_agent(agent_ID);
		}
	}
}

// Implement transitions relevant to susceptible
std::vector<int> Transitions::susceptible_transitions(Agent& agent, const double time, 
				const double dt, Infection& infection,	
				std::vector<Household>& households, std::vector<School>& schools,
				std::vector<Workplace>& workplaces, std::vector<Hospital>& hospitals,
				std::vector<RetirementHome>& retirement_homes,
				std::vector<Transit>& carpools, std::vector<Transit>& public_transit,
				std::vector<Leisure>& leisure_locations, 
				const std::map<std::string, double>& infection_parameters, 
				std::vector<Agent>& agents, Flu& flu, const Testing& testing, const int n_strains)
{
	// Ingected, tested, negative, false positive 
	std::vector<int> state_changes(4,0);
	int got_infected = 0;
	if (agent.symptomatic_non_covid()){
		// Currently only symptomatic non-COVID can reach all 4 states
		state_changes = flu_tr.susceptible_transitions(agent, time, infection,
				households, schools, workplaces, hospitals, retirement_homes,
			    carpools, public_transit, leisure_locations, 
				infection_parameters, agents, flu, testing, dt, n_strains);
	} else if (agent.hospital_employee()){
		got_infected = hsp_emp_tr.susceptible_transitions(agent, time, infection,
				households, schools, hospitals,
			    carpools, public_transit, leisure_locations,
				infection_parameters, agents, testing, n_strains);
		state_changes.at(0) = got_infected;
	} else if (agent.hospital_non_covid_patient()){
		got_infected = hsp_pt_tr.susceptible_transitions(agent, time, infection,
				hospitals, infection_parameters, agents, testing, n_strains);
		state_changes.at(0) = got_infected;
	} else {
		got_infected = regular_tr.susceptible_transitions(agent, time, infection,
				households, schools, workplaces, hospitals, retirement_homes,
			    carpools, public_transit, leisure_locations,
				infection_parameters, agents, flu, testing, n_strains);
		state_changes.at(0) = got_infected;
	}
	return state_changes;	
}

// Implement transitions relevant to exposed 
std::vector<int> Transitions::exposed_transitions(Agent& agent, Infection& infection, const double time, const double dt, 
										std::vector<Household>& households, std::vector<School>& schools,
										std::vector<Workplace>& workplaces, std::vector<Hospital>& hospitals,
										std::vector<RetirementHome>& retirement_homes,
										std::vector<Transit>& carpools, std::vector<Transit>& public_transit,
										const std::map<std::string, double>& infection_parameters, const Testing& testing)
{
	// Recovered, dead, tested, tested positive, tested false negative
	std::vector<int> state_changes(5,0);
	if (agent.hospital_employee()){
		state_changes = hsp_emp_tr.exposed_transitions(agent, infection, time, dt, 
					households, schools, hospitals,
					carpools, public_transit, infection_parameters, testing);
	} else if (agent.hospital_non_covid_patient()){
		state_changes = hsp_pt_tr.exposed_transitions(agent, infection, time, dt, 
					households, hospitals, infection_parameters, testing);
	} else {
		state_changes = regular_tr.exposed_transitions(agent, infection, time, dt, 
					households, schools, workplaces, hospitals, retirement_homes,
			    	carpools, public_transit, infection_parameters, testing);
	}
	return state_changes;
}

// Transitions of a symptomatic agent 
std::vector<int> Transitions::symptomatic_transitions(Agent& agent, const double time, 
				   	const double dt, Infection& infection,
					std::vector<Household>& households, std::vector<School>& schools,
					std::vector<Workplace>& workplaces, std::vector<Hospital>& hospitals,
					std::vector<RetirementHome>& retirement_homes,
					std::vector<Transit>& carpools, std::vector<Transit>& public_transit,
					const std::map<std::string, double>& infection_parameters)
{
	// Recovered, dead, tested, tested positive, false negative
	std::vector<int> state_changes(5,0);
	if (agent.hospital_employee()){
		// Home isolated upon developing symptoms - no travel to work or leisure
		// This holds for symptoms of anything flu-like, people will not go to
		// work in hospital when sick - likely
		state_changes = hsp_emp_tr.symptomatic_transitions(agent, time, dt, infection,  
					households, schools, hospitals, carpools, public_transit, infection_parameters);
	} else if (agent.hospital_non_covid_patient()){
		state_changes = hsp_pt_tr.symptomatic_transitions(agent, time, dt, 
					infection, households, hospitals, infection_parameters);
	} else {
		
		state_changes = regular_tr.symptomatic_transitions(agent, time, dt, infection, 
					households, schools, workplaces, hospitals, retirement_homes,
				    carpools, public_transit, infection_parameters);
	}
	return state_changes; 
}


