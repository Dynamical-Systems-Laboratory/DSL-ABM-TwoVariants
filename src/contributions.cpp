#include "../include/contributions.h"

/***************************************************** 
 * class: Contributions
 *
 * Functionality for computing of infection probability
 * contributions from agents to places and eventually 
 * mobility 
 * 
 ******************************************************/

// Count contributions of an exposed agent
void Contributions::compute_exposed_contributions(const Agent& agent, const double time,	
				std::vector<Household>& households, std::vector<School>& schools,
				std::vector<Workplace>& workplaces, std::vector<Hospital>& hospitals,
				std::vector<RetirementHome>& retirement_homes,
				std::vector<Transit>& carpools, std::vector<Transit>& public_transit,
				std::vector<Leisure>& leisure_locations)
{
	// Skip if not yet infectious
	if (time < agent.get_infectiousness_start_time()){
		return;
	}
	
	// Agent's infection variability
	double inf_var = 0.0;
	inf_var = agent.get_inf_variability_factor();
	const std::map<std::string, double>& trans_rates = agent.curr_strain_tr_rates();
	const int strain_id = agent.get_strain();

	// If main state "tested"
	if (agent.tested()){
		if (agent.get_time_of_test() <= time && agent.tested_awaiting_test() == true){
			// If being tested at this step 			
			if (agent.tested_in_hospital() == true){
				compute_hospital_tested_contributions(agent, inf_var, hospitals);
			}else if (agent.tested_in_car() == true){
				return;
			}else{
				throw std::runtime_error("Agents testing site not specified");
			}
		} else if (agent.tested_awaiting_results() || agent.tested_awaiting_test()){
			// Assuming home isolation except for hospital employees and non-covid
			if (agent.hospital_non_covid_patient() == false && 
					agent.hospital_employee() == false){
				compute_home_isolated_contributions(agent, inf_var, households, retirement_homes);
			} else if (agent.hospital_non_covid_patient()){
				Hospital& hospital = hospitals.at(agent.get_hospital_ID()-1);
				hospital.add_exposed_patient(inf_var, trans_rates.at("hospital patients transmission rate"), strain_id);
			} else if (agent.hospital_employee()){
				// Exposed confirmed COVID in home isolation or quarantined due to COVID exposure
				if (agent.contact_traced()){
					compute_home_isolated_contributions(agent, inf_var, households, retirement_homes);
					return;
				}
				Hospital& hospital = hospitals.at(agent.get_hospital_ID()-1);
				hospital.add_exposed(inf_var, trans_rates.at("healthcare employees transmission rate"), strain_id);
				// Household
				Household& household = households.at(agent.get_household_ID()-1);
				household.add_exposed(inf_var, trans_rates.at("household transmission rate"), strain_id);
				// Other places
				if (agent.student() == true){
					School& school = schools.at(agent.get_school_ID()-1);
					school.add_exposed(inf_var, trans_rates.at("school transmission rate"), strain_id);	
				}
				// Transit
				if (agent.get_work_travel_mode() == "carpool") {
					Transit& carpool = carpools.at(agent.get_carpool_ID()-1);
					carpool.add_exposed(inf_var, trans_rates.at("carpool transmission rate"), strain_id);
				}
				if (agent.get_work_travel_mode() == "public") {
					Transit& bus = public_transit.at(agent.get_public_transit_ID()-1);
					bus.add_exposed(inf_var, trans_rates.at("public transit transmission rate"), strain_id);
				}
				// Leisure
				if (agent.get_leisure_ID() > 0) {
					if (agent.get_leisure_type() == "public") {
						Leisure& les_loc = leisure_locations.at(agent.get_leisure_ID()-1); 
						if (!les_loc.outside_town()){
							les_loc.add_exposed(inf_var, trans_rates.at("leisure locations transmission rate"), strain_id);
						}
					} else if (agent.get_leisure_type() == "household") {
						Household& household = households.at(agent.get_leisure_ID()-1);
						household.add_exposed(inf_var, trans_rates.at("household transmission rate"), strain_id);
					} else {
						throw std::invalid_argument("Wrong leisure type: " + agent.get_leisure_type());
					}
				} 
			}
		}
	}else{
		// If hospitalized with a different condition, 
		// and exposed (infectious), only hospital contribution
		if (agent.hospital_non_covid_patient() == true &&
				agent.tested_covid_positive() == false){
			Hospital& hospital = hospitals.at(agent.get_hospital_ID()-1);
			hospital.add_exposed_patient(inf_var, trans_rates.at("hospital patients transmission rate"), strain_id);
			return;
		}
	
		// Exposed confirmed COVID in home isolation or quarantined due to COVID exposure
		if (agent.tested_covid_positive() || agent.contact_traced()){
			compute_home_isolated_contributions(agent, inf_var, households, retirement_homes);
			return;
		}

		// Household or retirement home
		if (agent.retirement_home_resident()){
			RetirementHome& rh = retirement_homes.at(agent.get_household_ID()-1);
			rh.add_exposed(inf_var, trans_rates.at("RH resident transmission rate"), strain_id);
		} else {
			Household& household = households.at(agent.get_household_ID()-1);
			household.add_exposed(inf_var, trans_rates.at("household transmission rate"), strain_id);
		}

		// Other places
		if (agent.student() == true){
			School& school = schools.at(agent.get_school_ID()-1);
			school.add_exposed(inf_var, trans_rates.at("school transmission rate"), strain_id);	
		}
		if (agent.works() && !agent.works_from_home()) {
			if (agent.retirement_home_employee()){
				RetirementHome& rh = retirement_homes.at(agent.get_work_ID()-1);
				rh.add_exposed_employee(inf_var, trans_rates.at("RH employee transmission rate"), strain_id);
			} else if (agent.school_employee()){
				School& sch = schools.at(agent.get_work_ID()-1);
				sch.add_exposed_employee(inf_var, trans_rates.at("school employee transmission rate"), strain_id);
			} else {
				Workplace& workplace = workplaces.at(agent.get_work_ID()-1);
				if (!workplace.outside_town()) {
					workplace.add_exposed(inf_var, trans_rates.at("workplace transmission rate"), strain_id);
				}
			}
		}
		if (agent.hospital_employee() == true){
			Hospital& hospital = hospitals.at(agent.get_hospital_ID()-1);
			hospital.add_exposed(inf_var, trans_rates.at("healthcare employees transmission rate"), strain_id);
		}
		// Transit
		if (agent.get_work_travel_mode() == "carpool") {
			Transit& carpool = carpools.at(agent.get_carpool_ID()-1);
			carpool.add_exposed(inf_var, trans_rates.at("carpool transmission rate"), strain_id);
		}
		if (agent.get_work_travel_mode() == "public") {
			Transit& bus = public_transit.at(agent.get_public_transit_ID()-1);
			bus.add_exposed(inf_var, trans_rates.at("public transit transmission rate"), strain_id);
		}
		// Leisure
		if (agent.get_leisure_ID() > 0) {
			if (agent.get_leisure_type() == "public") {
				Leisure& les_loc = leisure_locations.at(agent.get_leisure_ID()-1); 
				if (!les_loc.outside_town()){
					les_loc.add_exposed(inf_var, trans_rates.at("leisure locations transmission rate"), strain_id);
				}
			} else if (agent.get_leisure_type() == "household") {
				Household& household = households.at(agent.get_leisure_ID()-1);
				household.add_exposed(inf_var, trans_rates.at("household transmission rate"), strain_id);
			} else {
				throw std::invalid_argument("Wrong leisure type: " + agent.get_leisure_type());
			}
		}
	}
}

// Count contributions of a symptomatic agent
void Contributions::compute_symptomatic_contributions(const Agent& agent, const double time,	
					std::vector<Household>& households, std::vector<School>& schools,
					std::vector<Workplace>& workplaces, std::vector<Hospital>& hospitals,
					std::vector<RetirementHome>& retirement_homes,
			   		std::vector<Transit>& carpools, std::vector<Transit>& public_transit,
					std::vector<Leisure>& leisure_locations)
{
	// Agent's infection variability
	double inf_var = 0.0;
	inf_var = agent.get_inf_variability_factor();
	const std::map<std::string, double>& trans_rates = agent.curr_strain_tr_rates();
	const int strain_id = agent.get_strain();

	// If main state "tested"
	if (agent.tested()){
		// If awaiting results 
		if (agent.get_time_of_test() <= time && agent.tested_awaiting_test() == true){
			// If being tested at this step
			if (agent.tested_in_hospital() == true){
				compute_hospital_tested_contributions(agent, inf_var, hospitals);
			}else if (agent.tested_in_car() == true){
				return;
			}else{
				throw std::runtime_error("Agents testing site not specified");
			}
		} else if (agent.tested_awaiting_results() || agent.tested_awaiting_test()){
			// Assuming home isolation except for hospital patients formerly
			// non-COVID
			if (agent.hospital_non_covid_patient() == false){
				compute_home_isolated_contributions(agent, inf_var, households, retirement_homes);
			}else{
				compute_hospitalized_contributions(agent, inf_var, hospitals);
			}		
		}
	} else if (agent.being_treated()){ 
		// If getting treatment
		if (agent.home_isolated() == true){
			compute_home_isolated_contributions(agent, inf_var, households, retirement_homes);
		}else if (agent.hospitalized() == true){
			compute_hospitalized_contributions(agent, inf_var, hospitals);
		}else if (agent.hospitalized_ICU() == true){
			compute_hospitalized_ICU_contributions(agent, inf_var, hospitals);
		}
	} else {
		if ((agent.tested_false_negative() && (agent.hospital_non_covid_patient()))
						|| (agent.hospital_non_covid_patient())){
			Hospital& hospital = hospitals.at(agent.get_hospital_ID()-1);
			hospital.add_symptomatic_patient(inf_var, trans_rates.at("hospital patients transmission rate"), strain_id);	
		} else {
			// If regular symptomatic
			compute_regular_symptomatic_contributions(agent, inf_var, households,
								schools, workplaces, retirement_homes, carpools, 
								public_transit, leisure_locations);
		}
	}
}

// Compute the total contribution to infection probability at every place
void Contributions::total_place_contributions(std::vector<Household>& households, 
					std::vector<School>& schools, std::vector<Workplace>& workplaces, 
					std::vector<Hospital>& hospitals, 
					std::vector<RetirementHome>& retirement_homes,
					std::vector<Transit>& carpools, std::vector<Transit>& public_transit,
					std::vector<Leisure>& leisure_locations)
{	
	auto infected_contribution = [](Place& place){ place.compute_infected_contribution(); };

	std::for_each(households.begin(), households.end(), infected_contribution);
	std::for_each(retirement_homes.begin(), retirement_homes.end(), infected_contribution);
	std::for_each(schools.begin(), schools.end(), infected_contribution);
	std::for_each(workplaces.begin(), workplaces.end(), infected_contribution);
	std::for_each(hospitals.begin(), hospitals.end(), infected_contribution);
	std::for_each(carpools.begin(), carpools.end(), infected_contribution);
	std::for_each(public_transit.begin(), public_transit.end(), infected_contribution);
	std::for_each(leisure_locations.begin(), leisure_locations.end(), infected_contribution);
}

// Count contributions of a untreated and not tested symptomatic agent
void Contributions::compute_regular_symptomatic_contributions(const Agent& agent, 
				const double inf_var, std::vector<Household>& households, 
				std::vector<School>& schools, std::vector<Workplace>& workplaces,
				std::vector<RetirementHome>& retirement_homes,
				std::vector<Transit>& carpools, std::vector<Transit>& public_transit,
				std::vector<Leisure>& leisure_locations)
{
	// Special absenteeism correction if any
	double psi_s = 0.0;
	const std::map<std::string, double>& trans_rates = agent.curr_strain_tr_rates();
	const int strain_id = agent.get_strain();

	// Household or retirement home
	if (agent.retirement_home_resident()){
		RetirementHome& rh = retirement_homes.at(agent.get_household_ID()-1);
		rh.add_symptomatic(inf_var, trans_rates.at("RH resident transmission rate"), strain_id);
	} else {
		Household& household = households.at(agent.get_household_ID()-1);
		household.add_symptomatic(inf_var, trans_rates.at("household transmission rate"), strain_id);
	}

	// Other places
	if (agent.student() == true){
		School& school = schools.at(agent.get_school_ID()-1);
		school.add_symptomatic_student(inf_var, trans_rates.at("school transmission rate"), strain_id);	
	}
	if (agent.works() && !agent.works_from_home()) {
		if (agent.retirement_home_employee()){
			RetirementHome& rh = retirement_homes.at(agent.get_work_ID()-1);
			rh.add_symptomatic_employee(inf_var, trans_rates.at("RH employee transmission rate"), strain_id);
			psi_s = rh.get_absenteeism_correction();
		} else if (agent.school_employee()){
			School& sch = schools.at(agent.get_work_ID()-1);
			sch.add_symptomatic_employee(inf_var, trans_rates.at("school employee transmission rate"), strain_id);
			psi_s = sch.get_absenteeism_correction();
		} else {
			Workplace& workplace = workplaces.at(agent.get_work_ID()-1);
			if (!workplace.outside_town()) {
				workplace.add_symptomatic(inf_var, trans_rates.at("workplace transmission rate"), strain_id);
			}
		}
	}
	// Transit
	if (agent.get_work_travel_mode() == "carpool") {
		Transit& carpool = carpools.at(agent.get_carpool_ID()-1);
		if (agent.retirement_home_employee() || agent.school_employee()) {
			carpool.add_special_symptomatic(inf_var, psi_s, trans_rates.at("carpool transmission rate"), strain_id);
		} else {
			carpool.add_symptomatic(inf_var, trans_rates.at("carpool transmission rate"), strain_id);
		}
	}
	if (agent.get_work_travel_mode() == "public") {
		Transit& bus = public_transit.at(agent.get_public_transit_ID()-1);
		if (agent.retirement_home_employee() || agent.school_employee()) {
			bus.add_special_symptomatic(inf_var, psi_s, trans_rates.at("public transit transmission rate"), strain_id);
		} else {
			bus.add_symptomatic(inf_var, trans_rates.at("public transit transmission rate"), strain_id);
		}
	}
	// Leisure
	if (agent.get_leisure_ID() > 0) {
		if (agent.get_leisure_type() == "public") {
			Leisure& les_loc = leisure_locations.at(agent.get_leisure_ID()-1);
			if (!les_loc.outside_town()){
				les_loc.add_symptomatic(inf_var, trans_rates.at("leisure locations transmission rate"), strain_id);
			}
		} else if (agent.get_leisure_type() == "household") {
			Household& household = households.at(agent.get_leisure_ID()-1);
			household.add_symptomatic(inf_var, trans_rates.at("household transmission rate"), strain_id);
		} else {
			throw std::invalid_argument("Wrong leisure type: " + agent.get_leisure_type());
		}
	}
}

/// \brief Count contributions of a symptomatic-hospital tested agent
/// \details At that time step this only refers to testing
void Contributions::compute_hospital_tested_contributions(const Agent& agent, 
				const double inf_var, std::vector<Hospital>& hospitals)   
{
	Hospital& hospital = hospitals.at(agent.get_hospital_ID()-1);
	const std::map<std::string, double>& trans_rates = agent.curr_strain_tr_rates();
	const int strain_id = agent.get_strain();
	if (agent.exposed()){
		hospital.add_exposed_hospital_tested(inf_var, trans_rates.at("hospital tested transmission rate"), strain_id);
	}else{
		hospital.add_hospital_tested(inf_var, trans_rates.at("hospital tested transmission rate"), strain_id);
	}
	if (agent.home_isolated()){
		hospital.increase_total_tested();
	}
}

/// \brief Count contributions of a home-isolated agent 
void Contributions::compute_home_isolated_contributions(const Agent& agent, 
				const double inf_var, std::vector<Household>& households,
				std::vector<RetirementHome>& retirement_homes)   
{
	const std::map<std::string, double>& trans_rates = agent.curr_strain_tr_rates();
	const int strain_id = agent.get_strain();
	if (agent.retirement_home_resident()){
		RetirementHome& rh = retirement_homes.at(agent.get_household_ID()-1);
		if (agent.exposed()){
			rh.add_exposed_home_isolated(inf_var, trans_rates.at("RH transmission rate of home isolated"), strain_id);
		}else{
			rh.add_symptomatic_home_isolated(inf_var, trans_rates.at("RH transmission rate of home isolated"), strain_id);
		}			
	} else {
		Household& household = households.at(agent.get_household_ID()-1);
		if (agent.exposed()){
			household.add_exposed_home_isolated(inf_var, trans_rates.at("transmission rate of home isolated"), strain_id);
		}else{
			household.add_symptomatic_home_isolated(inf_var, trans_rates.at("transmission rate of home isolated"), strain_id);
		}	
	}
}

/// \brief Count contributions of a hospitalized agent
void Contributions::compute_hospitalized_contributions(const Agent& agent, 
				const double inf_var, std::vector<Hospital>& hospitals)   
{
	const std::map<std::string, double>& trans_rates = agent.curr_strain_tr_rates();
	const int strain_id = agent.get_strain();
	Hospital& hospital = hospitals.at(agent.get_hospital_ID()-1);
	hospital.add_hospitalized(inf_var, trans_rates.at("hospitalized transmission rate"), strain_id);
}

/// \brief Count contributions of an agent hospitalized in ICU
void Contributions::compute_hospitalized_ICU_contributions(const Agent& agent, 
				const double inf_var, std::vector<Hospital>& hospitals)   
{
	const std::map<std::string, double>& trans_rates = agent.curr_strain_tr_rates();
	const int strain_id = agent.get_strain();
	Hospital& hospital = hospitals.at(agent.get_hospital_ID()-1);
	hospital.add_hospitalized_ICU(inf_var, trans_rates.at("hospitalized ICU transmission rate"), strain_id);
}

/// \brief Set contributions/sums from all agents in places to 0.0 
void Contributions::reset_sums(std::vector<Household>& households, std::vector<School>& schools,
					std::vector<Workplace>& workplaces, std::vector<Hospital>& hospitals, 
					std::vector<RetirementHome>& retirement_homes,
					std::vector<Transit>& carpools, std::vector<Transit>& public_transit,
					std::vector<Leisure>& leisure_locations)
{
	auto reset_contributions = [](Place& place){ place.reset_contributions(); };

	std::for_each(households.begin(), households.end(), reset_contributions);
	std::for_each(retirement_homes.begin(), retirement_homes.end(), reset_contributions);
	std::for_each(schools.begin(), schools.end(), reset_contributions);
	std::for_each(workplaces.begin(), workplaces.end(), reset_contributions);
	std::for_each(hospitals.begin(), hospitals.end(), reset_contributions);
	std::for_each(carpools.begin(), carpools.end(), reset_contributions);
	std::for_each(public_transit.begin(), public_transit.end(), reset_contributions);
	std::for_each(leisure_locations.begin(), leisure_locations.end(), reset_contributions);
}


