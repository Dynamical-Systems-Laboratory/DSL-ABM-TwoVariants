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
src_files = path + 'mobility.cpp'
src_files += ' ' + path + 'infection.cpp'
src_files += ' ' + path + 'utils.cpp'
src_files += ' ' + path + 'places/place.cpp'
src_files += ' ' + path + 'places/household.cpp'
src_files += ' ' + path + 'places/workplace.cpp'
src_files += ' ' + path + 'places/school.cpp'
src_files += ' ' + path + 'places/hospital.cpp'
src_files += ' ' + path + 'places/retirement_home.cpp'
src_files += ' ' + path + 'places/transit.cpp'
src_files += ' ' + path + 'places/leisure.cpp'
src_files += ' ' + path + 'io_operations/FileHandler.cpp'
src_files += ' ' + path + 'io_operations/load_parameters.cpp'
tst_files = '../common/test_utils.cpp'
# Directory with files for testing
data_dir = './test_data/'

#
# Tests
#

# Public functionality 
# Name of the executable
exe_name = 'mb_tests'
# Files needed only for this build
spec_files = 'mobility_tests.cpp '
compile_com = ' '.join([cx, std, opt, '-o', exe_name, spec_files, tst_files, src_files])
subprocess.call([compile_com], shell=True)
