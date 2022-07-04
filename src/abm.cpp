#include "../include/abm.h"

/***************************************************** 
 * class: ABM
 * 
 * Interface for agent-based modeling 
 *
 * Provides operations for creation, management, and
 * progression of an agent-based model
 *
 * Stores model-related data
 *
 * NOTE: IDs of objects correspond to their positions
 * in the vectors of objects and determine the way
 * they are accessed; IDs start with 1 but are corrected
 * by -1 when accessing;   
 * 
******************************************************/

//
// Initialization and object construction
//

// Set initial values on all the data collection variables and containers
void ABM::initialize_data_collection()
{
	n_infected_tot = 0;
	n_infected_tot_strain = {};
	n_dead_tot = 0;
	n_dead_tested = 0;
	n_dead_not_tested = 0;
	n_recovered_tot = 0;
	n_recovering_exposed = 0;

	tot_tested = 0;
	tot_tested_pos = 0;
	tot_tested_neg = 0;
	tot_tested_false_pos = 0;
	tot_tested_false_neg = 0;

	n_infected_day = {};
	n_dead_day = {};
	n_recovered_day = {};
	tested_day = {};
	tested_pos_day = {};	
	tested_neg_day = {};
	tested_false_pos_day = {};
	tested_false_neg_day = {};
}

// Create the town, agents, infection properties, and introduce initially infected 
void ABM::simulation_setup(const std::string filename, const std::vector<int>& inf0, 
		const bool custom_vac_offsets, const bool custom_boost_offsets)
{
	// Load filenames - key is the tag, value is the actual file name
	LoadParameters ldparam;
	std::map<std::string, std::string> setup_files = ldparam.load_parameter_map<std::string>(filename);
	
	// Load parameters
	// Separately prepare a map for age-dependent parameters
	std::map<std::string, std::string> dfiles = 
		{ {"exposed never symptomatic", setup_files.at("exposed never symptomatic")}, 
		  {"hospitalization", setup_files.at("hospitalization")}, 
		  {"ICU", setup_files.at("ICU")}, 
		  {"mortality", setup_files.at("mortality")} 
		};
	load_infection_parameters(setup_files.at("Simulation parameters"));
	load_age_dependent_distributions(dfiles);
	load_testing(setup_files.at("Testing manager"));

	// So not to require extra parameters (and be backwards compatible)
	if (custom_vac_offsets && !custom_boost_offsets) {
		load_vaccinations(setup_files.at("Vaccination parameters"), 
			setup_files.at("Vaccination tables directory"), custom_vac_offsets, 
			setup_files.at("File with vaccination offsets"));
	} else if (custom_vac_offsets && custom_boost_offsets) {
		load_vaccinations(setup_files.at("Vaccination parameters"), 
			setup_files.at("Vaccination tables directory"), custom_vac_offsets, 
			setup_files.at("File with vaccination offsets"), custom_boost_offsets, 
			setup_files.at("File with booster offsets")); 
	} else {
		load_vaccinations(setup_files.at("Vaccination parameters"), setup_files.at("Vaccination tables directory"));
	}
	// Setup the town and mobility components
	create_households(setup_files.at("Household data"));
	create_schools(setup_files.at("School data"));
	create_workplaces(setup_files.at("Workplace data"));
	create_hospitals(setup_files.at("Hospital data"));
	create_retirement_homes(setup_files.at("Retirement home data"));
	create_carpools(setup_files.at("Carpool data"));
	create_public_transit(setup_files.at("Public transit data"));
	create_leisure_locations(setup_files.at("Leisure location data"));
	initialize_mobility();

	// Initialize strain number and strain tracking
	n_strains = static_cast<int>(infection_parameters.at("number of strains"));
	n_infected_tot_strain.resize(n_strains);
	// Initialize strain corrections
	strain_correction.resize(n_strains, 1.0);

	// Create the agents, including initially infected
	create_agents(setup_files.at("Agent data"), inf0);
}

// Load infection parameters, store in a map
void ABM::load_infection_parameters(const std::string infile)
{
	// Load parameters
	LoadParameters ldparam;
	infection_parameters = ldparam.load_parameter_map<double>(infile);

	// Set infection distributions
	infection.set_latency_distribution(infection_parameters.at("latency log-normal mean"),
					infection_parameters.at("latency log-normal standard deviation"));	
	infection.set_inf_variability_distribution(infection_parameters.at("agent variability gamma shape"),
					infection_parameters.at("agent variability gamma scale"));
	infection.set_onset_to_death_distribution(infection_parameters.at("otd logn mean"), 
					infection_parameters.at("otd logn std"));
	infection.set_onset_to_hospitalization_distribution(infection_parameters.at("oth gamma shape"), infection_parameters.at("oth gamma scale"));
	infection.set_hospitalization_to_death_distribution(infection_parameters.at("htd wbl shape"), infection_parameters.at("htd wbl scale"));

	// Set single-number probabilities
	infection.set_other_probabilities(infection_parameters.at("average fraction to get tested"),
									  infection_parameters.at("probability of death in ICU"), 
								  infection_parameters.at("probability dying if needing but not admitted to icu"));
}

// Load age-dependent distributions, store in a map of maps
void ABM::load_age_dependent_distributions(const std::map<std::string, std::string> dist_files)
{
	// dist_files entries are property tag : filename with that property
	// This part loads each file content and stores it in a map of maps
	// property tag : [age or age interval as a string : value for that interval] 
	LoadParameters ldparam;
	std::map<std::string, double> one_file;
	for (const auto& dfile : dist_files){
		one_file = ldparam.load_age_dependent(dfile.second);
		for (const auto& entry : one_file){
			age_dependent_distributions[dfile.first][entry.first] = entry.second;
		}
		one_file.clear();
	}

	// Send to Infection class for further processing 
	infection.set_expN2sy_fractions(age_dependent_distributions.at("exposed never symptomatic"));
	infection.set_mortality_rates(age_dependent_distributions.at("mortality"));
	infection.set_hospitalized_fractions(age_dependent_distributions.at("hospitalization"));
	infection.set_hospitalized_ICU_fractions(age_dependent_distributions.at("ICU"));
}

// Initialize testing and its time dependence
void ABM::load_testing(const std::string fname) 
{
	// Regular properties
	testing.initialize_testing(infection_parameters.at("start testing"),
					infection_parameters.at("negative tests fraction"),
					infection_parameters.at("fraction false negative"),
					infection_parameters.at("fraction false positive"),
					infection_parameters.at("fraction to get tested"),
					infection_parameters.at("exposed fraction to get tested"));	
	// Time-dependent test fractions
	std::vector<std::vector<std::string>> file = read_object(fname);
	std::vector<std::vector<double>> fractions_times = {};
	std::vector<double> temp(3,0.0);
	for (auto& entry : file){
		for (int i=0; i<3; ++i){
			temp.at(i) = std::stod(entry.at(i));
		}
		fractions_times.push_back(temp);
	}
	testing.set_time_varying(fractions_times);
}

// Initialize Vaccinations class 
void ABM::load_vaccinations(const std::string& fname, const std::string& data_path, 
								const bool use_custom, const std::string& offset_file,
								const bool use_boost_custom, const std::string& offset_file_boosters) 
{
	if (use_custom && !use_boost_custom) {
		vaccinations = Vaccinations(fname, data_path, offset_file, infection);
	} else if (use_custom && use_boost_custom) {
		vaccinations = Vaccinations(fname, data_path, offset_file, infection, offset_file_boosters);
	} else {
		vaccinations = Vaccinations(fname, data_path);
	}
}

// Generate and store household objects
void ABM::create_households(const std::string fname)
{
	// Read the whole file
	std::vector<std::vector<std::string>> file = read_object(fname);
	// One household per line
	for (auto& house : file){
		// Extract properties, add infection parameters
		Household temp_house(std::stoi(house.at(0)), 
			std::stod(house.at(1)), std::stod(house.at(2)),
			infection_parameters.at("household scaling parameter"),
			infection_parameters.at("severity correction"),
			static_cast<int>(infection_parameters.at("number of strains")));
		// Store 
		households.push_back(temp_house);
	}
}

// Generate and store retirement homes objects
void ABM::create_retirement_homes(const std::string fname)
{
	// Read the whole file
	std::vector<std::vector<std::string>> file = read_object(fname);
	// One household per line
	for (auto& rh : file){
		// Extract properties, add infection parameters
		RetirementHome temp_RH(std::stoi(rh.at(0)), 
			std::stod(rh.at(1)), std::stod(rh.at(2)),
			infection_parameters.at("severity correction"),
			infection_parameters.at("RH employee absenteeism factor"),
			static_cast<int>(infection_parameters.at("number of strains")));
		// Store 
		retirement_homes.push_back(temp_RH);
	}
}

// Generate and store school objects
void ABM::create_schools(const std::string fname)
{
	// Read the whole file
	std::vector<std::vector<std::string>> file = read_object(fname);
	// One workplace per line
	for (auto& school : file){
		// Extract properties, add infection parameters
		// School-type dependent absenteeism
		double psi = 0.0;
		std::string school_type = school.at(3);
		if (school_type == "daycare")
 			psi = infection_parameters.at("daycare absenteeism correction");
		else if (school_type == "primary" || school_type == "middle")
 			psi = infection_parameters.at("primary and middle school absenteeism correction");
		else if (school_type == "high")
 			psi = infection_parameters.at("high school absenteeism correction");
		else if (school_type == "college")
 			psi = infection_parameters.at("college absenteeism correction");
		else
			throw std::invalid_argument("Wrong school type: " + school_type);
		School temp_school(std::stoi(school.at(0)), 
			std::stod(school.at(1)), std::stod(school.at(2)),
			infection_parameters.at("severity correction"),	
			infection_parameters.at("school employee absenteeism correction"),
			psi, static_cast<int>(infection_parameters.at("number of strains")));
		// Store 
		schools.push_back(temp_school);
	}
}

// Generate and store workplace objects
void ABM::create_workplaces(const std::string fname)
{
	// Read the whole file
	std::vector<std::vector<std::string>> file = read_object(fname);
	// One workplace per line
	for (auto& work : file){
		// Extract properties, add infection parameters
		Workplace temp_work(std::stoi(work.at(0)), 
			std::stod(work.at(1)), std::stod(work.at(2)),
			infection_parameters.at("severity correction"),
			infection_parameters.at("work absenteeism correction"),
			work.at(3), static_cast<int>(infection_parameters.at("number of strains")));
		// Store 
		workplaces.push_back(temp_work);
	}
	set_outside_workplace_transmission();
}

// Create hospitals based on information in a file
void ABM::create_hospitals(const std::string fname)
{
	// Read the whole file
	std::vector<std::vector<std::string>> file = read_object(fname);
	// One hospital per line
	for (auto& hospital : file){
		Hospital temp_hospital(std::stoi(hospital.at(0)), 
			std::stod(hospital.at(1)), std::stod(hospital.at(2)),
			infection_parameters.at("severity correction"), static_cast<int>(infection_parameters.at("number of strains")));
		// Store 
		hospitals.push_back(temp_hospital);
	}
}

// Generate and store carpool objects
void ABM::create_carpools(const std::string fname)
{
	// Read the whole file
	std::vector<std::vector<std::string>> file = read_object(fname);
	// One carpool per line
	for (auto& cpl : file) {
		// Extract properties, add infection parameters
		Transit temp_transit(std::stoi(cpl.at(0)), 
			infection_parameters.at("severity correction"),  
			infection_parameters.at("work absenteeism correction"),
			cpl.at(1), static_cast<int>(infection_parameters.at("number of strains")));
		// Store 
		carpools.push_back(temp_transit);
	}
}

// Generate and store public transit objects
void ABM::create_public_transit(const std::string fname)
{
	// Read the whole file
	std::vector<std::vector<std::string>> file = read_object(fname);
	
	// One public transit per line
	for (auto& pbt : file) {
		// Extract properties, add infection parameters
		Transit temp_transit(std::stoi(pbt.at(0)),
			infection_parameters.at("severity correction"),  
			infection_parameters.at("work absenteeism correction"),  
			pbt.at(1), static_cast<int>(infection_parameters.at("number of strains")));
		// Store 
		public_transit.push_back(temp_transit);
	}
}

// Generate and store leisure locations/weekend objects
void ABM::create_leisure_locations(const std::string fname)
{
	// Read the whole file
	std::vector<std::vector<std::string>> file = read_object(fname);
	// One leisure location per line
	for (auto& lsr : file) {
		// Extract properties, add infection parameters
		Leisure temp_lsr(std::stoi(lsr.at(0)), 
			std::stod(lsr.at(1)), std::stod(lsr.at(2)),
			infection_parameters.at("severity correction"), 
			lsr.at(3), static_cast<int>(infection_parameters.at("number of strains")));
		// Store 
		leisure_locations.push_back(temp_lsr);
	}
	set_outside_leisure_transmission();
}

// Initialize Mobility and assignment of leisure locations
void ABM::initialize_mobility()
{
	mobility.set_probability_parameters(infection_parameters.at("leisure - dr0"), infection_parameters.at("leisure - beta"), infection_parameters.at("leisure - kappa"));
	mobility.construct_public_probabilities(households, leisure_locations);
}

// Create agents and assign them to appropriate places
void ABM::create_agents(const std::string fname, const std::vector<int>& ninf0)
{
	load_agents(fname, ninf0);
	register_agents();
	initialize_contact_tracing();
}

// Retrieve agent information from a file
void ABM::load_agents(const std::string fname, const std::vector<int>& ninf0)
{
	// Read the whole file
	std::vector<std::vector<std::string>> file = read_object(fname);

	// One of the many flu setups	
	setup_flu();

	// IDs of initially infected agents for each strain
	// Outer vector - strains, inner vectors - IDs for that strain
	int n_agents = file.size();
	std::vector<std::vector<int>> infected_IDs = select_initially_infected(n_agents, ninf0);
	int strain_id = 0;

	// Counter for agent IDs
	int agent_ID = 1;
	// One agent per line, with properties as defined in the line
	for (auto agent : file){
		// Agent status
		bool student = false, works = false, livesRH = false, worksRH = false,  
			 worksSch = false, patient = false, hospital_staff = false,
			 works_from_home = false;
		int house_ID = -1, workID = 0, cpID = 0, ptID = 0;
		double work_travel_time = 0.0;
		std::string work_travel_mode;
		// Entries will be common for all agents, values may change
		std::vector<std::map<std::string, double>> transmission_rates = generate_initial_tr_rates(n_strains);

		// Infection status 
		bool infected = false;
		for (int i = 0; i < infected_IDs.size(); ++i) {
			std::vector<int>& one_strain_IDs = infected_IDs.at(i);
			if (one_strain_IDs.empty() || one_strain_IDs.at(0) == -1) {
				continue;
			}
			auto iter = std::find(one_strain_IDs.begin(), one_strain_IDs.end(), agent_ID); 
			if (iter != one_strain_IDs.end()) {
				one_strain_IDs.erase(iter);
				infected = true;
				strain_id = i + 1;
				++n_infected_tot;
				++n_infected_tot_strain.at(strain_id - 1);
				break;
			}
		}
		
		// Properties
		assign_roles(agent, house_ID, patient, hospital_staff, student, 
						works, livesRH, worksRH, worksSch, workID);
		assign_transit(agent, transmission_rates, works_from_home, work_travel_time, 
					work_travel_mode, cpID, ptID, works, hospital_staff);
		assign_workplace_transmission_rate(agent, transmission_rates);
		
		// Construction
		Agent temp_agent(student, works, std::stoi(agent.at(2)), 
			std::stod(agent.at(3)), std::stod(agent.at(4)), house_ID,
			patient, std::stoi(agent.at(7)), livesRH, worksRH,
		    worksSch, workID, hospital_staff, std::stoi(agent.at(13)), 
			infected, work_travel_mode, work_travel_time, cpID, ptID, 
			works_from_home, transmission_rates, n_strains);

		// Post-processing
		temp_agent.set_ID(agent_ID++);
		temp_agent.set_occupation(agent.at(21));
		temp_agent.set_occupation_transmission();

		// Set properties for exposed if initially infected
		if (temp_agent.infected() == true) {
			temp_agent.set_strain(strain_id);
			initial_exposed(temp_agent);
		}	

		// Store
		agents.push_back(temp_agent);
	}
}

// Setup flu properties
void ABM::setup_flu()
{
	// Flu settings
	// Set fraction of flu (non-covid symptomatic)
	flu.set_fraction(infection_parameters.at("fraction with flu"));
	flu.set_fraction_tested_false_positive(infection_parameters.at("fraction false positive"));
	// Time interval for testing
	flu.set_testing_duration(infection_parameters.at("flu testing duration"));
}

// Select IDs of agents initially infected with each strain
std::vector<std::vector<int>> ABM::select_initially_infected(const int n_agents, const std::vector<int>& ninf0)
{
	// For custom generation of initially infected
	std::vector<std::vector<int>> infected_IDs;
	// All possible IDs
	std::vector<int> agent_ids(n_agents);
	std::iota(agent_ids.begin(), agent_ids.end(), 1);
	// One agent can be infected with only one strain - this ensures it
	for (const auto& strain_i0 : ninf0) {
		// Safety and redundancy check
		if (strain_i0 == 0) {
			infected_IDs.push_back({-1});
			continue;
		}
		if (strain_i0 > agent_ids.size()) {
			throw std::runtime_error("Requesting more initially infected than available agents");
		}
		// Randomly rearrange the remaining IDs
		std::vector<int> temp_ids(strain_i0);
		infection.vector_shuffle(agent_ids);
		// Copy strain_i0 of them into the ID vector for that strain, save it
		auto it_end = std::next(agent_ids.begin(), strain_i0); 
		std::copy(agent_ids.begin(), it_end, temp_ids.begin());		
		infected_IDs.push_back(temp_ids);
		// Remove the selected IDs
		for (const auto& id : temp_ids) {
			auto iter = std::find(agent_ids.begin(), agent_ids.end(), id);
			agent_ids.erase(iter);
		}
	}
	return infected_IDs;
}

// Initialize all transmission rates, assign nominal (common) values
std::vector<std::map<std::string, double>> ABM::generate_initial_tr_rates(const int& n_strains)
{
	std::vector<std::map<std::string, double>> nominal_rates;
	for (int i=1; i<=n_strains; ++i) {
		std::map<std::string, double> temp =  
		{
			{"household transmission rate", infection_parameters.at(std::string("household transmission rate") + std::string(" strain ") + std::to_string(i))},
			{"workplace transmission rate", infection_parameters.at(std::string("workplace transmission rate") + std::string(" strain ") + std::to_string(i))},
			{"carpool transmission rate", infection_parameters.at(std::string("carpool transmission rate") + std::string(" strain ") + std::to_string(i))}, 
			{"public transit transmission rate", infection_parameters.at("public transit transmission rate")},
			{"leisure locations transmission rate", infection_parameters.at(std::string("leisure locations transmission rate") + std::string(" strain ") + std::to_string(i))},
			{"RH employee transmission rate", infection_parameters.at(std::string("RH employee transmission rate") + std::string(" strain ") + std::to_string(i))},
			{"RH resident transmission rate", infection_parameters.at(std::string("RH resident transmission rate") + std::string(" strain ") + std::to_string(i))},
			{"RH transmission rate of home isolated", infection_parameters.at(std::string("RH transmission rate of home isolated") + std::string(" strain ") + std::to_string(i))},
			{"school transmission rate", infection_parameters.at(std::string("school transmission rate") + std::string(" strain ") + std::to_string(i))},
			{"school employee transmission rate", infection_parameters.at(std::string("school employee transmission rate") + std::string(" strain ") + std::to_string(i))},
			{"healthcare employees transmission rate", infection_parameters.at(std::string("healthcare employees transmission rate") + std::string(" strain ") + std::to_string(i))},
			{"hospital patients transmission rate", infection_parameters.at(std::string("hospital patients transmission rate") + std::string(" strain ") + std::to_string(i))},
			{"hospitalized transmission rate", infection_parameters.at(std::string("hospitalized transmission rate") + std::string(" strain ") + std::to_string(i))},
			{"hospitalized ICU transmission rate", infection_parameters.at(std::string("hospitalized ICU transmission rate") + std::string(" strain ") + std::to_string(i))},
			{"hospital tested transmission rate", infection_parameters.at(std::string("hospital tested transmission rate") + std::string(" strain ") + std::to_string(i))},
			{"transmission rate of home isolated", infection_parameters.at(std::string("transmission rate of home isolated") + std::string(" strain ") + std::to_string(i))}		
		};
		nominal_rates.push_back(temp);
	}
	return nominal_rates;
}

// Assign proper transmission rate for an out-of-town or a generic workplace
void ABM::assign_workplace_transmission_rate(const std::vector<std::string>& agent,
					std::vector<std::map<std::string, double>>& transmission_rates)
{
	// Set agent occupation
	std::string work_type = agent.at(21);
	std::string rate_by_type;
	// And the corresponding transmission rate
	double work_rate = 0.0;	 
	if (work_type != "none") {
		if (work_type == "A") { 
			rate_by_type = "management science art transmission rate";
		} else if (work_type == "B") { 
			rate_by_type = "service occupation transmission rate";
		} else if (work_type == "C") { 
			rate_by_type = "sales office transmission rate"; 
		} else if (work_type == "D") { 
			rate_by_type = "construction maintenance transmission rate"; 
		} else if (work_type == "E") { 
			rate_by_type = "production transportation transmission rate"; 
		}
 		for (int ip = 1; ip <= n_strains; ++ip) {
			transmission_rates.at(ip-1).at("workplace transmission rate") 
					= infection_parameters.at(std::string(rate_by_type) + std::string(" strain ") + std::to_string(ip));
		}
	}
}

// Select transit and calculate transmission rates if necessary
void ABM::assign_transit(const std::vector<std::string>& agent,
					std::vector<std::map<std::string, double>>& transmission_rates,
					bool& works_from_home, double& work_travel_time, 
					std::string& work_travel_mode, int& cpID, int& ptID,
					const bool works, const bool hospital_staff)
{
	// Transit information
	if (std::stoi(agent.at(15)) == 1) {
		works_from_home = true;
		work_travel_mode = agent.at(17);
	} else {
		if (!(works || hospital_staff)) {
			work_travel_mode = "None";
		} else {
			work_travel_mode = agent.at(17);
			if (work_travel_mode == "carpool") {
				cpID = std::stoi(agent.at(19));
			}
			if (work_travel_mode == "public") {
				ptID = std::stoi(agent.at(20));
				// Transmission rate based on current capacity
				for (int ip = 1; ip <= n_strains; ++ip) {
					double beta_T = infection_parameters.at(std::string("public transit beta0") + std::string(" strain ") + std::to_string(ip)) 
						+ infection_parameters.at(std::string("public transit beta full") + std::string(" strain ") + std::to_string(ip))
						*infection_parameters.at("public transit current capacity");
					transmission_rates.at(ip-1).at("public transit transmission rate") = beta_T;
				}
			}
			work_travel_time = std::stod(agent.at(16));
		}
	}
}

// Properties of agents: housing, work and school status
void ABM::assign_roles(const std::vector<std::string>& agent, int& house_ID,
						bool& patient, bool& hospital_staff, bool& student, 
						bool& works, bool& livesRH, bool& worksRH, bool& worksSch, 
						int& workID)
{
		// Household ID only if not hospitalized with condition
		// different than COVID-19
		if (std::stoi(agent.at(6)) == 1){
			patient = true;
			house_ID = 0;
		}else{
			house_ID = std::stoi(agent.at(5));
		}

		// No school or work if patient with condition other than COVID
		if (std::stoi(agent.at(12)) == 1 && !patient){
			hospital_staff = true;
		}
		if (std::stoi(agent.at(0)) == 1 && !patient){
			student = true;
		}
	   	// No work flag if a hospital employee	
		if (std::stoi(agent.at(1)) == 1 && !(patient || hospital_staff)){
			works = true; 
		}
			// Retirement home resident
		if (std::stoi(agent.at(8)) == 1){
			 livesRH = true;
		}
		// Retirement home or school employee
		if (std::stoi(agent.at(9)) == 1){
			 worksRH = true;
		}
		if (std::stoi(agent.at(10)) == 1){
			 worksSch = true;
		}

		// Select correct work ID for special employment types
		// Hospital ID is set separately, but for consistency
		if (worksRH || worksSch || hospital_staff) {
			workID = std::stoi(agent.at(18));
		} else if (works) {
			workID = std::stoi(agent.at(11));
		}


}

// Assign agents to households, schools, and worplaces
void ABM::register_agents()
{
	int house_ID = 0, school_ID = 0, work_ID = 0, hospital_ID = 0;
	int agent_ID = 0, tr_ID = 0;
	bool infected = false;

	for (const auto& agent : agents){
		
		// Agent ID and infection status
		agent_ID = agent.get_ID();
		infected = agent.infected();

		// If not a non-COVID hospital patient, 
		// register in the household or a retirement home
		if (agent.hospital_non_covid_patient() == false){
			if (agent.retirement_home_resident()){
				house_ID = agent.get_household_ID();
				RetirementHome& rh = retirement_homes.at(house_ID - 1); 
				rh.register_agent(agent_ID);
			} else {
				house_ID = agent.get_household_ID();
				Household& house = households.at(house_ID - 1); 
				house.register_agent(agent_ID);
			}
		}

		// Register in schools, workplaces, and hospitals 
		if (agent.student()){
			school_ID = agent.get_school_ID();
			School& school = schools.at(school_ID - 1); 
			school.register_agent(agent_ID);		
		}

		if (agent.works() && !agent.works_from_home()
				&& !agent.hospital_employee()){
			work_ID = agent.get_work_ID();
			if (agent.retirement_home_employee()){
				RetirementHome& rh = retirement_homes.at(work_ID - 1);
				rh.register_agent(agent_ID);
			} else if (agent.school_employee()){
				School& school = schools.at(work_ID - 1); 
				school.register_agent(agent_ID);
			} else {
				Workplace& work = workplaces.at(work_ID - 1);
				work.register_agent(agent_ID);
			}		
		}

		if (agent.hospital_employee() || 
				agent.hospital_non_covid_patient()){
			hospital_ID = agent.get_hospital_ID();
			Hospital& hospital = hospitals.at(hospital_ID - 1);
			hospital.register_agent(agent_ID);	
		}

		// Register transit if carpool or public
		if (agent.get_work_travel_mode() == "carpool") {
			tr_ID = agent.get_carpool_ID();
			Transit& carpool = carpools.at(tr_ID-1);
			carpool.register_agent(agent_ID);	
		}
		if (agent.get_work_travel_mode() == "public") {
			tr_ID = agent.get_public_transit_ID();
			Transit& public_tr = public_transit.at(tr_ID-1);
			public_tr.register_agent(agent_ID);	
		}
	} 
}

// Initial set-up of exposed agents
void ABM::initial_exposed(Agent& agent)
{
	int strain_id = agent.get_strain();
	bool never_sy = infection.recovering_exposed(agent.get_age(), agent.asymptomatic_correction(time, strain_id));
	// Total latency period
	double latency = infection.latency();
	// Portion of latency when the agent is not infectious
	double dt_ninf = std::min(infection_parameters.at("time from exposed to infectiousness"), latency);
	if (never_sy){
		// Set to total latency + infectiousness duration
		double rec_time = infection_parameters.at("recovery time");
		agent.set_latency_duration(latency + rec_time);
		agent.set_latency_end_time(time);
		agent.set_infectiousness_start_time(time, dt_ninf);
	}else{
		// If latency shorter, then  not infectious during the entire latency
		agent.set_latency_duration(latency);
		agent.set_latency_end_time(time);
		agent.set_infectiousness_start_time(time, dt_ninf);
	}
	agent.set_inf_variability_factor(infection.inf_variability()*agent.transmission_correction(time, strain_id));
	agent.set_exposed(true);
	agent.set_recovering_exposed(never_sy);
}

// Set up contact tracing functionality 
void ABM::initialize_contact_tracing()
{
	contact_tracing = Contact_tracing(agents.size(), households.size(), 
						infection_parameters.at("maximum number of visits to track"));
}

// Initialization for vaccination vs. reopening studies
void ABM::initialize_simulations(const bool dont_vac)
{
	// Flu and initial vaccination
	n_vaccinated = static_cast<int>(infection_parameters.at("initially vaccinated"));
	random_vaccines = true;
	// To invoke flu, testing, and vaccinations
	infection_parameters.at("start testing") = 0.0;
	start_testing_flu_and_vaccination(dont_vac);
}

// Start with N_inf agents that have COVID-19 in various stages and N_R recovered from strain 1
void ABM::initialize_active_cases(const std::vector<int> N_inf, const bool vaccinate, 
							const std::vector<int> N_vac, const std::vector<int> N_bst, const int N_R)
{
	// Vaccination of agents with randomly perturbed 
	// vaccination times 
	if (vaccinate) {
		// Adjust n_vaccinated and boosted
		n_vaccinated = N_vac.at(0);
		n_boosted = N_bst.at(0);
		// Apply at random to eligible agents
		vaccinate_random_time_offset();
	}

	// Initially recovered from strain 1
	int ini_strain = 1;
	std::vector<int> can_be_removed;
	// Select qualifying agents
	for (auto& agent : agents) {
		if (!agent.symptomatic_non_covid() && !agent.infected()) {
			can_be_removed.push_back(agent.get_ID());
		}
	}
	infection.vector_shuffle(can_be_removed);
	if (N_R > can_be_removed.size()) {
		std::cerr << "Requested number of agents to initially be recovered from strain 1 "
				  << "larger than number of available agents" << std::endl;
		throw std::invalid_argument("Too many agents to be initially recovered: " + N_R);
	}
	for (int ir = 0; ir < N_R; ++ir) {
		int aID = can_be_removed.at(ir);
		Agent& agent = agents.at(aID-1);
		agent.set_removed_recovered(true, ini_strain);
		// Just one - recovery from both that overlaps will just be extended (the immunity
		// will last longer if they later recover from strain 2)
		agent.set_time_recovered_to_susceptible(time +
				infection_parameters.at("Post-infection immunity duration")*
				infection.get_uniform());
	}

	// Increase total infected count
	// This will be based on strain 1 + need to add the removed for strain 1
	// And account for infected with strain 2 at t = 0 (done because infected() is
	// still a general call that holds for both)
	int tot_requested = N_inf.at(0);
	n_infected_tot += tot_requested;
	std::vector<int> can_have_covid;
	// Select qualifying agents
	for (auto& agent : agents) {
		if (!agent.symptomatic_non_covid() && !agent.infected()
				&& !agent.exposed() && !agent.symptomatic() 
				&& !agent.removed_recovered(ini_strain)) {
			can_have_covid.push_back(agent.get_ID());
		}
	}

	// Randomly rearrange, then select first N_inf if available 
	infection.vector_shuffle(can_have_covid);
	if (tot_requested > can_have_covid.size()) {
		std::cerr << "Requested number of agents to initially have covid "
				  << "larger than number of available agents" << std::endl;
		throw std::invalid_argument("Too many agents to have covid: " + tot_requested);
	}
	int i_tot = 0;
	int N0_i = 0;
	for (int j = 0; j < n_strains; ++j) {
		N0_i = N_inf.at(j);
		n_infected_tot_strain.at(j) += N0_i;
		for (int i = 0; i < N0_i; ++i) {
			Agent& agent = agents.at(can_have_covid.at(i_tot)-1);
			// Currently aymptomatic or altogether asymptomatic
			agent.set_strain(j+1);
			bool never_sy = infection.recovering_exposed(agent.get_age(),
					agent.asymptomatic_correction(time, j+1));
			if (never_sy) {
				process_initial_asymptomatic(agent);
			} else {
				process_initial_symptomatic(agent);
			}
			++i_tot;
		}
	}
}

// Initialize an asymptomatic agent, randomly in the course of disease
void ABM::process_initial_asymptomatic(Agent& agent)
{
	// Temporary transition objects
	RegularTransitions regular_transitions;
	HspEmployeeTransitions hsp_employee_transitions;
    HspPatientTransitions hsp_patient_transitions;

	// Flags
	agent.set_infected(true);
	agent.set_exposed(true);
	agent.set_recovering_exposed(true);

	// Common properties
	// Total latency period offset with a random number from 0 to 1
	double latency = infection.latency()*infection.get_uniform();
	// Portion of latency when the agent is not infectious
	double dt_ninf = std::min(infection_parameters.at("time from exposed to infectiousness"), latency);
	// Set to total latency + infectiousness duration, also offset
	double rec_time = infection_parameters.at("recovery time")*infection.get_uniform();
	agent.set_latency_duration(latency + rec_time);
	agent.set_latency_end_time(time);
	agent.set_infectiousness_start_time(time, dt_ninf);
	// Agent characteristics
	agent.set_inf_variability_factor(infection.inf_variability()*agent.transmission_correction(time, agent.get_strain()));
	// Remove from potential flu population if a regular agent
	if (!agent.hospital_employee() && !agent.hospital_non_covid_patient()) {
		flu.remove_susceptible_agent(agent.get_ID());
	}

	// Testing status
	if (testing.started(time)) {
		if (agent.hospital_employee()) {
			hsp_employee_transitions.set_testing_status(agent, infection, time, schools, 
							hospitals, infection_parameters, testing);
		} else if (agent.hospital_non_covid_patient()) {
			hsp_patient_transitions.set_testing_status(agent, infection, time, hospitals, infection_parameters, testing);
		} else {
    		regular_transitions.set_testing_status(agent, infection, time, schools,
        		workplaces, hospitals, retirement_homes, carpools, public_transit, infection_parameters, testing);
		}

		// If tested, randomly choose if pre-test, being tested now, or waiting for results
		std::vector<std::string> testing_stages = {"waiting for test", "getting tested", "waiting for results"};
		if (agent.tested()) {
			std::string cur_stage = testing_stages.at(infection.get_int(0, testing_stages.size()-1));
			double test_time_lag = 0.0;
			// Adjust properties accordingly
			if (cur_stage == "waiting for test") {
				// Just perturb the time to wait
				test_time_lag = infection.get_uniform()*std::max(0.0, agent.get_time_of_test() - time);
				agent.set_time_to_test(test_time_lag);
				agent.set_time_of_test(time);
			} else if (cur_stage == "getting tested") {
				// Adjust the time, transitions will happen on their own 
				agent.set_time_to_test(0.0);
				agent.set_time_of_test(time);
			} else {
				// Reset time to wait for the test 
				test_time_lag = infection.get_uniform()*std::max(0.0, agent.get_time_of_test() - time);
				agent.set_time_to_test(-1.0*test_time_lag);
				agent.set_time_of_test(time);
				// Perturb the time to wait for results
				test_time_lag = infection.get_uniform()*std::max(0.0, agent.get_time_of_results() - time);
				agent.set_time_until_results(test_time_lag);
				agent.set_time_of_results(time);
				// Flags
				agent.set_tested_awaiting_test(false);
				agent.set_tested_awaiting_results(true);
			}		
		}
     }
}

// Initialize a symptomatic agent, randomly in the course of disease
void ABM::process_initial_symptomatic(Agent& agent)
{
	// Temporary transition objects
	RegularTransitions regular_transitions;
	HspEmployeeTransitions hsp_employee_transitions;
    HspPatientTransitions hsp_patient_transitions;
	// Flags
 	agent.set_infected(true);
	agent.set_symptomatic(true);
	// Agent characteristics
	agent.set_inf_variability_factor(infection.inf_variability()*agent.transmission_correction(time, agent.get_strain()));
	// Remove from potential flu population if a regular agent
	if (!agent.hospital_employee() && !agent.hospital_non_covid_patient()) {
		flu.remove_susceptible_agent(agent.get_ID());
	}

	// Testing status
	if (agent.hospital_employee()) {
		// Hospital employee will go under IH and test for sure
		hsp_employee_transitions.remove_from_hospitals_and_schools(agent, schools, hospitals, carpools,  public_transit);
		// Removal settings		
		int agent_age = agent.get_age();
		bool is_hsp = true;
		if (infection.will_die_non_icu(agent_age, 									
									agent.asymptomatic_correction(time, agent.get_strain()),
									agent.severe_correction(time, agent.get_strain()),
									agent.death_correction(time, agent.get_strain()), is_hsp)){
			agent.set_dying(true);
			agent.set_recovering(false);			
			agent.set_time_to_death(infection.time_to_death());
			agent.set_death_time(time);
		} else {
			agent.set_dying(false);
			agent.set_recovering(true);			
			agent.set_recovery_duration(infection_parameters.at("recovery time")*infection.get_uniform());
			agent.set_recovery_time(time);		
		}
		if (testing.started(time)) {
			hsp_employee_transitions.set_testing_status(agent, infection, time, schools,
                        hospitals, infection_parameters, testing);
		}
	} else if (agent.hospital_non_covid_patient()) {
		// Removal settings
		bool is_hsp = true; 		
		int agent_age = agent.get_age();
		if (infection.will_die_non_icu(agent_age,
									agent.asymptomatic_correction(time, agent.get_strain()),
									agent.severe_correction(time, agent.get_strain()),
									agent.death_correction(time, agent.get_strain()), is_hsp)){
			states_manager.set_dying_symptomatic(agent);			
			agent.set_time_to_death(infection.time_to_death());
			agent.set_death_time(time);
		} else {
			states_manager.set_recovering_symptomatic(agent);			
			// This may change if treatment is ICU
			agent.set_recovery_duration(infection_parameters.at("recovery time"));
			agent.set_recovery_time(time);		
		}
		if (testing.started(time)) {
			hsp_patient_transitions.set_testing_status(agent, infection, time, hospitals, infection_parameters, testing);
		}
	} else {
    	regular_transitions.untested_sy_setup(agent, infection, time, dt, households, 
                schools, workplaces, hospitals, retirement_homes,
                carpools, public_transit, infection_parameters, testing);
	}

	// If tested, randomly choose if pre-test, being tested now, or waiting for results
	std::vector<std::string> testing_stages = {"waiting for test", "getting tested", "waiting for results",
												"getting treated"};
	if (agent.tested()) {
		std::string cur_stage = testing_stages.at(infection.get_int(0, testing_stages.size()-1));
		double test_time_lag = 0.0;
		// Adjust properties accordingly
		if (cur_stage == "waiting for test") {
			// Just perturb the time to wait
			test_time_lag = infection.get_uniform()*std::max(0.0, agent.get_time_of_test() - time);
			agent.set_time_to_test(test_time_lag);
			agent.set_time_of_test(time);
		} else if (cur_stage == "getting tested") {
			// Adjust the time, transitions will happen on their own 
			agent.set_time_to_test(0.0);
			agent.set_time_of_test(time);
		} else if (cur_stage == "waiting for results") {
			// Reset time to wait for the test 
			test_time_lag = infection.get_uniform()*std::max(0.0, agent.get_time_of_test() - time);
			agent.set_time_to_test(-1.0*test_time_lag);
			agent.set_time_of_test(time);
			// Perturb the time to wait for results
			test_time_lag = infection.get_uniform()*std::max(0.0, agent.get_time_of_results() - time);
			agent.set_time_until_results(test_time_lag);
			agent.set_time_of_results(time);
			// Flags
			agent.set_tested_awaiting_test(false);
			agent.set_tested_awaiting_results(true);
		} else {
			// Got results (treatment or false negative)
			// Reset time to wait for the test 
			test_time_lag = infection.get_uniform()*std::max(0.0, agent.get_time_of_test() - time);
			agent.set_time_to_test(-1.0*test_time_lag);
			agent.set_time_of_test(time);
			// Set time for results to now
			agent.set_time_until_results(0);
			agent.set_time_of_results(time);
			// Flags
			agent.set_tested(true);
			agent.set_tested_awaiting_test(false);
			agent.set_tested_awaiting_results(true);
		} 		
	}
}

// Vaccinate random members of the population that are not Flu or infected agents
void ABM::vaccinate_random()
{
	int cur_vaccinated = 0;
	// Check against allowable maximum (hesitancy, inability to vaccinate)
	if (total_vaccinated >= infection_parameters.at("Maximum number to vaccinate")) {
		return; 
	} else if (total_vaccinated + n_vaccinated >= 
					infection_parameters.at("Maximum number to vaccinate")) {
		n_vaccinated = infection_parameters.at("Maximum number to vaccinate") - total_vaccinated;
//		std::cout << "Requested number of agents to vaccinate exceeds" 
//				  << " the maximum allowable count - reducing to " << n_vaccinated << std::endl; 
	}
	if (total_boosted + n_boosted >= 
					infection_parameters.at("Maximum number to boost")) {
		n_boosted = infection_parameters.at("Maximum number to boost") - total_boosted;
//		std::cout << "Requested number of agents to boost exceeds" 
//				  << " the maximum allowable count - reducing to " << n_boosted << std::endl; 
	}
	// Vaccinate if possible, update the counter
	std::vector<int> cur = vaccinations.vaccinate_random(agents, n_vaccinated, n_boosted, infection, time);	
	total_vaccinated += cur.at(0);
	total_boosted += cur.at(1);
}

// Vaccinate random members of the population with a variable time offset  
void ABM::vaccinate_random_time_offset()
{
	int cur_vaccinated = 0;
	// Check against allowable maximum (hesitancy, inability to vaccinate)
	if ((total_vaccinated > infection_parameters.at("Maximum number to vaccinate")) || 
		(total_boosted > infection_parameters.at("Maximum number to boost"))) {
		return; 
	} else if (total_vaccinated + n_vaccinated > 
					infection_parameters.at("Maximum number to vaccinate")) {
		n_vaccinated = infection_parameters.at("Maximum number to vaccinate") - total_vaccinated;
		std::cout << "Requested number of agents to vaccinate exceeds" 
				  << " the maximum allowable count - reducing to " << n_vaccinated << std::endl; 
	}
	if (total_boosted + n_boosted >
					infection_parameters.at("Maximum number to boost")) {
		n_boosted = infection_parameters.at("Maximum number to boost") - total_boosted;
		std::cout << "Requested number of agents to boost exceeds" 
				  << " the maximum allowable count - reducing to " << n_boosted << std::endl; 
	}

	// Vaccinate and boost if possible, update the counter
	// Assumes enough eligible
	cur_vaccinated = vaccinations.vaccinate_random_time_offset(agents, n_vaccinated, n_boosted, infection, time);	
	total_vaccinated += cur_vaccinated;	
	total_boosted += n_boosted;
}

// Vaccinate specific group of agents in the population
void ABM::vaccinate_group()
{
	;
}

//
// Transmission of infection
//

// Transmit infection - original way 
void ABM::transmit_infection() 
{
	check_events();
	distribute_leisure();
	compute_outside_locations();
	compute_place_contributions();	
	compute_state_transitions();
	reset_contributions();
	advance_in_time();
}

// Constant rate testing and vaccination 
void ABM::transmit_with_vac() 
{
	check_events();
	vaccinate();
	distribute_leisure();
	compute_place_contributions();	
	compute_state_transitions();
	reset_contributions();
	advance_in_time();	
}

// Randomly vaccinate agents based on the daily rate
void ABM::vaccinate()
{
	// Adjust n_vaccinated 
	n_vaccinated = static_cast<int>(infection_parameters.at("vaccination rate")*dt);
	n_boosted = static_cast<int>(infection_parameters.at("fraction boosters")*n_vaccinated);
	// Apply at random to eligible agents 
	vaccinate_random();				
}

// Increase transmission rate and visiting frequency of leisure locations 
void ABM::reopen_leisure_locations()
{
	;
}

// Assign leisure locations for this step
void ABM::distribute_leisure()
{
	// Remove previous leisure assignments
	// Reset the ID for all that had a location 
	// This includes all agents, passed as well
	int old_loc_ID = 0;
	for (auto& agent : agents) {
		old_loc_ID = agent.get_leisure_ID();
		if (old_loc_ID > 0) {
			if (agent.get_leisure_type() == "household") {
				households.at(old_loc_ID - 1).remove_agent(agent.get_ID());
			} else if (agent.get_leisure_type() == "public") {
				// Only remove in-town leisure locations
				if(!leisure_locations.at(old_loc_ID -1).outside_town()){
					leisure_locations.at(old_loc_ID - 1).remove_agent(agent.get_ID());
				}
			} else {
				throw std::invalid_argument("Wrong leisure type: " + agent.get_leisure_type());
			}
		}
		agent.set_leisure_ID(0);
	}

	// One leisure location per household, or one per each more mobile agent
	// Automatically excludes hospital patients (including non-COVID ones)
	// and retirement home residents; also passed agents, alive and removed participate
	int house_ID = 0;
	for (auto& house : households) {
		house_ID = house.get_ID();
		// Exclude fully isolated
		if (contact_tracing.house_is_isolated(house_ID)) {
			continue;
		}
		// Looping through households automatically excludes 
		// agents that died and that are hospitalized
		std::vector<int> agent_IDs = house.get_agent_IDs();
		// First check for the whole household
		if (infection.get_uniform() <= infection_parameters.at("leisure - fraction")) {
			// Household is going as a whole
			check_select_and_register_leisure_location(agent_IDs, house_ID);
		}
	}	
}

// Checks if agent is in a condition that allows going to leisure locations
bool ABM::check_leisure_eligible(const Agent& agent, const int house_ID)
{
	// Skip agents that are treated or in home isolation
	// due to waiting for test, flu, or contact tracing
	// Skip symptomatic too
	if (agent.being_treated() || agent.home_isolated() 
		|| agent.symptomatic() || agent.symptomatic_non_covid()) {
		return false;
	}
	// Also skip if the agent is being tested at this step
	if ((agent.tested()) && (agent.get_time_of_test() <= time)
		&& (agent.tested_awaiting_test() == true)) {
		return false;	
	}
	// Skip guests
	if (agent.get_household_ID() != house_ID) { 
		return false;
	}
	return true;
}

// Finds the actual leisure location and registers eligible agent(s)
void ABM::check_select_and_register_leisure_location(const std::vector<int>& agent_IDs, const int house_ID)
{
	bool is_public = false;
	bool is_house = false;
	int loc_ID = 0; 
	
	// Assign location - single agent (one element ID vector) or the entire household
	loc_ID = mobility.assign_leisure_location(infection, house_ID, is_house, is_public);
	assert(loc_ID > 0);
	assert((is_house == true) || (is_public == true));

	// Skip households that are fully isolated
	if (is_house) {
		if (contact_tracing.house_is_isolated(loc_ID)) {
			// Continue drawing until either public or not isolated
			while (is_house && contact_tracing.house_is_isolated(loc_ID)) {
				loc_ID = mobility.assign_leisure_location(infection, house_ID, is_house, is_public);
				assert(loc_ID > 0);
				assert((is_house == true) || (is_public == true));
			}
		}
	}

	for (auto& aID : agent_IDs) {
		// Conditions under which the agent won't visit a leisure location
		if (check_leisure_eligible(agents.at(aID-1), house_ID) == false) {
			continue;
		}
		// Register an eligible agent at the leisure location
		if (is_house) {
			households.at(loc_ID-1).add_agent(aID);
			agents.at(aID-1).set_leisure_type("household");
			agents.at(aID-1).set_leisure_ID(loc_ID);
			// Record this visit
			contact_tracing.add_household(aID, loc_ID, static_cast<int>(time));
		} else if (is_public) {
			// Only add if leisure location is within town
			if(!leisure_locations.at(loc_ID-1).outside_town()){
				leisure_locations.at(loc_ID-1).add_agent(aID);
			}
			agents.at(aID-1).set_leisure_type("public");
			agents.at(aID-1).set_leisure_ID(loc_ID);
		}
	}
}

// Verify if anything happens at this step
void ABM::check_events()
{
	double tol = 1e-3;
	
	// New strain - random selection of the first carrier out of the susceptible poll
	// Assumes that strain 2 didn't exist before
	if (equal_floats<double>(time, infection_parameters.at("introduction of a new strain"), tol)){
		// Select agent
		std::vector<int> susceptible;
		int new_agent_ID = 0;
		for (const auto& agent : agents){
			if (!agent.infected()){
				susceptible.push_back(agent.get_ID());
			}
		}
		infection.vector_shuffle(susceptible);
		new_agent_ID = susceptible.at(0);
		Agent& new_agent = agents.at(new_agent_ID-1);
		// Remove from flu
		flu.remove_susceptible_agent(new_agent_ID);
		// Initialize properties without a possibility of testing (like initial exposed)
		int strain_id = 2;
		new_agent.set_infected(true);
		new_agent.set_strain(strain_id);
		initial_exposed(new_agent);		
	}
}

// Verify if anything that requires parameter changes happens at this step 
void ABM::check_events(std::vector<School>& schools, std::vector<Workplace>& workplaces)
{
	;
}

// Correct outside location fraction with relative prevalence of each strain
void ABM::compute_outside_locations() 
{	
	std::vector<int> all_infected = get_num_infected_strains(n_strains);
	double tot_strains = static_cast<double>(std::accumulate(all_infected.begin(),
							all_infected.end(), 0)); 
	for (int ist = 0; ist < n_strains; ++ist) {
		strain_correction.at(ist) = static_cast<double>(all_infected.at(ist))/tot_strains;
	}
	set_outside_workplace_transmission();
	set_outside_leisure_transmission();
}

// Update transmission dynamics in workplaces outside of the town
void ABM::set_outside_workplace_transmission()
{
	for (auto& workplace : workplaces) {
		if (workplace.outside_town()) {
			for (int ist = 1; ist <= n_strains; ++ist) {
				workplace.set_outside_infected(
					infection_parameters.at("fraction estimated infected")*strain_correction.at(ist-1), ist);
			}
		}
	}
}

// Update transmission dynamics in leisure locations outside of the town
void ABM::set_outside_leisure_transmission()
{
	for (auto& leisure_location : leisure_locations) {
		if (leisure_location.outside_town()) {
			for (int ist = 1; ist <= n_strains; ++ist) {
				leisure_location.set_outside_infected(
					infection_parameters.at("fraction estimated infected")*strain_correction.at(ist-1), ist);
			}
		}
	}
}


// Count contributions of all infectious agents in each place
void ABM::compute_place_contributions()
{
	for (const auto& agent : agents){

		// Only removed - dead don't contribute
		if (agent.removed_dead() == true) {
			continue;
		}

		// If susceptible and being tested - add to hospital's
		// total number of people present at this time step
		if (agent.infected() == false){
			if ((agent.tested() == true) && 
				(agent.tested_in_hospital() == true) &&
				(agent.get_time_of_test() <= time) && 
		 		(agent.tested_awaiting_test() == true)){
					hospitals.at(agent.get_hospital_ID() - 1).increase_total_tested();
			}			
			continue;
		}

		// Consider all infectious cases, raise 
		// exception if no existing case
		if (agent.exposed() == true){
			contributions.compute_exposed_contributions(agent, time, households, 
							schools, workplaces, hospitals, retirement_homes,
							carpools, public_transit, leisure_locations);
		}else if (agent.symptomatic() == true){
			contributions.compute_symptomatic_contributions(agent, time, households, 
							schools, workplaces, hospitals, retirement_homes,
							carpools, public_transit, leisure_locations);
		}else{
			throw std::runtime_error("Agent does not have any state");
		}
	}
	contributions.total_place_contributions(households, schools, 
											workplaces, hospitals, retirement_homes,
											carpools, public_transit, leisure_locations);
}

// Determine infection propagation and
// state changes 
void ABM::compute_state_transitions()
{
	int newly_infected = 0, is_recovered = 0;
	bool re_vac = false;
	// Infected state change flags: 
	// recovered - healthy, recovered - dead, tested at this step,
	// tested positive at this step, tested false negative
	std::vector<int> state_changes = {0, 0, 0, 0, 0};
	// Susceptible state changes
	// infected, tested, tested negative, tested false positive
	std::vector<int> s_state_changes = {0, 0, 0, 0};
	// First entry is one if agent recovered, second if agent died
	std::vector<int> removed = {0,0};

	// Store information for that day
	n_infected_day.push_back(0);
	tested_day.push_back(0);
	tested_pos_day.push_back(0);
	tested_neg_day.push_back(0);
	tested_false_pos_day.push_back(0);
	tested_false_neg_day.push_back(0);

	for (auto& agent : agents){

		// Skip the removed - dead 
		if (agent.removed_dead() == true){
			continue;
		}

		std::fill(state_changes.begin(), state_changes.end(), 0);
		std::fill(s_state_changes.begin(), s_state_changes.end(), 0);
		re_vac = transitions.common_transitions(agent, time, 
								schools, workplaces, hospitals, 
								retirement_homes, carpools, public_transit, 
								contact_tracing, infection_parameters);
		if (re_vac == true) {
			// Subtract from total since re-vaccinating (to not count twice)
			--total_vaccinated;
		}
		// For an agent not infected with any strain
		if (agent.infected() == false){
			s_state_changes = transitions.susceptible_transitions(agent, time,
							dt, infection, households, schools, workplaces, 
							hospitals, retirement_homes, carpools, public_transit,
						   	leisure_locations, infection_parameters, 
							agents, flu, testing, n_strains);
			n_infected_tot += s_state_changes.at(0);
			if (agent.infected()) {
				n_infected_tot_strain.at(agent.get_strain()-1) += s_state_changes.at(0);
			}
			// True infected by timestep, from the first time step
			if (s_state_changes.at(0) == 1){
				++n_infected_day.back();
			}
		}else if (agent.exposed() == true){
			state_changes = transitions.exposed_transitions(agent, infection, time, dt, 
										households, schools, workplaces, hospitals,
										retirement_homes, carpools, public_transit,
						   				infection_parameters, testing);
			n_recovering_exposed += state_changes.at(0);
			n_recovered_tot += state_changes.at(0);
		}else if (agent.symptomatic() == true){
			state_changes = transitions.symptomatic_transitions(agent, time, dt,
						infection, households, schools, workplaces, hospitals,
							retirement_homes, carpools, public_transit,
						   	infection_parameters);
			n_recovered_tot += state_changes.at(0);
			// Collect only after a specified time
			if (time >= infection_parameters.at("time to start data collection")){
				if (state_changes.at(1) == 1){
					// Dead after testing
					++n_dead_tested;
					++n_dead_tot;
				} else if (state_changes.at(1) == 2){
					// Dead with no testing
					++n_dead_not_tested;
					++n_dead_tot;
				}
			}
		}else{
			throw std::runtime_error("Agent does not have any infection-related state");
		}

		// Recording testing changes for this agent
		if (time >= infection_parameters.at("time to start data collection")){
			if (agent.exposed() || agent.symptomatic()){
				if (state_changes.at(2) == 1){
					++tested_day.back();
					++tot_tested;
				}
				if (state_changes.at(3) == 1){
					++tested_pos_day.back();
					++tot_tested_pos;
					// Confirmed positive - initiate contact tracing
					contact_trace_agent(agent);
				}
				if (state_changes.at(4) == 1){
					++tested_false_neg_day.back();
					++tot_tested_false_neg;
				}
			} else {
				// Susceptible
				if (s_state_changes.at(1) == 1){
					++tested_day.back();
					++tot_tested;
				}
				if (s_state_changes.at(2) == 1){
					++tested_neg_day.back();
					++tot_tested_neg;
				}
				if (s_state_changes.at(3) == 1){
					++tested_false_pos_day.back();
					++tot_tested_false_pos;
					// False positive - initiate contact tracing
					contact_trace_agent(agent);
				}
			}
		}
	}
}

// Initiate contact tracing of an agent
void ABM::contact_trace_agent(Agent& agent)
{
	// All the cases that don't need to be traced now
	if (agent.hospital_non_covid_patient() || agent.hospitalized()
			|| agent.hospitalized_ICU()) {
		return;
	}

	int aID = agent.get_ID();

	// Collect all agents to trace
	std::unordered_set<int> all_traced;	
	std::vector<int> traced;

	// Consider each type
	if (agent.student()) {
		traced = contact_tracing.isolate_school(aID, agents, 
					schools.at(agent.get_school_ID()-1), 
					static_cast<int>(infection_parameters.at("max contacts at school")), 
					infection);
		all_traced.insert(traced.begin(), traced.end());
	}
	if (agent.works() && !agent.works_from_home()) {
		if (agent.retirement_home_employee()) {
			traced = contact_tracing.isolate_retirement_home(aID, agents, 
					retirement_homes.at(agent.get_work_ID()-1), 
					static_cast<int>(infection_parameters.at("max contacts at RH")),
					static_cast<int>(infection_parameters.at("max contacts residents at RH")),
					infection);
			all_traced.insert(traced.begin(), traced.end());
		} else if (agent.school_employee()) {
			traced = contact_tracing.isolate_school(aID, agents, 
					schools.at(agent.get_work_ID()-1), 
					static_cast<int>(infection_parameters.at("max contacts at school")), 
					infection);
			all_traced.insert(traced.begin(), traced.end());
		} else {
			traced = contact_tracing.isolate_workplace(aID, agents, 
				workplaces.at(agent.get_work_ID()-1), 
				static_cast<int>(infection_parameters.at("max contacts at workplace")),
				infection);
			all_traced.insert(traced.begin(), traced.end());
		}
	}
	if (agent.hospital_employee()) {
		traced = contact_tracing.isolate_hospital(aID, agents, 
				hospitals.at(agent.get_hospital_ID()-1), 
				static_cast<int>(infection_parameters.at("max contacts at hospital")),
				infection);
		all_traced.insert(traced.begin(), traced.end());	
	}
 	if (agent.get_work_travel_mode() == "carpool") {
		traced = contact_tracing.isolate_carpools(aID, agents, 
				carpools.at(agent.get_carpool_ID()-1)); 
		all_traced.insert(traced.begin(), traced.end());
	}
	if (agent.retirement_home_resident()) { 
			traced = contact_tracing.isolate_retirement_home(aID, agents, 
						retirement_homes.at(agent.get_household_ID()-1), 
						static_cast<int>(infection_parameters.at("max contacts at RH")),
						static_cast<int>(infection_parameters.at("max contacts residents at RH")),
						infection);
		all_traced.insert(traced.begin(), traced.end());
	} else {
		// Private visits
		traced = contact_tracing.isolate_visited_households(aID, households,
					infection_parameters.at("contact tracing compliance"), infection,
					static_cast<int>(time), dt);
		all_traced.insert(traced.begin(), traced.end());
		// Agent's household (only eligible)
		traced = contact_tracing.isolate_household(aID, 
						households.at(agent.get_household_ID()-1));
		all_traced.insert(traced.begin(), traced.end());
	}
	// Process all the traced agents
	setup_traced_isolation(all_traced);
}

void ABM::setup_traced_isolation(const std::unordered_set<int>& traced_IDs) 
{
	for (const auto& aID : traced_IDs) {
		if (!agents.at(aID-1).contact_traced()) {
			transitions.new_quarantined(agents.at(aID-1), time, dt, 
    	            infection, households, schools, workplaces, hospitals, retirement_homes,
    	            carpools, public_transit, infection_parameters);
		}
	}
}

// Start detection, initialize agents with flu, vaccinate
void ABM::start_testing_flu_and_vaccination(const bool dont_vac)
{
	double tol = 1e-3;
	// Initialize agents with flu the time step the testing starts 
	// Optionally also vaccinate part of the population or/and specific groups
	if (equal_floats<double>(time, infection_parameters.at("start testing"), tol)){
		// Vaccinate
		if (dont_vac == false) {
			if (random_vaccines == true){
				vaccinate_random();
			}
			if (group_vaccines == true){
				vaccinate_group();
			}
		}
		// Initialize flu agents
		for (const auto& agent : agents){
			if (!agent.infected() && !agent.removed() && !agent.vaccinated()){
				// If not patient or hospital employee
				// Add to potential flu group
				if (!agent.hospital_employee() && !agent.hospital_non_covid_patient()){
					flu.add_susceptible_agent(agent.get_ID());
				}
			}
		}
		// Randomly assign portion of susceptible with flu
		// The set agents flags
		std::vector<int> flu_IDs = flu.generate_flu();
		for (const auto& ind : flu_IDs){
			Agent& agent = agents.at(ind-1);
			const int n_hospitals = hospitals.size();
			transitions.process_new_flu(agent, n_hospitals, time,
					   		 schools, workplaces, retirement_homes,
							 carpools, public_transit, infection, 
							 infection_parameters, flu, testing);
		}
	}
}

//
// Getters
//

// Calculate the average number of contacts an agent makes
double ABM::get_average_contacts()
{
	int n_tot = 0, les_loc = 0;
	for (const auto& agent : agents){
		if (agent.hospital_employee()) {
			n_tot += households.at(agent.get_household_ID()-1).get_number_of_agents();
			
			if (agent.student()) {
				if (time < infection_parameters.at("school closure")) { 
					n_tot += std::min(schools.at(agent.get_school_ID()-1).get_number_of_agents(),
									static_cast<int>(infection_parameters.at("max contacts at school")));
				}
			}

			n_tot += std::min(hospitals.at(agent.get_hospital_ID()-1).get_number_of_agents(),
								static_cast<int>(infection_parameters.at("max contacts at hospital")));
		} else if (agent.hospital_non_covid_patient()) {
			n_tot += std::min(hospitals.at(agent.get_hospital_ID()-1).get_number_of_agents(),
								static_cast<int>(infection_parameters.at("max contacts at hospital")));
		} else {
			if (agent.retirement_home_resident()) {
				n_tot += std::min(retirement_homes.at(agent.get_household_ID()-1).get_number_of_agents(),
									static_cast<int>(infection_parameters.at("max contacts at RH")));
			} else {
				n_tot += households.at(agent.get_household_ID()-1).get_number_of_agents();
			}

			if (agent.student()) {
				if (time < infection_parameters.at("school closure")) {
					n_tot += std::min(schools.at(agent.get_school_ID()-1).get_number_of_agents(),
									static_cast<int>(infection_parameters.at("max contacts at school")));
				}
			}

			if (agent.works()) {
				if (agent.retirement_home_employee()) {
					n_tot +=  std::min(retirement_homes.at(agent.get_work_ID()-1).get_number_of_agents(),
									static_cast<int>(infection_parameters.at("max contacts at RH")));
				} else if (agent.school_employee()) {
					if (time < infection_parameters.at("school closure")) {
						n_tot += std::min(schools.at(agent.get_work_ID()-1).get_number_of_agents(),
									static_cast<int>(infection_parameters.at("max contacts at school")));
					}
				} else {
					if (!agent.works_from_home()) {
						n_tot +=  std::min(workplaces.at(agent.get_work_ID()-1).get_number_of_agents(),
							static_cast<int>(infection_parameters.at("max contacts at workplace")));
					}
				}
			}
		}

		// Transit
		if (agent.get_work_travel_mode() == "carpool") {
			n_tot +=  carpools.at(agent.get_carpool_ID()-1).get_number_of_agents();
		} else if (agent.get_work_travel_mode() == "public") {
			n_tot +=  public_transit.at(agent.get_public_transit_ID()-1).get_number_of_agents();
		}

		// Leisure locations
		les_loc = agent.get_leisure_ID();	
		if (les_loc > 0) {
			if (agent.get_leisure_type() == "public") {
				n_tot += leisure_locations.at(les_loc - 1).get_number_of_agents();
			} else {
				n_tot += households.at(les_loc - 1).get_number_of_agents();
			}
		}	
	}
	return (static_cast<double>(n_tot))/(static_cast<double>(agents.size()));
}

//
// I/O
//

// Save infection information
void ABM::print_infection_parameters(const std::string filename) const
{
	FileHandler file(filename, std::ios_base::out | std::ios_base::trunc);
	std::fstream &out = file.get_stream();	

	for (const auto& entry : infection_parameters){
		out << entry.first << " " << entry.second << "\n";
	}	
}

// Save age-dependent distributions
void ABM::print_age_dependent_distributions(const std::string filename) const
{
	FileHandler file(filename, std::ios_base::out | std::ios_base::trunc);
	std::fstream &out = file.get_stream();	

	for (const auto& entry : age_dependent_distributions){
		out << entry.first << "\n";
		for (const auto& e : entry.second)
			out << e.first << " " << e.second << "\n";
	}	
}



