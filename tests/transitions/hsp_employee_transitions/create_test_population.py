#
# Script for generating population files for 
#	running contributions tests
#

import random

# Input file with info on population
agents_in = 'test_data/NR_agents.txt' 
# Number of agents to create
n_agents = 80000
# Place numbers
n_houses = 29645 
n_works = 2544 
n_hsp = 3 
n_rh = 5
n_schools = 68 
n_carpools = 1335 
n_public = 14 
n_leisure = 21
n_inf0 = 80 

# Transit choices
transit_modes = ['car', 'carpool', 'public', 'walk', 'other']
# Occupation choices
occupations = ['A', 'B', 'C', 'D', 'E']

# IDs of initially infected
inf_agents = random.sample(range(1,n_agents+1), n_inf0);

with open(agents_in, 'w') as fout:
	for i in range(1,n_agents+1):
		agent = ['0']*22
		# Default occupation
		agent[21] = 'none'	
		# Age and location
		agent[2] = str(random.randint(0,110))
		agent[3] = str(random.random()*10)
		agent[4] = str(random.random()*10)

		# Default travel mode
		agent[17] = 'None'

		# Infected
		if i in inf_agents:
			agent[14] = '1'
		
		# Residence
		if random.random() < 0.05:
			print('Creating a non-COVID hospital patient')
			agent[6] = '1'
			agent[13] = str(random.randint(1,n_hsp))
			fout.write((' ').join(agent)+'\n')
			continue
		if random.random() < 0.1:
			print('Creating a retirement home resident')
			agent[8] = '1'
			agent[5] = str(random.randint(1,n_rh))
			fout.write((' ').join(agent)+'\n')
			continue
		# Regular household, determine if infected	
		agent[5] = str(random.randint(1,n_houses))

		# Workplaces and schools
		# Hospital staff
		if random.random() < 0.7:
			print('Agent works in a hospital')
			# Occupation
			agent[21] = 'A'
			agent[12] = '1' 
			agent[13] = str(random.randint(1,n_hsp))
			# Time to reach work
			agent[16] = str(random.uniform(5.0,90.0))
			# Travel mode 
			agent[17] = random.choice(transit_modes)
			# Special work ID
			agent[18] = agent[13]
			# Transit IDs for carpools and public
			if agent[17] == 'carpool':
				agent[19] = str(random.randint(1, n_carpools))
			if agent[17] == 'public':
				agent[20] = str(random.randint(1, n_public))
			if random.random() < 0.25:
				print('\t\t Agent also goes to school')
				agent[0] = '1'
				agent[7]= str(random.randint(1,n_schools))
			fout.write((' ').join(agent)+'\n')
			continue
		# Retirement homes
		if random.random() < 0.3:
			print('Agent works in a retirement home')
			# Occupation
			agent[21] = 'A'
			agent[1] = '1' 
			agent[9] = '1'
			agent[11] = str(random.randint(1,n_rh))
			# Time to reach work
			agent[16] = str(random.uniform(5.0,90.0))
			# Travel mode 
			agent[17] = random.choice(transit_modes)
			# Special work ID
			agent[18] = agent[11]
			# Transit IDs for carpools and public
			if agent[17] == 'carpool':
				agent[19] = str(random.randint(1, n_carpools))
			if agent[17] == 'public':
				agent[20] = str(random.randint(1, n_public))
			if random.random() < 0.25:
				print('\t\t Agent also goes to school')
				agent[0] = '1'
				agent[7]= str(random.randint(1,n_schools))
			fout.write((' ').join(agent)+'\n')
			continue
		# Works at school
		if random.random() < 0.3:
			print('Agent works at school')
			# Occupation
			agent[21] = 'A'
			agent[1] = '1' 
			agent[10] = '1'
			agent[11] = str(random.randint(1,n_schools))
			# Time to reach work
			agent[16] = str(random.uniform(5.0,90.0))
			# Travel mode 
			agent[17] = random.choice(transit_modes)
			# Special work ID
			agent[18] = agent[11]
			# Transit IDs for carpools and public
			if agent[17] == 'carpool':
				agent[19] = str(random.randint(1, n_carpools))
			if agent[17] == 'public':
				agent[20] = str(random.randint(1, n_public))
			if random.random() < 0.25:
				print('\t\t Agent also goes to school')
				agent[0] = '1'
				agent[7]= str(random.randint(1,n_schools))
			fout.write((' ').join(agent)+'\n')
			continue
		# Works elsewhere
		if random.random() < 0.4:
			agent[1] = '1' 
			# Occupation
			agent[21] = random.choice(occupations)
			agent[11] = str(random.randint(1, n_works))
			if random.random() < 0.1:
				agent[15] = '1'
				agent[17] = 'wfh'
			else:
				# Time to reach work
				agent[16] = str(random.uniform(5.0,90.0))
				# Travel mode 
				agent[17] = random.choice(transit_modes)
				# Transit IDs for carpools and public
				if agent[17] == 'carpool':
					agent[19] = str(random.randint(1, n_carpools))
				if agent[17] == 'public':
					agent[20] = str(random.randint(1, n_public))
			if random.random() < 0.25:
				print('\t\t Agent also goes to school')
				agent[0] = '1'
				agent[7]= str(random.randint(1,n_schools))
			fout.write((' ').join(agent)+'\n')
		else:
			if random.random() < 0.5:
				print('Agent goes to school')
				agent[0] = '1'
				agent[7]= str(random.randint(1,n_schools))
			fout.write((' ').join(agent)+'\n')
		

