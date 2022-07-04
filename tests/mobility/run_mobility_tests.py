import subprocess

import sys
py_path = '../../scripts/'
sys.path.insert(0, py_path)

import utils as ut
from colors import *

py_version = 'python3'

#
# Compile and run the Mobility class specific tests
#

# Compile
subprocess.call([py_version + ' compilation.py'], shell=True)

# Run the test suite 
ut.msg('Tests for the Mobility class', CYAN)
subprocess.call(['./mb_tests'], shell=True)

