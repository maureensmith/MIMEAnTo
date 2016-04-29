
"""
--Plain text output of numpy array
--Version of June 15, 2014--

COMMAND LINE ARGUMENTS

python 1d_output.py numpy_file_in plain_text_file_out

"""

from numpy import *
from sys import argv

code_dict = {0:'A', 1:'C', 2:'G', 3:'T'}

array = load(argv[1])
array_dimensions = array.shape

print array.shape
print array

outfile = open(argv[2], 'w')
outfile.write("pos1\tA\tC\tG\tT\n")

for i in range(0, array_dimensions[0]):
        outfile.write("%s" %(i+1))
            
        for k in range(0, array_dimensions[1]):
            outfile.write("\t%s" %(array[i,k]))
    
        outfile.write("\n")
outfile.close()

