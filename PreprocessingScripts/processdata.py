##############################################
#                                            #
# Mutational Interference Mapping Experiment #
#               Python Module                #
#                                            #
##############################################

"""
    -- RS
    --Version including numba optimisations--
    --21-July, 2014--
    --Based on L.D. CoVar script
    
    COMMAND-LINE
    You can run the program with command-lines like these:
    ./use_script3d.py 3D samfilea.sam samfileb.sam indexes/HIV1_535.fas outputfile
    
    ARGUMENTS
    Arguments to give in the command-line in the following order:
    argument 1: String: '1D','2D','3D'
    argument 2: String: name of samfile 1
    argument 3: String: name of samfile 2
    argument 4: String: name of FASTA file containing the reference sequence
    argument 5: String: name of results file to save
    argument 6: Optional argument start and end position of small sequence to analyse
    
    DESCRIPTION
    Python module that counts base variations between a reference sequence (HIV RNA sequence) and RNA-Sequencing reads
    - First step: it parses both SAM files from paired end read, containing reads aligned to the reference sequence. It fetches alignment Matches by decoding the CIGAR and using the position of the first reference base that gives a Match. Thus, reads are translated into alignment matches or mismatches in comparison to the reference sequence.
    - Second step: it merged Matched paired-end reads. When these reads overlap, it removes redundancy and all sequencing errors.
    - Third step: it counts base variations using one of three methods:
    
        The '1d' method is a "vertical" analysis that depends on the number of reads. At each position, it counts the number of A, C, G, T and fills a 1D matrix [number of A, number of C, number of G, number of T]. When base in the read sequence is different to A,C,G,T (for example, N = unknown base), it removes this base.
        
        The '2d' method is a "vertical and horizontal" analysis that depends on the number of reads and the length of reads. It tests all nonredundant combinations of two positions (pos1-pos2, pos1-pos3, pos2-pos3 ; but not pos2-pos1) and fills a 2D matrix:
        
        [number of AA, AC, AG, AT]
        [number of CA, CC, CG, CT]
        [number of GA, GC, GG, GT]
        [number of TA, TC, TG, TT]
    
        The '3d' method is similar to the 2d analysis, except it tests all nonredundant combinations of three positions, filling a 3d matrix
        
    - Final step: it serializes the results by writing a binary OUTFILE (.npy)

"""

from sys import argv, exit
from os import listdir
from sys import exit
from numpy import *
from pysam import *
from numba import autojit

def fasta_parser(file):
    """
        Read in the reference sequence used for the alignment
    """
    sequence = ''
    infh = open(file, 'r')
    line = infh.readline()
    while line:
        if line[0] == '>':
            l = line.split()
            name = l[0][1:]
        elif line[0] != '>':
            sequence += line[:-1]
        line = infh.readline()
    infh.close()
    if sequence.isupper() == 0:
        sequence = sequence.upper()
    print "Reference sequence: %s, %s bases" %(name, len(sequence))
    return sequence

def sam_parser(file,file2,file_out,refseq,flag,s):
    """
        Read in the both sam files (output from the alignment) and process each aligned read sequentially
    """
    pairend = []
    total_read_nb = 0
    used_read_nb = 0
    no_cig_read_nb = 0
    remaining_read_nb = 0
    seq_error = 0
    seq_true = 0
    pos1 = None
    pos2 = None
 
    if ':' in s:
        l = s.split(':')
        pos1 = int(l[0])
        pos2 = int(l[1])
        array_dimension = pos2-pos1

        if pos1 >= pos2:
            print "Error: the first integer of the fourth argument must be lower than the second integer."
            exit()
    elif s=='':
        print "Analysing "+str(len(refseq))+"bp"
        array_dimension = len(refseq)
        pos1 = 1
        pos2 = len(refseq)
    elif s != '' and ':' not in s:
        print "Error: the fourth argument must be 2 integers separated by the character ':'."
        exit()

    """ Create appropriate array based on 1d, 2d or 3d analysis"""
    if flag == '1D':
        results = zeros((array_dimension,4), dtype=int32)
    elif flag == '2D':
        results = zeros((array_dimension,array_dimension,16), dtype=int32)
    elif flag == '3D':
        results = zeros((array_dimension,array_dimension,array_dimension,64), dtype=int32)

    samfile1 = Samfile(file, 'r') #samfile: <type 'csamtools.Samfile'> ; 'r' for sam files and 'rb' for bam files
    samfile2 = Samfile(file2, 'r')
    alignedread2iter=samfile2.fetch()

    for alignedread1 in samfile1.fetch(): #alignedread: <type 'csamtools.AlignedRead'>
        alignedread2=alignedread2iter.next()
        total_read_nb += 1
        
    	if total_read_nb%1000 == 0:
            print file.split('.sam')[0]+': Number of reads: '+str(total_read_nb)

        if alignedread1.is_unmapped or alignedread2.is_unmapped: #alignedread.cigar is a list or None
            no_cig_read_nb += 1
        else:
            read1 = align(alignedread1, refseq)
            read2 = align(alignedread2, refseq)

            used_read_nb += 1
            res = merger(read1[1:], read2[1:], seq_error, seq_true) #qname removed from each read

            read12 = res[0]
            seq_error = res[1]
            seq_true = res[2]

            arrays = list_to_array(read12, pos1, pos2) # Convert dictionary representation of specified sequence to numpy array
            position_array = arrays[0]
            base_array = arrays[1]
            
            if flag == '1D':
                results = analyze1d(position_array, base_array, results, pos1)
            elif flag == '2D':
                results = analyze2d(position_array, base_array, results, pos1)
            elif flag == '3D':
                results = analyze3d(position_array, base_array, results, pos1)

    print results
    save(file_out+'_'+flag,results)
    samfile1.close()
    samfile2.close()
    return total_read_nb, used_read_nb, no_cig_read_nb, remaining_read_nb, seq_error, seq_true

def align(alignedread, refseq):
    c = 0 #counter for the read sequence which is incremented with each M/I/S
    r = alignedread.pos #counter for the reference sequence which is incremented with each M/D/S ; POS=Integer=position(-1) of the first base of the reference sequence that gives a match (M)
    tag = 'no'
    read = [alignedread.qname]
    for tu in alignedread.cigar:
        if tu[0] == 0: #0=M=alignment Match (match or mismatch)
            tag = 'yes'
            read_pos_list = range(c,c+tu[1],1)
            for p in read_pos_list: #p+1=position on the read sequence
                s = alignedread.seq[p] #base of the read
                r += 1 #position on the reference sequence
                s_ref = refseq[r-1] #base of the reference
                read.append((s_ref,r,s))
            c += tu[1]
        elif tu[0] == 1: #1=I=Insertion
            c += tu[1]
        elif tu[0] == 2: #2=D=Deletion
            r += tu[1]
        elif tu[0] == 4: #4=S=Soft clipping (match or mismatch or no alignment)
            if tag == 'no':
                r -= tu[1]
            c += tu[1]
            r += tu[1]
    return read

def merger(read1, read2, counter1, counter2):
    end = []
    s1 = read1[0][1]
    e1 = read1[-1][1]
    s2 = read2[0][1]
    e2 = read2[-1][1]
    if s2 > e1: #no overlap between both reads
        read1.extend(read2)
        return read1, counter1, counter2
    elif s1 > e2: #no overlap (this case exists really)
        read2.extend(read1)
        return read2, counter1, counter2
    else:
        read12 = []
        if s1 < s2:
            res = starting(read12, read1, s2)
            read12 = res[0]
            read1 = res[1]
            e1 = read1[-1][1] #if read1 is shorter=> e1 is different!
        elif s2 < s1:
            res = starting(read12, read2, s1)
            read12 = res[0]
            read2 = res[1]
            e2 = read2[-1][1]
        if e1 < e2:
            res = ending(end, read2, e1)
            end = res[0]
            read2 = res[1]
        elif e2 < e1:
            res = ending(end, read1, e2)
            end = res[0]
            read1 = res[1]
        res = overlap(read12, read1, read2, counter1, counter2)
        read12 = res[0]
        counter1 = res[1]
        counter2 = res[2]
        if end != []:
            read12.extend(end)
        return read12, counter1, counter2

def starting(start, read, pos):
    """
    the case where pos does not exist for the analyzed read is possible (point deletion in the read sequence corresponding to the start position of the other read). Thus, tu[1] == pos is never true !
    """
    c = -1
    for tu in read:
        c += 1
        if tu[1] < pos:
            start.append(tu)
        elif tu[1] >= pos:
            del read[:c]
            break
    return start, read

def ending(end, read, pos):
    """
    the case where pos does not exist for the analyzed read is possible (point deletion in the read sequence corresponding to the end position of the other read). Thus, tu[1] == pos is never true !
    """
    c = -1
    i = 'None'
    for tu in read:
        c += 1
        if tu[1] > pos and i == 'None':
            i = c
            end.append(tu)
        elif tu[1] > pos and i != 'None':
            end.append(tu)
    del read[i:]
    return end, read
                                                
def overlap(body, read1, read2, c1, c2):
    """
    we select only identical bases between both pairend reads in the area where reads overlap
    """
    dict2 = {}
    for tu2 in read2:
        dict2[tu2[1]] = tu2[2] #key is reference position ; value is read base
    for tu1 in read1:
        if dict2.has_key(tu1[1]) == 1:
            if tu1[2] == dict2[tu1[1]]: #read1 base == read2 base (at the same ref position)
                body.append(tu1)
                c2 +=1
            else: #read1 base != read2 base, thus sequencing error!
                c1 += 1
    return body, c1, c2

def list_to_array(read, start, end):
    """
        Convert list of tuples into position array and base array (this will discard data that is outside the 'start' and 'end' variables
    """
    code_dict = {'A':0, 'C':1, 'G':2, 'T':3}
    array1 = zeros(len(read), dtype=int16) #positions
    array2 = zeros(len(read), dtype=int8) #base
    
    n=-1 # counter current read
    for tu in read:
        if tu[1]>=start and tu[2] in code_dict:
            if tu[1]<end:
                n+=1
                array1[n]=tu[1]
                array2[n]=code_dict[tu[2]]

    array1.resize(n+1)
    array2.resize(n+1)
    return array1, array2

@autojit
def analyze1d(pos, base, results1d, offset):
    code_1d = arange(4)
    i = 0
    
    for i in range(0, len(pos)):
            pos1=pos[i]
            rb1=base[i]
            
            results1d[(pos1-offset,code_1d[rb1])]+=1
    return results1d

@autojit
def analyze2d(pos, base, results2d, offset):
    code_2d = arange(16).reshape(4,4)
    i, j = 0,0
    
    for i in range(0, len(pos)):
        for j in range(i, len(pos)):
                pos1=pos[i]
                pos2=pos[j]
                
                rb1=base[i]
                rb2=base[j]
                
                results2d[(pos1-offset,pos2-offset,code_2d[rb1,rb2])]+=1
    return results2d

@autojit
def analyze3d(pos, base, results3d, offset):
    code_3d = arange(64).reshape(4,4,4)
    i, j, k = 0,0,0

    for i in range(0, len(pos)):
        for j in range(i, len(pos)):
            for k in range(j, len(pos)):
                pos1=pos[i]
                pos2=pos[j]
                pos3=pos[k]
                
                rb1=base[i]
                rb2=base[j]
                rb3=base[k]
                
                results3d[(pos1-offset,pos2-offset,pos3-offset,code_3d[rb1,rb2,rb3])]+=1
    return results3d

flag = argv[1]
file_path1 = argv[2]
file_path2 = argv[3]
refseq = fasta_parser(argv[4]) #refseq is the reference sequence
file_out = argv[5]

try:
    if argv[6]:
        s = argv[6]
except:
    s = ''

res = sam_parser(file_path1, file_path2, file_out, refseq, flag, s)

f1=open(file_out+'_stats.txt','w')
f1.write('File: %s\nTotal number of reads: %s\nNumber of analyzed reads: %s\nNumber of reads without cigar: %s\nNumber of remaining reads: %s\nTotal number of sequencing errors: %s\nTotal number of bases without sequencing error: %s\n' %(file_out,str(res[0]),str(res[1]),str(res[2]),str(res[3]),str(res[4]),str(res[5])))
f1.close()
