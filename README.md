# MIMEAnTo

Compiling MIMEAnTo
---------------------
To compile MIMEAnTo a compiler capable of c++20 is required.

Download or clone this repository for MIMEAnTo.
Go to the directory where you want to have your program files.
Then call:

```
cmake  /path/to/MIMEAnTo/source
make
```


Running MIMEAnTo
-------------------

The program is called from the command line with
```
MIMAnTo result_directory [filename_suffix]
```

and requires the following parameters


| parameter       | type          | description  |
| :---  |:---:| :----------------|
| result_directory         | (string)      |   directory where the resulting tables are written to, has to contain the config file named project.txt |
| filename_suffix          | (string)      |   (optional) suffix for the table names, for example when using different options |

config file with the name *project.txt* has to contain the following parameter, separated by tab:

```
refSeqFile  </path/to/reference/sequence/ref.fasta>
dataDir </directory/containing/count/files>
alpha <significance_level>
minimumNrCalls  <coverage>
minNumberEstimatableKds <minEstimates>
minSignal2NoiseStrength <s2n_threshold>
minMutRate  <exponent_mutRate>
seqBegin	<seqStart>
seqEnd	<seqEnd>
percOfMaxCov	<percent_of_max_coverage>
joinErrors	false
signThreshold	<significance_value_threshold>
selected <selected_wt_barcode_1>	<sample_name_1>	0
nonselected	<unselected_wt_barcode_1>	<sample_name_1>	0
selected	<selected_barcode_1>	<sample_name_1>	<sample_id_1>
nonselected	<unselected_barcode_1>	<sample_name_1>	<sample_id_1>
[selected <selected_wt_barcode_2>	<sample_name_2>	0
nonselected	<unselected_wt_barcode_2>	<sample_name_2>	0
selected	<selected_barcode_2>	<sample_name_2>	<sample_id_2>
nonselected	<unselected_barcode_2>	<sample_name_2>	<sample_id_2>]
```

The parameter are explained in the former MIMEmanual.pdf, but will be updated here in the README soon. 

A note regarding the sample files: 
The dataDir has to contain the countfiles in the directories 1d and 2d. The count files contain the nucleotde (co-)occurrences of mapped reads, which can be inferred with the tool [sam2coutns](https://github.com/maureensmith/sam2counts).
The 1d and 2d count file for the respective sample is named with the respective id/barcode:

```
/path/to/counts
+-- 1d
|   +-- 1.txt
|   +-- 2.txt
|   +-- 3.txt
|   +-- 4.txt
+-- 2d
|   +-- 1.txt
|   +-- 2.txt
|   +-- 3.txt
|   +-- 4.txt
```

A sample set consists of 4 sample files, the selected and unselected sample file and the selected and unselected wild type sample for error correction.
Each row of the this quadruple contains 

* the specifier "selected" or "nonselected"
* the id/barcode of the respective filename 
* the name for the quadruple set
* an id for the sampleset: The wild type samples always have "0", the mutation libraries have an id for this, for example starting with 1. 

Arbitrarily many sample sets can be added. They can also use the same wild type samples, by giving the same id.

An example entry for two sample sets could be

```
selected  1 sampleSet1	0
nonselected	2	sampleSet1	0
selected	3	sampleSet1	1
nonselected	4	sampleSet1	1
selected  5 sampleSet1	0
nonselected	6	sampleSet1	0
selected	7	sampleSet1	2
nonselected	8	sampleSet1	2
```

The ouput are two tables containing 1. the posisition wise maxKds and 2. the Kds for all 3 mutations, both with additional information, also described in the manual.
The according plots can be generated with the R Script here: [MIMEplot](https://github.com/maureensmith/MIMEplot)
