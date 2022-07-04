import subprocess

import sys
py_path = '../../scripts/'
sys.path.insert(0, py_path)

import utils as ut
from colors import *

py_version = 'python3'

#
# Compile and run all the tests
#

# Compile
subprocess.call([py_version + ' compilation.py'], shell=True)

# Test suite 1
ut.msg('ThreePartFunction functionality tests', CYAN)
subprocess.call(['./tpf_tests'], shell=True)

# Test suite 2
ut.msg('FourPartFunction functionality tests', CYAN)
subprocess.call(['./fpf_tests'], shell=True)


