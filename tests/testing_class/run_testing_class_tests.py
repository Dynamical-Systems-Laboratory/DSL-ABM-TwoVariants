import subprocess

import sys
py_path = '../../scripts/'
sys.path.insert(0, py_path)

import utils as ut
from colors import *

py_version = 'python3'

#
# Compile and run Testing class tests
#

# Compile
subprocess.call([py_version + ' compilation.py'], shell=True)

ut.msg('Testing class tests', CYAN)
subprocess.call(['./tst_cls_tst'], shell=True)
