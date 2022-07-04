import subprocess, glob, os

#
# Input 
#

# Path to the main directory
path = '../../src/'
# Compiler options
cx = 'g++'
std = '-std=c++11'
opt = '-O0'
# Common source files
src_files = path + 'vaccinations.cpp' 
src_files += ' ' + path + 'agent.cpp'
src_files += ' ' + path + 'three_part_function.cpp'
src_files += ' ' + path + 'four_part_function.cpp'
src_files += ' ' + path + 'infection.cpp'
src_files += ' ' + path + 'utils.cpp'
src_files += ' ' + path + 'io_operations/FileHandler.cpp'
src_files += ' ' + path + 'io_operations/load_parameters.cpp'
tst_files = '../common/test_utils.cpp'

#
# Tests
#

# Test suite 1
exe_name = 'vac_tests'
# Files needed only for this build
spec_files = 'vaccinations_tests.cpp '
compile_com = ' '.join([cx, std, opt, '-o', exe_name, spec_files, tst_files, src_files])
subprocess.call([compile_com], shell=True)

