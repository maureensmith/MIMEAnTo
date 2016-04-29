"""
--Plain text output of numpy array
--Version of June 15, 2014--

COMMAND LINE ARGUMENTS

python output_3d.py numpy_file_in plain_text_file_out

"""

from numpy import *
from sys import argv


array = load(argv[1])
array_dimensions = array.shape

outfile = open(argv[2], 'w')
outfile.write("pos1\tpos2\tpos3\tAAA\tAAC\tAAG\tAAT\tACA\tACC\tACG\tACT\tAGA\tAGC\tAGG\tAGT\tATA\tATC\tATG\tATT\tCAA\tCAC\tCAG\tCAT\tCCA\tCCC\tCCG\tCCT\tCGA\tCGC\tCGG\tCGT\tCTA\tCTC\tCTG\tCTT\tGAA\tGAC\tGAG\tGAT\tGCA\tGCC\tGCG\tGCT\tGGA\tGGC\tGGG\tGGT\tGTA\tGTC\tGTG\tGTT\tTAA\tTAC\tTAG\tTAT\tTCA\tTCC\tTCG\tTCT\tTGA\tTGC\tTGG\tTGT\tTTA\tTTC\tTTG\tTTT\n")

for i in range(0, array_dimensions[0]):
    for j in range(i+1, array_dimensions[1]):
        for k in range(i+2, array_dimensions[2]):
            outfile.write("%s\t%s\t%s" %(i+1,j+1,k+1))
            
            for l in range(0, array_dimensions[3]):
                outfile.write("\t%s" %(array[i,j,k,l]))

            outfile.write("\n")

outfile.close()

