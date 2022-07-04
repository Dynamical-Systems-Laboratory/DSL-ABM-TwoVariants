import subprocess, os
import math

# Protection reduction (0 - no reduction, 1 - all reduced)
prot_red = [x*0.1 for x in range(0,11)] 
# Testing efficacy (fraction false negatives for strain 2)
test_eff = [x*0.1 for x in range(0,11)]
# Max asymptomatic correction
a_vac = 1.15

root_dir = os.path.abspath(os.getcwd())
for pr in prot_red:
    value = pr
    for te in test_eff:
        dir_name = 'dir_' + str(round(pr, 1)) + '_' + str(round(te, 1))
        print('Processing ' + dir_name)
        
        # Create working directory and copy all required files
        subprocess.run('mkdir ' + dir_name, shell=True, check=True)
        subprocess.run('cp -r templates/. ' + dir_name, shell=True, check=True)
        os.chdir(dir_name)
        
        # Compute and substitute parameters
        # -- Testing efficiency
        mv_param = 'mv input_data/infection_parameters.txt input_data/temp.txt'
        subprocess.run(mv_param, shell=True, check=True)
        py_comm = 'python3 sub_rate.py input_data/temp.txt input_data/infection_parameters.txt \"fraction false negative - strain 2\" ' + str(round(te, 1))
        subprocess.run(py_comm, shell=True, check=True)
        # -- Vaccine effectiveness
        mv_param = 'mv input_data/vaccination_parameters_strain_1.txt input_data/temp.txt'  
        subprocess.run(mv_param, shell=True, check=True)
        py_comm = 'python3 sub_rate.py input_data/temp.txt input_data/vaccination_parameters_strain_1.txt \"Effectiveness reduction for strain 2\" ' + str(round(value, 3))
        subprocess.run(py_comm, shell=True, check=True)
        # -- Asymptomatic 
        mv_param = 'mv input_data/vaccination_parameters_strain_1.txt input_data/temp.txt'  
        subprocess.run(mv_param, shell=True, check=True)
        py_comm = 'python3 sub_rate.py input_data/temp.txt input_data/vaccination_parameters_strain_1.txt \"Asymptomatic reduction for strain 2\" ' + str(round(value, 3))
        subprocess.run(py_comm, shell=True, check=True)
        # -- Transmission 
        mv_param = 'mv input_data/vaccination_parameters_strain_1.txt input_data/temp.txt'  
        subprocess.run(mv_param, shell=True, check=True)
        py_comm = 'python3 sub_rate.py input_data/temp.txt input_data/vaccination_parameters_strain_1.txt \"Transmission reduction for strain 2\" ' + str(round(value, 3))
        subprocess.run(py_comm, shell=True, check=True)
        # -- Severity 
        mv_param = 'mv input_data/vaccination_parameters_strain_1.txt input_data/temp.txt'  
        subprocess.run(mv_param, shell=True, check=True)
        py_comm = 'python3 sub_rate.py input_data/temp.txt input_data/vaccination_parameters_strain_1.txt \"Severe reduction for strain 2\" ' + str(round(value, 3))
        subprocess.run(py_comm, shell=True, check=True)
        # -- Mortality 
        mv_param = 'mv input_data/vaccination_parameters_strain_1.txt input_data/temp.txt'  
        subprocess.run(mv_param, shell=True, check=True)
        py_comm = 'python3 sub_rate.py input_data/temp.txt input_data/vaccination_parameters_strain_1.txt \"Death reduction for strain 2\" ' + str(round(value, 3))
        subprocess.run(py_comm, shell=True, check=True)

        # Run
        subprocess.run('sbatch abm_submission.sh', shell=True, check=True)

        # Go back to root
        os.chdir(root_dir)
