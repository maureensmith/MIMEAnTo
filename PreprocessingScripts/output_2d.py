"""
--Plain text output of numpy array
--Version of June 15, 2014--

COMMAND LINE ARGUMENTS

python output_2d.py numpy_file_in plain_text_file_out

"""

from numpy import *
from sys import argv


array = load(argv[1])
array_dimensions = array.shape

outfile = open(argv[2], 'w')
outfile.write("pos1\tpos2\tAA\tAC\tAG\tAT\tCA\tCC\tCG\tCT\tGA\tGC\tGG\tGT\tTA\tTC\tTG\tTT\n")

for i in range(0, array_dimensions[0]):
    for j in range(i+1, array_dimensions[1]):
            
            outfile.write("%s\t%s" %(i+1,j+1))
            
            for k in range(0, array_dimensions[2]):
                outfile.write("\t%s" %(array[i,j,k]))

            outfile.write("\n")

outfile.close()

