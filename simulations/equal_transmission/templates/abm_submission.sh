#!/bin/bash
#SBATCH --time=15:00:00
#SBATCH --nodes=1
#SBATCH --mem=4GB 

module load matlab/2020b 

# Compile
python3 compilation.py  

# Run
matlab -nodisplay -nojvm -nosplash -nodesktop -r \
        "try, parametric_driver, catch e, disp(getReport(e)), exit(1), end, exit(0);"
      echo "matlab exit code: $?"
