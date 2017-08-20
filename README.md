# Parallan
Parallel Analyzer and Classifier of LTR Retrotransposons for Large Genomes

# General description: 

Parallan was developed using MPI standard, in C language. It is composed by 4 modules.

As previous requirement Parallan needs the output of LTR_STRUC or repet (TEdenovo package)(view software requirement section).

In a configuration file is possible to define general information such as input information (folder with LTR_STRUC output, repet output file or fasta file), result directory, verbose mode and clean mode at the end of the execution. In addition each module requires that different parameters must be indicated in the configuration file. Preprocessing, Classification and domain extraction modules can run independently, in contrast Tree creation and insertion time Module needs to be executed with Domain extraction module.

The first module executed in the process is the preprocessing. The objective here is to group together all information from the input information into one tabular text file, to organize the information. Input files can be:
1) LTR_STRUC output: Parallan uses two LTR_STRUC files: (i) in report file we got features such as LTR Identity, primer binding site (PBS), PolyPurine Tract (PPT), length, Active size, Longest Open Reading Frame (ORF), Target Site Duplication (TSD), Long Terminal Repeat (LTR) A length, LTR B length, and strand; (ii) Parallan uses Fasta file to extract important sequences like LTR A and B using Seqret and Extractseq tools from Emboss and the sequence of the full element.
2) repet otput (TEdenovo Package) in fasta format: Parallan extracts information such as element length, LTR Identity, Long Terminal Repeat (LTR) A length, LTR A sequence, LTR B length, LTR B sequence and information about domains found. 
3) fasta file: Parallan can analyze fasta files with contigs and also with whole genome. In this case Parallan executes LTR-FINDER to find completed elements inside the input file, looking for features such as element length, LTR Identity, PPT, Longest ORF, Long Terminal Repeat (LTR) A length, LTR A sequence, LTR B length, LTR B sequence, Strand and information about domains found.

The second step is the classification module. Using the result file from previous module, a classification was performed as follow: (i) if the element carried at least one principal domain (RT, INT, and RNAseH) with keywords RLC or RLG, the LTR-RT was classified as completed-family element (Copia or Gypsy); (ii) if the element didn’t carry any domain, it was classified as non-autonomous element; (iii) if the element had only a GAG domain or GAG and AP domains, the element was classified as TR-GAG elements. Also a deep classification in families is done with completed elements (Copia or Gypsy).

The third module of Parallan is the Domain extraction module, in this part of the process, we were interested into the extraction of RT domain sequences from each complete-family element, because this domain is the most conserved and appropriated for phylogenetic analysis. Other domains from the LTR-RT polyprotein might be used alternatively. 

Lastly, Parallan execute the fourth module, composed by two steps:
Analyzing LTR retrotransposon insertion times.
The insertion times of full-length copies, as defined by a minimum of 80% of nucleotide identity over 100% of the reference element length, were dated.
Phylogenetic tree creation.
Using the protein Fasta file from RT domain extraction module, a multiple alignment was performed using Mafft with –thread option to indicate the number of cores.

# Prerequisites: 
Parallan run over linux environments, the software was tested in Centos 6,7.  Following we show a list of the prerequisites of Parallan installation:

- NCBI-Blast version 2.5.0 (ftp://ftp.ncbi.nlm.nih.gov/blast/executables/blast+/2.5.0/)
- Emboss version 6.6.0 (ftp://emboss.open-bio.org/pub/EMBOSS/)
- Wise2 version 2.4.0 (http://www.ebi.ac.uk/~birney/wise2/)
- OpenMPI version 1.8.8 (https://www.open-mpi.org/software/ompi/v1.8/)
- Censor version 4.2.29 (http://www.girinst.org/downloads/software/censor/)
- Mafft version 7.305 (http://mafft.cbrc.jp/alignment/software/)
- LTR-FINDER version 1.0.5 (http://mafft.cbrc.jp/alignment/software/)

# Installation:
After install all prerequistes, you must clone the repository of the current version of Parallan using: 

$ git clone https://github.com/simonorozcoarias/Parallan.git, then you might run as following:

$ cd Parallan

$ mpicc parallan.c -o parallan

This step produces an executable, which will be used in next sections. 

Setting up the process of analysis:
Parallan need a configuration file for define the parameters which will be use in the execution of the analysis. An example of configuration file is given in this repository.

# Executing analysis with Parallan:

This command execute the process of analysis. Is very important to consider that all software listed in the prerequisites section must be load in the path of the system. This step uses Parallan’s executable file generated in installation section.

mpirun -np “number of process (depend of the number of cores available in your system)” parallan “configuration file”

# License:
Parallan is licensed under GNU GLP v3
(https://www.gnu.org/licenses/gpl-3.0.en.html)

# Authors:
IRD France: (http://www.ird.fr/) 

Romain Guyot 

Bioinformatics and computational biology center of Colombia: (www.bios.co) 

Simón Orozco Arias

Reinel Tabares Soto

Diego Hernando Ceballos López

For more information please write to: tecnologia@bios.co - contacto@bios.co

