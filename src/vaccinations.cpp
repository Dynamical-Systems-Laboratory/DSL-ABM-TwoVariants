#include "../include/vaccinations.h"

// Load parameters related to vaccinations store in a map
void Vaccinations::load_vaccination_parameters(const std::string& infile, const std::string& data_dir)
{
	// Collect and store the parameters
	LoadParameters ldparam;
	vaccination_parameters = ldparam.load_parameter_map<double>(infile);
	// Information on other strains
	strain_id = static_cast<int>(vaccination_parameters.at("Strain id"));
	num_strains = static_cast<int>(vaccination_parameters.at("Number of strains"));
	for (int i = 1; i <= num_strains; ++i) {
		std::map<std::string, double> red_factor;
		if (i != strain_id) {
			red_factor["effectiveness"] = vaccination_parameters.at(std::string("Effectiveness reduction for strain ") + std::to_string(i));
			red_factor["asymptomatic"] = vaccination_parameters.at(std::string("Asymptomatic reduction for strain ") + std::to_string(i));
			red_factor["transmission"] = vaccination_parameters.at(std::string("Transmission reduction for strain ") + std::to_string(i));
			red_factor["severe"] = vaccination_parameters.at(std::string("Severe reduction for strain ") + std::to_string(i));
			red_factor["death"] = vaccination_parameters.at(std::string("Death reduction for strain ") + std::to_string(i));	
			other_strains.push_back(red_factor);
		} else {
			other_strains.push_back({{"placeholder", 0.0}});
		}
	}
	// Loading and storing of time, value pairs for creating the time dependencies
	// All the one dose types 
	int num_one_dose = static_cast<int>(vaccination_parameters.at("Number of one dose types"));
	for (int i=1; i<=num_one_dose; ++i) {
		std::string file_name = data_dir + "one_dose_vac_type_" + std::to_string(i) + ".txt";
		std::string tag = "one dose - type "+ std::to_string(i);
		vac_types_properties[tag] = ldparam.load_table(file_name);
		vac_types_probs["one dose CDF"].push_back(vaccination_parameters.at(tag + " probability vaccinated, CDF"));	
		for (int os = 1; os <= num_strains; ++os) {
			if (os != strain_id) {
				std::string other_tag = tag + " other strain " + std::to_string(os);
				add_other_strain(tag, other_tag, other_strains.at(os-1));	
			} 
		}
	}
	// All the two dose types
	int num_two_dose = static_cast<int>(vaccination_parameters.at("Number of two dose types"));
	for (int i=1; i<=num_two_dose; ++i) {
		std::string file_name = data_dir + "two_dose_vac_type_" + std::to_string(i) + ".txt";
		std::string tag = "two dose - type "+ std::to_string(i);
		vac_types_properties[tag] = ldparam.load_table(file_name);
		vac_types_probs["two dose CDF"].push_back(vaccination_parameters.at(tag + " probability vaccinated, CDF"));		
		for (int os = 1; os <= num_strains; ++os) {
			if (os != strain_id) {
				std::string other_tag = tag + " other strain " + std::to_string(os);
				add_other_strain(tag, other_tag, other_strains.at(os-1));	
			}
		}
	}

// For debugging
/*	if (strain_id == 1) {
		for (auto& vtype : vac_types_properties) {
			std::cout << vtype.first << std::endl;
			for (auto& benefit : vtype.second) {
				std::cout << benefit.first << std::endl;
				for (auto& prop : benefit.second) {
					std::cout << prop.at(0) << " " << prop.at(1) << std::endl;
				}
			}
		}
	}
*/
}

/// Create a parameter entry in vaccination_parameters for another strain
void Vaccinations::add_other_strain(const std::string& this_tag, const std::string& other_tag, 
										const std::map<std::string, double>& reduction)
{
	const std::map<std::string, std::vector<std::vector<double>>>& this_strain = vac_types_properties.at(this_tag);
	std::map<std::string, std::vector<std::vector<double>>> other_strain;
	double temp_val;
	for (const auto& benefit : this_strain) {
		other_strain[benefit.first] = benefit.second;
		for (auto& one_point : other_strain[benefit.first]){
			temp_val = one_point.at(1);
			if (benefit.first == "effectiveness") {
				one_point.at(1) = -reduction.at(benefit.first) + temp_val 
									+ reduction.at(benefit.first) - reduction.at(benefit.first)*temp_val;
			} else {
				one_point.at(1) = reduction.at(benefit.first) + temp_val - reduction.at(benefit.first)*temp_val;
			}
		}
	}
	vac_types_properties[other_tag] = other_strain;
}

/// Load and shuffle time custom offsets for vaccines 
void Vaccinations::load_and_shuffle_time_offsets(const std::string& offset_file, Infection& infection)
{
	// Unused options of the AbmIO object
	std::string delim(" ");
	bool one_file = true;
	std::vector<size_t> dims = {0,0,0};

	AbmIO io(offset_file, delim, one_file, dims);
	time_offsets =  io.read_rows<double>();
	infection.vector_shuffle(time_offsets);
}

/// Load and shuffle time custom offsets for vaccines and boosters
void Vaccinations::load_and_shuffle_time_offsets(const std::string& offset_file, 
			const std::string& offset_file_boosters, Infection& infection)
{
	// Unused options of the AbmIO object
	std::string delim(" ");
	bool one_file = true;
	std::vector<size_t> dims = {0,0,0};

	AbmIO io(offset_file, delim, one_file, dims);
	time_offsets =  io.read_rows<double>();
	infection.vector_shuffle(time_offsets);

	AbmIO ioB(offset_file_boosters, delim, one_file, dims);
	time_offsets_boosters =  ioB.read_rows<double>();
	infection.vector_shuffle(time_offsets_boosters);
}

// Copy parameters in v1 into v2
void Vaccinations::copy_vaccination_dependencies(std::forward_list<double>&& lst,
									std::vector<std::vector<double>>& vec)
{
	std::vector<double> temp(2, 0.0);
	while (!lst.empty()) {
		// Collect point pairs (time and value)
		temp.at(0) = lst.front();
		lst.pop_front();
		temp.at(1) = lst.front();
		lst.pop_front();
		// Store the pair in the main vector
		vec.push_back(temp);
	}
}

// Randomly vaccinates requested number of agents
std::vector<int> Vaccinations::vaccinate_random(std::vector<Agent>& agents, int n_vac, 
							int n_boost, Infection& infection, const double time)
{
	// Pick ones that can be vaccinated
	int max_boost = 0;
	std::vector<int> can_be_vaccinated = filter_general(agents, max_boost);
	if (can_be_vaccinated.empty()) {
		std::cout << "No more agents eligible for random vaccination" << std::endl;
		return {0, 0};
	}
	// Reduce n_vac and n_boost if larger than available
	if (n_vac + n_boost > can_be_vaccinated.size()) {
		n_vac = can_be_vaccinated.size();
		n_boost = max_boost;
		std::cout << "Requested number of agents for random vaccination"
				  << " larger than currently eligible -- decreasing to " 
				  << n_vac << std::endl;
	}
	// If not processing all avaiblable,
	// randomly shuffle the indices, then vaccinate first n_vac
	if (n_vac != can_be_vaccinated.size()) {
		infection.vector_shuffle(can_be_vaccinated);
		can_be_vaccinated.resize(n_vac);		
	}	
	// Vaccinate and set agent properties
	vaccinate_and_setup(agents, can_be_vaccinated, infection, time);
	return {n_vac - n_boost, n_boost};
}

// Randomly vaccinates requested number of agents with a negative time offset
int Vaccinations::vaccinate_random_time_offset(std::vector<Agent>& agents, int n_vac, 
							const int n_boost, Infection& infection, const double time)
{
	// Pick ones that can be vaccinated
	int max_boost = 0;
	std::vector<int> can_be_vaccinated = filter_general(agents, max_boost);
	if (can_be_vaccinated.empty()) {
		std::cout << "No more agents eligible for random vaccination" << std::endl;
		return 0;
	}
	// Reduce n_vac if larger than available
	if (n_vac > can_be_vaccinated.size()) {
		n_vac = can_be_vaccinated.size();
		std::cout << "Requested number of agents for random vaccination"
				  << " larger than currently eligible -- decreasing to " 
				  << n_vac << std::endl;
	}
	// If not processing all avaiblable,
	// randomly shuffle the indices, then vaccinate first n_vac
	if (n_vac != can_be_vaccinated.size()) {
		infection.vector_shuffle(can_be_vaccinated);
		can_be_vaccinated.resize(n_vac);		
	}	
	// Vaccinate and set agent properties
	vaccinate_and_setup_time_offset(agents, can_be_vaccinated, infection, time, n_boost);
	return n_vac;
}

// Randomly vaccinates requested number of agents
int Vaccinations::vaccinate_group(std::vector<Agent>& agents, const std::string& group_name,
									int n_vac, Infection& infection, const double time,
									const bool vaccinate_all)
{
	// Pick ones that can be vaccinated
	std::vector<int> can_be_vaccinated = filter_general_and_group(agents, group_name);
	if (can_be_vaccinated.empty()) {
		std::cout << "No more agents eligible for vaccination of group " 
				  << group_name << std::endl;
		return 0;
	}
	// Reduce n_vac if larger than available
	if (n_vac > can_be_vaccinated.size()) {
		n_vac = can_be_vaccinated.size();
		std::cout << "Requested number of agents for vaccination of group "
				  << group_name 
				  << " larger than currently eligible -- decreasing to " 
				  << n_vac << std::endl;
	} else if (vaccinate_all){
		// If all are requested to be vaccinated
		n_vac =  can_be_vaccinated.size();
		std::cout << "Vaccinating all " << n_vac << " eligible agents in group "
				  << group_name << std::endl;
	}
	// If not processing all avaiblable,
	// randomly shuffle the indices, then vaccinate first n_vac
	if (n_vac != can_be_vaccinated.size()) {
		infection.vector_shuffle(can_be_vaccinated);
		can_be_vaccinated.resize(n_vac);		
	}	
	// Vaccinate and set agent properties
	vaccinate_and_setup(agents, can_be_vaccinated, infection, time);
	return n_vac;
}

// Returns maximum number of agents currently eligible for vaccination
int Vaccinations::max_eligible_random(const std::vector<Agent>& agents)
{
	int max_boost = 0;
	std::vector<int> eligible_IDs = filter_general(agents, max_boost);
	return eligible_IDs.size();
}

// Returns maximum number of agents in a group currently eligible for vaccination
int Vaccinations::max_eligible_group(const std::vector<Agent>& agents, const std::string& group_name)
{
	std::vector<int> eligible_IDs = filter_general_and_group(agents, group_name);
	return eligible_IDs.size();
}

// Select agents eligible for vaccination based on criteria valid for all agents
std::vector<int> Vaccinations::filter_general(const std::vector<Agent>& agents, int& max_boost)
{
	std::vector<int> eligible_agents;
	for (const auto& agent : agents) {
		// Verify if the agent meets all the core requirements
		if (check_general(agent, max_boost)) {
			eligible_agents.push_back(agent.get_ID());
		} 
	}
	return eligible_agents;	
}

// Select agents in a given group eligible for vaccination based on criteria valid for all agents
std::vector<int> Vaccinations::filter_general_and_group(const std::vector<Agent>& agents, 
									const std::string& group_name)
{
	std::vector<int> eligible_agents;
	int max_boost = 0;
	for (const auto& agent : agents) {
		// First check if the agent belongs to the target group
		if (check_group(agent, group_name)) {
			// Then verify if the agent meets all the core requirements
			if (check_general(agent, max_boost)) {
				eligible_agents.push_back(agent.get_ID());
			} 
		}
	}
	return eligible_agents;
}

// True if agent meets core criteria for vaccination eligibility
bool Vaccinations::check_general(const Agent& agent, int& max_boost)
{
	if (agent.is_vaccinated_for_strain(strain_id) && !agent.needs_next_vaccination()) {
		return false;
	}
	if (agent.is_vaccinated_for_strain(strain_id) && agent.needs_next_vaccination()) {
		++max_boost;
	}
	if (agent.removed_dead()) {
		return false;
	} 
	if (agent.get_age() < vaccination_parameters.at("Minimum vaccination age")) {
		return false;	
	}
	if (agent.tested_covid_positive()) {
		return false; 
	}
	if (agent.removed() && !agent.removed_can_vaccinate()) {
		return false;
	}
 	if (agent.former_suspected() && !agent.suspected_can_vaccinate()) {
		return false;
	}
	if (agent.symptomatic()) {
		return false;
	}
	if (agent.symptomatic_non_covid()) {
		return false;
	}
	if (agent.home_isolated()) {
		return false;
	}
	if (agent.contact_traced()) {
		return false; 
	}
	return true;
}

// True if agent is in the target vaccination group 
bool Vaccinations::check_group(const Agent& agent, const std::string& vaccine_group_name)
{
	if (vaccine_group_name == "hospital employees"
			&& agent.hospital_employee()) {
		return true;
	} else if (vaccine_group_name == "school employees"
			&& agent.school_employee()) {
		return true;
	} else if (vaccine_group_name == "retirement home employees"
			&& agent.retirement_home_employee()) {
		return true;
	} else if (vaccine_group_name == "retirement home residents"
			&& agent.retirement_home_resident()) {
		return true;
	} 	
	return false;
}

// Vaccinates agents with provided IDs and sets all the agent properties
void Vaccinations::vaccinate_and_setup(std::vector<Agent>& agents, std::vector<int>& agent_IDs, 
										Infection& infection, const double time)
{
	for (auto& id : agent_IDs) {
		Agent& agent = agents.at(id-1);
		// Third dose
		if (agent.vaccinated() && agent.needs_next_vaccination() && agent.is_vaccinated_for_strain(strain_id)) {
			double next_step = vaccination_parameters.at("Third dose max effects time");
			double max_end = vaccination_parameters.at("Third dose max effects end time");
			double tot_end = vaccination_parameters.at("Third dose no effects time");
			std::string vac_type = agent.get_vaccine_type(strain_id);
			std::string tag = agent.get_vaccine_subtype(strain_id);
			set_booster(agent, tag, time, next_step, max_end, tot_end);
			agent.set_up_to_date(true);
			agent.set_got_booster(true);
			continue;
		}
		// First vaccination ever
		agent.set_vaccinated(true);
		agent.set_needs_next_vaccination(false);
		if (infection.get_uniform() <= vaccination_parameters.at("Fraction taking one dose vaccine")) {
			agent.set_vaccine_type("one_dose", strain_id);
			// Select the type based on the iterator in the CDF
			// This assumes all types are loaded sequentially
			std::vector<double> one_dose_probs = vac_types_probs.at("one dose CDF");
			double cur_prob = infection.get_uniform();
			const auto& iter = std::find_if(one_dose_probs.cbegin(), one_dose_probs.cend(), 
					[&cur_prob](const double x) { return x >= cur_prob; });
			// Types start with 1
			std::string tag = "one dose - type "+ std::to_string(std::distance(one_dose_probs.cbegin(), iter) + 1);
			agent.set_vaccine_subtype(tag, strain_id);
			set_regular_one_dose(agent, tag, time);
		} else {
			agent.set_vaccine_type("two_doses", strain_id);
			// Select the type based on the iterator in the CDF
			// This assumes all types are loaded sequentially
			std::vector<double> two_dose_probs = vac_types_probs.at("two dose CDF");
			double cur_prob = infection.get_uniform();
			const auto& iter = std::find_if(two_dose_probs.cbegin(), two_dose_probs.cend(), 
					[&cur_prob](const double x) { return x >= cur_prob; });
			// Types start with 1
			std::string tag = "two dose - type "+ std::to_string(std::distance(two_dose_probs.cbegin(), iter) + 1);
			agent.set_vaccine_subtype(tag, strain_id);
			set_regular_two_dose(agent, tag, time);
		}
	}
}

// Vaccinates agents with provided IDs and sets all the agent properties while applying a negative time offset
void Vaccinations::vaccinate_and_setup_time_offset(std::vector<Agent>& agents, std::vector<int>& agent_IDs, 
										Infection& infection, const double time, const int n_boosted)
{
	const double t0 = vaccination_parameters.at("Start of time offset interval");
	const double tf = vaccination_parameters.at("End of time offset interval");
	double offset = 0, offset_booster = 0;
	for (auto& id : agent_IDs) {
		Agent& agent = agents.at(id-1);
		// First vaccination ever
		agent.set_vaccinated(true);
		agent.set_up_to_date(true);
		agent.set_needs_next_vaccination(false);
		// This amount of time will be subtracted from the current time
		if (use_offsets_from_file) {
			//std::cout << "Offsets from a custom distribution" << std::endl;
			offset = time_offsets.at(infection.get_int(0, time_offsets.size()-1)); 		
			if (!time_offsets_boosters.empty()) {
				offset_booster = time_offsets_boosters.at(infection.get_int(0, time_offsets_boosters.size()-1));
			} else {
				offset_booster = offset;
			} 
		} else {
			//std::cout << "Offsets from a uniform distribution" << std::endl;
			offset = -1.0*infection.get_uniform(t0, tf);
		}
		agent.set_vac_time_offset(offset);
		if ( infection.get_uniform() <= vaccination_parameters.at("Fraction taking one dose vaccine")) {
			agent.set_vaccine_type("one_dose", strain_id);
			// Select the type based on the iterator in the CDF
			// This assumes all types are loaded sequentially
			std::vector<double> one_dose_probs = vac_types_probs.at("one dose CDF");
			double cur_prob = infection.get_uniform();
			const auto& iter = std::find_if(one_dose_probs.cbegin(), one_dose_probs.cend(), 
					[&cur_prob](const double x) { return x >= cur_prob; });
			// Types start with 1
			std::string tag = "one dose - type "+ std::to_string(std::distance(one_dose_probs.cbegin(), iter) + 1);
			agent.set_vaccine_subtype(tag, strain_id);
			set_regular_one_dose(agent, tag, offset);
//			std::cout << "One dose" << std::endl;
		} else {
			// One of the two dose vaccines is a booster
			agent.set_vaccine_type("two_doses", strain_id);
			double cur_prob = infection.get_uniform();
			// Types start with 1
			std::string tag = "two dose - type 1";

			if (cur_prob <= vaccination_parameters.at("Fraction with boosters")) {
				offset = offset_booster;
				tag = "two dose - type 2";
			}

			agent.set_vaccine_subtype(tag, strain_id);
			set_regular_two_dose(agent, tag, offset);
		}
	}

	// Distribute boosters
/*	infection.vector_shuffle(agent_IDs);
	// Third dose with an offset
	int tot_boosted = 0;
	for (auto& id : agent_IDs) {
		Agent& agent = agents.at(id-1);
		double booster_time = agent.get_time_vaccine_effects_reduction();
		offset = -1.0*infection.get_uniform(booster_time, time);
		if (booster_time <= time) {
			if (tot_boosted >= n_boosted) {
				agent.set_up_to_date(false);
				continue;
			}
			// Assumes booster is given exactly when eligible
			agent.set_vaccinated(true);
			agent.set_needs_next_vaccination(false);
			double next_step = vaccination_parameters.at("Third dose max effects time");
			double max_end = vaccination_parameters.at("Third dose max effects end time");
			double tot_end = vaccination_parameters.at("Third dose no effects time");
			std::string vac_type = agent.get_vaccine_type(strain_id);
			std::string tag = agent.get_vaccine_subtype(strain_id);
			set_booster(agent, tag, offset, next_step, max_end, tot_end);
			agent.set_up_to_date(true);
			agent.set_got_booster(true);
			++tot_boosted;	
		}
	}
*/
}

// Assign benefits for this strain and other relevant strains for one dose vaccine
void Vaccinations::set_regular_one_dose(Agent& agent, const std::string& tag, const double time)
{
	// For the current strain
	agent.set_vaccinated_target_strain(strain_id);
	// Now access attributes for the three part functions and set-up agent properties		
	agent.set_vaccine_effectiveness(ThreePartFunction(vac_types_properties.at(tag).at("effectiveness"), time), strain_id);
	agent.set_asymptomatic_correction(ThreePartFunction(vac_types_properties.at(tag).at("asymptomatic"), time), strain_id);
	agent.set_transmission_correction(ThreePartFunction(vac_types_properties.at(tag).at("transmission"), time), strain_id);
	agent.set_severe_correction(ThreePartFunction(vac_types_properties.at(tag).at("severe"), time), strain_id);
	agent.set_death_correction(ThreePartFunction(vac_types_properties.at(tag).at("death"), time), strain_id);
	// Record the time when vaccine effects start dropping (assumes all these properties follow the same trend)
	agent.set_time_vaccine_effects_reduction(time+vac_types_properties.at(tag).at("effectiveness").at(2).at(0));
	// and the time when mobility increases (at peak effectiveness)
	agent.set_time_mobility_increase(time+vac_types_properties.at(tag).at("effectiveness").at(1).at(0));

	// For all other strains (except ones that received their target vaccine already)
	for (int i = 1; i<=num_strains; ++i) {
		if ((i != strain_id) && !agent.is_vaccinated_for_strain(i)) {
			agent.set_vaccine_type("one_dose", i);
			// Set the tag for this strain and vac type (make compatible with tag)
			std::string other_tag = tag + " other strain " + std::to_string(i);
			// Set the reduced benefits
			agent.set_vaccine_effectiveness(ThreePartFunction(vac_types_properties.at(other_tag).at("effectiveness"), time), i);
			agent.set_asymptomatic_correction(ThreePartFunction(vac_types_properties.at(other_tag).at("asymptomatic"), time), i);
			agent.set_transmission_correction(ThreePartFunction(vac_types_properties.at(other_tag).at("transmission"), time), i);
			agent.set_severe_correction(ThreePartFunction(vac_types_properties.at(other_tag).at("severe"), time), i);
			agent.set_death_correction(ThreePartFunction(vac_types_properties.at(other_tag).at("death"), time), i);						
		}
	}
}

// Assign benefits for this strain and other relevant strains for two dose vaccine
void Vaccinations::set_regular_two_dose(Agent& agent, const std::string& tag, const double time)
{
	// For the current strain
	agent.set_vaccinated_target_strain(strain_id);
	// Now access attributes for the four part functions and set-up agent properties		
	agent.set_vaccine_effectiveness(FourPartFunction(vac_types_properties.at(tag).at("effectiveness"), time), strain_id);
	agent.set_asymptomatic_correction(FourPartFunction(vac_types_properties.at(tag).at("asymptomatic"), time), strain_id);
	agent.set_transmission_correction(FourPartFunction(vac_types_properties.at(tag).at("transmission"), time), strain_id);
	agent.set_severe_correction(FourPartFunction(vac_types_properties.at(tag).at("severe"), time), strain_id);
	agent.set_death_correction(FourPartFunction(vac_types_properties.at(tag).at("death"), time), strain_id);
	// Record the time when vaccine effects start dropping (assumes all these properties follow the same trend)
	agent.set_time_vaccine_effects_reduction(time+vac_types_properties.at(tag).at("effectiveness").at(3).at(0));
	// and the time when mobility increases (at peak effectiveness)
	agent.set_time_mobility_increase(time+vac_types_properties.at(tag).at("effectiveness").at(2).at(0));

	// For all other strains (except ones that received their target vaccine already)
	for (int i = 1; i<=num_strains; ++i) {
		if ((i != strain_id) && !agent.is_vaccinated_for_strain(i)) {
			agent.set_vaccine_type("two_doses", i);
			// Set the tag for this strain and vac type (make compatible with tag)
			std::string other_tag = tag + " other strain " + std::to_string(i);
			// Set the reduced benefits
			agent.set_vaccine_effectiveness(FourPartFunction(vac_types_properties.at(other_tag).at("effectiveness"), time), i);
			agent.set_asymptomatic_correction(FourPartFunction(vac_types_properties.at(other_tag).at("asymptomatic"), time), i);
			agent.set_transmission_correction(FourPartFunction(vac_types_properties.at(other_tag).at("transmission"), time), i);
			agent.set_severe_correction(FourPartFunction(vac_types_properties.at(other_tag).at("severe"), time), i);
			agent.set_death_correction(FourPartFunction(vac_types_properties.at(other_tag).at("death"), time), i);						
		}
	}
}

// Assign benefits for a booster that consider other relevant strains
void Vaccinations::set_booster(Agent& agent, const std::string& tag, const double time, 
								const double next_step, const double max_end, const double tot_end)
{
	// Construct for each benefit: this step, current value | next step, max value | then as usual
	// 1) Effectiveness
	std::vector<std::vector<double>> orig_props = vac_types_properties.at(tag).at("effectiveness");
	double max_benefit = orig_props.at(orig_props.size()-2).at(1);
	double ini_benefit = time < 0.0 ? max_benefit : agent.vaccine_effectiveness(time, strain_id);
	std::vector<std::vector<double>> new_eff{{0.0, ini_benefit}, 
										{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 0.0}};
	agent.set_vaccine_effectiveness(ThreePartFunction(new_eff, time), strain_id);

	// 2) Asymptomatic correction
	orig_props = vac_types_properties.at(tag).at("asymptomatic");
	max_benefit = orig_props.at(orig_props.size()-2).at(1);
	ini_benefit = time < 0.0 ? max_benefit : agent.asymptomatic_correction(time, strain_id);
	std::vector<std::vector<double>> new_asm{{0.0, ini_benefit}, 
										{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 1.0}};
	agent.set_asymptomatic_correction(ThreePartFunction(new_asm, time), strain_id);

	// 3) Transmission correction
	orig_props = vac_types_properties.at(tag).at("transmission");
	max_benefit = orig_props.at(orig_props.size()-2).at(1);
	ini_benefit = time < 0.0 ? max_benefit : agent.transmission_correction(time, strain_id);
	std::vector<std::vector<double>> new_tr{{0.0, ini_benefit}, 
										{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 1.0}};
	agent.set_transmission_correction(ThreePartFunction(new_tr, time), strain_id);	

	// 4) Severity correction
	orig_props = vac_types_properties.at(tag).at("severe");
	max_benefit = orig_props.at(orig_props.size()-2).at(1);
	ini_benefit = time < 0.0 ? max_benefit : agent.severe_correction(time, strain_id);
	std::vector<std::vector<double>> new_sv{{0.0, ini_benefit}, 
										{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 1.0}};
	agent.set_severe_correction(ThreePartFunction(new_sv, time), strain_id);

	// 5) Death correction
	orig_props = vac_types_properties.at(tag).at("death");
	max_benefit = orig_props.at(orig_props.size()-2).at(1);
	ini_benefit = time < 0.0 ? max_benefit : agent.death_correction(time, strain_id);
	std::vector<std::vector<double>> new_dth{{0.0, ini_benefit}, 
										{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 1.0}};
//	std::cout << ini_benefit << " " << max_benefit << " " << time << " " << agent.death_correction(time, strain_id) <<std::endl;
	agent.set_death_correction(ThreePartFunction(new_dth, time), strain_id);

	// Other properties
	// Record the time when vaccine effects start dropping (assumes all these properties follow the same trend)
	agent.set_time_vaccine_effects_reduction(time+max_end);
	// and the time when mobility increases (at peak effectiveness)
	agent.set_time_mobility_increase(time);
	// To not keep on vaccinating
	agent.set_needs_next_vaccination(false);
	// Correct the type 
	agent.set_vaccine_type("one_dose", strain_id);
	agent.set_vaccine_subtype("former " + tag, strain_id);

	// For all other strains (except ones that received their target vaccine already)
	for (int i = 1; i<=num_strains; ++i) {
		if ((i != strain_id) && !agent.is_vaccinated_for_strain(i)) {
			// Set the tag for this strain and vac type (make compatible with tag)
			std::string other_tag = tag + " other strain " + std::to_string(i);
			// Set the reduced benefits
			// 1) Effectiveness
			std::vector<std::vector<double>> orig_props = vac_types_properties.at(other_tag).at("effectiveness");
			max_benefit = orig_props.at(orig_props.size()-2).at(1);
			ini_benefit = time < 0.0 ? max_benefit : agent.vaccine_effectiveness(time, i);
			std::vector<std::vector<double>> new_eff{{0.0, ini_benefit}, 
												{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 0.0}};
			agent.set_vaccine_effectiveness(ThreePartFunction(new_eff, time), i);
		
			// 2) Asymptomatic correction
			orig_props = vac_types_properties.at(other_tag).at("asymptomatic");
			max_benefit = orig_props.at(orig_props.size()-2).at(1);
			ini_benefit = time < 0.0 ? max_benefit : agent.asymptomatic_correction(time, i);
			std::vector<std::vector<double>> new_asm{{0.0, ini_benefit}, 
												{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 1.0}};
			agent.set_asymptomatic_correction(ThreePartFunction(new_asm, time), i);
		
			// 3) Transmission correction
			orig_props = vac_types_properties.at(other_tag).at("transmission");
			max_benefit = orig_props.at(orig_props.size()-2).at(1);
			ini_benefit = time < 0.0 ? max_benefit : agent.transmission_correction(time, i);
			std::vector<std::vector<double>> new_tr{{0.0, ini_benefit}, 
												{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 1.0}};
			agent.set_transmission_correction(ThreePartFunction(new_tr, time), i);	
		
			// 4) Severity correction
			orig_props = vac_types_properties.at(other_tag).at("severe");
			max_benefit = orig_props.at(orig_props.size()-2).at(1);
			ini_benefit = time < 0.0 ? max_benefit : agent.severe_correction(time, i);
			std::vector<std::vector<double>> new_sv{{0.0, ini_benefit}, 
												{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 1.0}};
			agent.set_severe_correction(ThreePartFunction(new_sv, time), i);
		
			// 5) Death correction
			orig_props = vac_types_properties.at(other_tag).at("death");
			max_benefit = orig_props.at(orig_props.size()-2).at(1);
			ini_benefit = time < 0.0 ? max_benefit : agent.death_correction(time, i);
			std::vector<std::vector<double>> new_dth{{0.0, ini_benefit}, 
												{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 1.0}};
			agent.set_death_correction(ThreePartFunction(new_dth, time), i);

//	std::cout << "Strain 2 pre: "<< ini_benefit << " " << max_benefit << " " << agent.death_correction(time, i)<< std::endl;

			// Booster type - one dose
			agent.set_vaccine_type("one_dose", i);

//	std::cout << "Strain 2: "<< ini_benefit << " " << max_benefit << " " << agent.death_correction(time, i)<< std::endl;
		}
	}
}
