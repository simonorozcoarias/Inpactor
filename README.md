# Inpactor
Integrated and Parallel Analyzer and Classifier of LTR Retrotransposons for Large Genomes

# General description: 

Inpactor was developed using MPI standard, in C language. It is composed by 4 modules.

As previous requirement Inpactor needs the output of LTR_STRUC or repet (TEdenovo package), but also Inpactor can be used with a fasta file which contained contigs, or the whole genome(view software requirement section).

In a configuration file is possible to define general information such as input information (folder with LTR_STRUC output, repet output file or fasta file), result directory, verbose mode and clean mode at the end of the execution. In addition each module requires that different parameters must be indicated in the configuration file. Preprocessing, Classification and domain extraction modules can run independently, in contrast Tree creation and insertion time Module needs to be executed with Domain extraction module.

The first module executed in the process is the preprocessing. The objective here is to group together all information from the input information into one tabular text file, to organize the information. Input files can be:
1) LTR_STRUC output: Inpactor uses two LTR_STRUC files: (i) in report file we got features such as LTR Identity, primer binding site (PBS), PolyPurine Tract (PPT), length, Active size, Longest Open Reading Frame (ORF), Target Site Duplication (TSD), Long Terminal Repeat (LTR) A length, LTR B length, and strand; (ii) Inpactor uses Fasta file to extract important sequences like LTR A and B using Seqret and Extractseq tools from Emboss and the sequence of the full element.
2) repet output (TEdenovo Package) in fasta format: Inpactor extracts information such as element length, LTR Identity, Long Terminal Repeat (LTR) A length, LTR A sequence, LTR B length, LTR B sequence and information about domains found. 
3) fasta file: Inpactor can analyze fasta files with contigs and also with whole genome. In this case Inpactor executes LTR-FINDER (in parallel) to find completed elements inside the input file, looking for features such as element length, LTR Identity, PPT, Longest ORF, Long Terminal Repeat (LTR) A length, LTR A sequence, LTR B length, LTR B sequence, Strand and information about domains found.

The second step is the classification module. Using the result file from previous module, a classification was performed as follow: (i) if the element carried at least one principal domain (RT, INT, and RNAseH) with keywords RLC or RLG, the LTR-RT was classified as completed-family element (Copia or Gypsy); (ii) if the element didn’t carry any domain, it was classified as non-autonomous element; (iii) if the element had only a GAG domain or GAG and AP domains, the element was classified as TR-GAG elements. Also a deep classification in families is done with completed elements (Copia or Gypsy).

The third module of Inpactor is the Domain extraction module, in this part of the process, we were interested into the extraction of RT domain sequences from each complete-family element, because this domain is the most conserved and appropriated for phylogenetic analysis. Other domains from the LTR-RT polyprotein might be used alternatively. 

Lastly, Inpactor execute the fourth module, composed by two steps:
Analyzing LTR retrotransposon insertion times.
The insertion times of full-length copies, as defined by a minimum of 80% of nucleotide identity over 100% of the reference element length, were dated.
Phylogenetic tree creation.
Using the protein Fasta file from RT domain extraction module, a multiple alignment was performed using Mafft with –thread option to indicate the number of cores.

# Pre-requisites: 
Inpactor run over linux environments, the software was tested in Centos 6,7.  Following we show a list of the prerequisites of Inpactor installation:

- NCBI-Blast version 2.5.0 (included blastall command) (ftp://ftp.ncbi.nlm.nih.gov/blast/executables/blast+/2.5.0/)
- Emboss version 6.6.0 (ftp://emboss.open-bio.org/pub/EMBOSS/)
- Wise2 version 2.4.0 (http://www.ebi.ac.uk/~birney/wise2/)
- OpenMPI version 1.8.8 (https://www.open-mpi.org/software/ompi/v1.8/)
- Censor version 4.2.29 (http://www.girinst.org/downloads/software/censor/) **Only used for re-classification (80-80-80-rule option in configure file)
- Mafft version 7.305 (http://mafft.cbrc.jp/alignment/software/)
- LTR-FINDER version 1.0.5 (https://github.com/xzhub/LTR_Finder)

# Installation:
To install almost all the pre-requisites is it recommended to use a conda environment. First clone the repo:
```
git clone https://github.com/simonorozcoarias/Inpactor.git
```
then, install the environment:
```
conda env create -f Inpactor1.yml
```
the command above will install the following pre-requesities for you: NCBI-Blast, Emboss, Wise2, Mafft, and LTR-FINDER. You need to install manually the OpenMPI software as following:
```
conda activate Inpactor1
wget https://download.open-mpi.org/release/open-mpi/v1.8/openmpi-1.8.8.tar.gz
tar xvf openmpi-1.8.8.tar.gz
cd openmpi-1.8.8
mkdir build
./configure --prefix=/path/to/openmpi_downloaded_folder/build
make
make install
```
Remeber to change the --prefix path with your own path. For example: /home/user/Downloads/openmpi-1.8.8/build. Remember also to include the "build" folder in the path.

Then, you need to add the binaries and libraries to your environment variables:
```
export PATH=$PATH:/path/to/openmpi_downloaded_folder/build/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/openmpi_downloaded_folder/build/lib
```
**Remember please to change "/path/to/openmpi_downloaded_folder/" with your own path.**

Now, install Censor software (for the optional step of 80-80-80 re-classification):
```
wget https://www.girinst.org/downloads/software/censor/protected/censor-4.2.22.tar.gz
tar xvf censor-4.2.22.tar.gz
cd censor-4.2.22
mkdir build
./configure --prefix=/path/to/censor_downloaded_folder/build
make
make install
```
Remeber to change the --prefix path with your own path. For example: /home/user/Downloads/censor-4.2.22/build. Remember also to include the "build" folder in the path.

Then, you need to add the binaries and libraries to your environment variables:
```
export PATH=$PATH:/path/to/censor_downloaded_folder/build/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/censor_downloaded_folder/build/lib
```
**Remember please to change "/path/to/censor_downloaded_folder/" with your own path.**

After install all pre-requisites, you must compile Inpactor using: 
```
mpicc Inpactor.c -o Inpactor
```
This step produces an executable, which will be used in next sections. 

Setting up the process of analysis:
Inpactor need a configuration file where is defined the parameters which will be used in the execution of the analysis. An example of configuration file is given in this repository.

# Executing analysis with Inpactor:

This command execute the process of analysis. Is very important to consider that all software listed in the prerequisites section must be load in the path of the system. This step uses Inpactor’s executable file generated in installation section. Is highly recommended if you have less sequences or elements in your input than CPUs in your system, to use the same number of processes than sequences or elements in your input, but if you have more sequences or elements than CPUs, is better to use the same number of processes than CPUs in your system.

**Be sure of formating your database using makeblastdb command before to run Inpactor.**

If you want to use a custom database, you must format sequence names to have the following key words:

>sequence_name#SuperfamilyKey+Lineage

Where SuperfamilyKey must be RLC for Ty1-Copia or RLG for TY3-Gypsy. Also sequence names in your files must not to have any special characters such as "#", " ", ";" "{", "}", parenthesis, ":" among others. Lineage can be change for any classification system.

**Please remember to activate the conda environment if you are using it (with the command: conda activate Inpactor1), and also to have the OpenMPI and censor binaries added to the PATH and LD_LIBRARY_PATH variables (see Installation section).**

Then, you can execute Inpactor as following:
```
mpirun -np “number of process (depend of the number of cores available in your system)” Inpactor “configuration file”
```

**NOTE: If you cannot get the censor (because it is now private) you only need to set the 80-80-80-rule option in the configuration file to false.**

# License:
Inpactor is licensed under GNU GLP v3
(https://www.gnu.org/licenses/gpl-3.0.en.html)

# Authors:
IRD France: (http://www.ird.fr/) 

Romain Guyot 

Autonomous University of Manizales (https://www.autonoma.edu.co/)

Simón Orozco Arias

Reinel Tabares Soto

University of Caldas (http://ucaldas.edu.co/)

Diego Hernando Ceballos López

Andrea garavito

For more information please write to: simon.orozco.arias@gmail.com

# Citation:
Orozco-Arias, S.; Liu, J.; Tabares-Soto, R.; Ceballos, D.; Silva Domingues, D.; Garavito, A.; Ming, R.; Guyot, R. Inpactor, Integrated and Parallel Analyzer and Classifier of LTR Retrotransposons and Its Application for Pineapple LTR Retrotransposons Diversity and Dynamics. Biology 2018, 7, 32.
