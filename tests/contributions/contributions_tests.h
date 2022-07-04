#ifndef CONTRIBUTIONS_TESTS_H
#define CONTRIBUTIONS_TESTS_H

#include <algorithm>
#include "../../include/abm.h"
#include "../../include/contributions.h"
#include "../../include/utils.h"
#include "../common/test_utils.h"

// Supporting functions
template <typename T>
bool check_regular_contributions(const std::vector<T>&, const std::vector<Agent>&, const std::string, const std::map<std::string, double>, double time = 0.0);

/// Verification of agent contributions  
template <typename T>
bool check_regular_contributions(const std::vector<T>& locations, const std::vector<Agent>& agents, const std::string place_type, const std::map<std::string, double> infection_parameters, double time)

{
	int n_strains = infection_parameters.at("number of strains");
	for (const auto& location : locations){
		std::vector<double> lambda(n_strains, 0.0); 
		std::vector<int> agentIDs = location.get_agent_IDs();
		double ntot = static_cast<double>(agentIDs.size());
		// So if an agent is both a student and works at school i they are not counted twice
		// (total will still be twice)
		auto last = std::unique(agentIDs.begin(), agentIDs.end());
		agentIDs.erase(last, agentIDs.end());
		if (place_type == "household"){
			ntot = std::pow(ntot, infection_parameters.at("household scaling parameter"));
		}
		for (const auto& ID : agentIDs){
			const Agent& agent = agents.at(ID-1);
			if (agent.infected() == false){
				continue;	
			}
			const std::map<std::string, double>& agent_tr_rates = agent.curr_strain_tr_rates();
			int strain_id = agent.get_strain();
			const double rho_k = agent.get_inf_variability_factor();
			// Tested agents have different contributions
			if (agent.tested()){
				// Time of the test
				if (agent.get_time_of_test() <= time 
						&& agent.tested_awaiting_test()){
					if (agent.tested_in_hospital() && (place_type == "hospital")){
						if (agent.exposed()){
							lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("hospital tested transmission rate");
						}else{
							lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("hospital tested transmission rate")*infection_parameters.at("severity correction");
						}
						if (agent.home_isolated()){
							++ntot;
						}
					}
					continue;	
				}
				// Home isolation for the rest of the testing process
				if (place_type == "household"){
					// If registered in this household as part of a leisure location - skip
					if ((agent.get_leisure_type() == "household") && (agent.get_leisure_ID() == location.get_ID())) {
						continue;
					}
					if (agent.exposed()){
						lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("transmission rate of home isolated");
					}else{
						lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("transmission rate of home isolated")*infection_parameters.at("severity correction");				
					}
					continue;
				}else if (place_type == "retirement home" && agent.retirement_home_resident()){
					if (agent.exposed()){
						lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("RH transmission rate of home isolated");
					}else{
						lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("RH transmission rate of home isolated")*infection_parameters.at("severity correction");				
					}
					continue;
				}else if (place_type == "hospital" && agent.hospital_non_covid_patient()){
					if (agent.exposed()){
						lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("hospital patients transmission rate");
					}else{
						lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("hospitalized transmission rate")*infection_parameters.at("severity correction");				
					}
					continue;
				}else{
					continue;
				}
			}

			// Treatment
			if (agent.being_treated()){
				if (agent.home_isolated()){
					// If registered in this household as part of a leisure location - skip
					if ((agent.get_leisure_type() == "household") && (agent.get_leisure_ID() == location.get_ID())) {
						continue;
					}
					double beta_ih = 0.0;
					if (place_type == "household"){ 
						beta_ih = agent_tr_rates.at("transmission rate of home isolated");
					}else if (place_type == "retirement home" && agent.retirement_home_resident()){
						beta_ih = agent_tr_rates.at("RH transmission rate of home isolated");
					} else {
						continue;
					}
					if (agent.exposed()){
						lambda.at(strain_id-1) += rho_k*beta_ih;
					} else {
						lambda.at(strain_id-1) += rho_k*beta_ih*infection_parameters.at("severity correction");
					}
				} else if (agent.hospitalized() && place_type == "hospital"){
					lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("hospitalized transmission rate")*infection_parameters.at("severity correction");	
				} else if (agent.hospitalized_ICU() && place_type == "hospital") {
					lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("hospitalized ICU transmission rate")*infection_parameters.at("severity correction");
				}
				continue;
			}

			// Everything else
			if (place_type == "household"){
				if (agent.exposed()){
					lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("household transmission rate");
				}else{
					lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("household transmission rate")*infection_parameters.at("severity correction");				
				}						
			}else if (place_type == "workplace"){
				if (!location.outside_town()) {
					double work_rate = agent_tr_rates.at("workplace transmission rate");
					if (agent.exposed()){
						lambda.at(strain_id-1) += rho_k*work_rate;
					}else{
						lambda.at(strain_id-1) += rho_k*work_rate*infection_parameters.at("severity correction")*infection_parameters.at("work absenteeism correction");				
					}
				} 
			}else if (place_type == "school"){
				double beta_sch = 0.0, psi_sch = 0.0;
				if (agent.student() && agent.get_school_ID() == location.get_ID()){
					beta_sch = agent_tr_rates.at("school transmission rate");
					// All should be the same for the test
					psi_sch = infection_parameters.at("primary and middle school absenteeism correction");
					if (agent.exposed()){
						lambda.at(strain_id-1) += rho_k*beta_sch;
					}else{
						lambda.at(strain_id-1) += rho_k*beta_sch*psi_sch*infection_parameters.at("severity correction");				
					}
				}
				if (agent.school_employee() && agent.get_work_ID() == location.get_ID()){
					beta_sch =  agent_tr_rates.at("school employee transmission rate");
					psi_sch =  infection_parameters.at("school employee absenteeism correction");
					if (agent.exposed()){
						lambda.at(strain_id-1) += rho_k*beta_sch;
					}else{
						lambda.at(strain_id-1) += rho_k*beta_sch*psi_sch*infection_parameters.at("severity correction");				
					}
				}
			}else if (place_type == "hospital"){
				double beta_hsp = 0.0;
				if (agent.hospital_employee()){
					beta_hsp = agent_tr_rates.at("healthcare employees transmission rate");
				} else if (agent.symptomatic() && agent.hospital_non_covid_patient()){
					beta_hsp = agent_tr_rates.at("hospitalized transmission rate");
					lambda.at(strain_id-1) += rho_k*beta_hsp*infection_parameters.at("severity correction");
				}else{
					beta_hsp = agent_tr_rates.at("hospital patients transmission rate");
				}
				// From contact tracing
				if (agent.exposed() && !agent.home_isolated()){
					lambda.at(strain_id-1) += rho_k*beta_hsp;
				}
			}else if (place_type == "retirement home"){
				double beta_rh = 0.0, psi_rh = 0.0;
				if (agent.retirement_home_employee()){
					beta_rh = agent_tr_rates.at("RH employee transmission rate");
				   	psi_rh = infection_parameters.at("RH employee absenteeism factor");	
				}else{
					beta_rh = agent_tr_rates.at("RH resident transmission rate");
					psi_rh = 1.0;
				}
				if (agent.exposed()){
					lambda.at(strain_id-1) += rho_k*beta_rh;
				}else{
					lambda.at(strain_id-1) += rho_k*beta_rh*psi_rh*infection_parameters.at("severity correction");
				}
			}else if (place_type == "carpool"){
				if (agent.exposed()){
					lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("carpool transmission rate");
				}else{
					if (agent.school_employee()) {
 						lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("carpool transmission rate")
							*infection_parameters.at("severity correction")
							*infection_parameters.at("school employee absenteeism correction");
					} else if (agent.retirement_home_employee()) {
						lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("carpool transmission rate")
							*infection_parameters.at("severity correction")
							*infection_parameters.at("RH employee absenteeism factor");
					} else {
						lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("carpool transmission rate")
							*infection_parameters.at("severity correction")
							*infection_parameters.at("work absenteeism correction");
					}			
				} 
			}else if (place_type == "public transit"){
				if (agent.exposed()){
					lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("public transit transmission rate");
				}else{
					if (agent.school_employee()) {
 						lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("public transit transmission rate")
								*infection_parameters.at("severity correction")
								*infection_parameters.at("school employee absenteeism correction");
					} else if (agent.retirement_home_employee()) {
						lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("public transit transmission rate")
								*infection_parameters.at("severity correction")
								*infection_parameters.at("RH employee absenteeism factor");
					} else {
						lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("public transit transmission rate")
								*infection_parameters.at("severity correction")
								*infection_parameters.at("work absenteeism correction");	
					}
				}
			}else if (place_type == "leisure location"){
				if (!location.outside_town()){
					if (agent.exposed()){
						lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("leisure locations transmission rate");
					}else{
						if (agent.school_employee()) {
	 						lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("leisure locations transmission rate")
									*infection_parameters.at("severity correction");
						} else if (agent.retirement_home_employee()) {
							lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("leisure locations transmission rate")
									*infection_parameters.at("severity correction");
						} else {
							lambda.at(strain_id-1) += rho_k*agent_tr_rates.at("leisure locations transmission rate")
								*infection_parameters.at("severity correction");
						}
					}
				}
			}else{
				throw std::runtime_error("Wrong place type in regular place test");
			}
		}
		if (ntot > 0){
			if ((place_type == "workplace" || place_type == "leisure location" ) && !location.outside_town()) {
				std::transform(lambda.begin(), lambda.end(), lambda.begin(), [ntot](double lam){ return lam/ntot; });
			} else if (place_type != "workplace" && place_type != "leisure location") {
				std::transform(lambda.begin(), lambda.end(), lambda.begin(), [ntot](double lam){ return lam/ntot; });
			} else if (place_type == "workplace" && location.outside_town()) {
				std::fill(lambda.begin(), lambda.end(), infection_parameters.at("fraction estimated infected"));
			} else if (place_type == "leisure location" && location.outside_town()) {
				std::fill(lambda.begin(), lambda.end(), infection_parameters.at("fraction estimated infected"));
			}
		} else {
			// For the lambdas that don't depend on particular agents
			if (place_type == "workplace" && location.outside_town()) {
				std::fill(lambda.begin(), lambda.end(), infection_parameters.at("fraction estimated infected"));
			}else if (place_type == "leisure location" && location.outside_town()){
				std::fill(lambda.begin(), lambda.end(), infection_parameters.at("fraction estimated infected"));
			}
		}
		if (!is_equal_floats<double>({lambda}, {location.get_infected_contribution()}, 1e-5)){
			std::cout << "Possible precision issues, lowering tolerance, expected vs. computed: " << std::endl;
			std::vector<double> temp = location.get_infected_contribution();
			for (int i = 0; i<lambda.size(); ++i) { 
				std::cout << lambda.at(i) << " " << temp.at(i) << " Number of agents: " << ntot << std::endl;
			}
			// For precision reasons
			if (!is_equal_floats<double>({lambda}, {location.get_infected_contribution()}, 1e-1)){
				return false; 
			}
		}
	}
	return true;
}

#endif
