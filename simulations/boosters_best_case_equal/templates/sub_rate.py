# Script to subsititute specific entry in the parameter file
#
# Arguments: input and output file names, exact or part of the entry to substitute,
#	new value of the entry
# Throws an error if no such entry found

import sys

if len(sys.argv) != 5:
	sys.stderr.write('Usage: python3.X %s in_file_name out_file_name entry_name entry_value\n' % sys.argv[0])
	raise SystemExit(1)

infile = sys.argv[1]
outfile = sys.argv[2]
entry_name = sys.argv[3]
entry_new_value = sys.argv[4]

found = False
with open(infile, 'r') as fin, open(outfile, 'w') as fout:
	for line in fin:
		if entry_name in line:
			# This parameter needs to be changed
			# Write this line to file (parameter name)
			fout.write(line)
			# Value should be on the next line	
			fout.write(entry_new_value +'\n')
			next(fin)
			found = True
		else:
			fout.write(line)	

if not found:
	raise RuntimeError(entry_name + ' not found in ' + infile)
