# shrec22_PLBinding_evaluationTools
 Evalutaion tools of the Shrec 2022  contest on protein-ligand binding site recognition
 
## Requirements
The script has been tested on Ubuntu 20.04.4 LTS and macOS Catalina 10.15.7.

Python3 must be installed

### Python non-builtin modules
- **numpy**

If not installed: *pip3 install numpy*

### C shared library installation
(recommended for OS)

Move in the *installLIB* folder and type ./install_script.
This will compile the shared C library and move it in the main folder under the name: *libCfunc.so*


## Usage 
For evaluating putative pockets in PQR or as a boolean map for the vertices of the structure OFF format (TXT file):

If OFF format, in the working directory the "testData" folder must be present. This folder contains the structures in OFF format.
It can be downloaded from https://github.com/concept-lab/shrec22_proteinLigandBenchmark.git

python3 evaluate.py \<directoryName containing participant results\>

Change in the script the fields 
pCoverageTH = 0.2
lCoverageTH = 0.5
to change the metric's threshold for the evaluation of a putative binding site(\*).

### Output of evaluation
*rankStats.txt*: file containing the ranking result (Top1, Top3, Top10 and metrics--LC and PC score as described in\*)

*failureList.txt*: file containing the list of structure-ligands pairs not matched by any of the putative pockets.
### Example
python3 evaluate.py examples/pqr

Evaluates the method M3 - DeepSurf to reproduce the 3rd line of Table 1 in \*

### Generation of complete set of results for "SHREC 2022: Protein-ligand binding site recognition" benchark

Download from https://github.com/concept-lab/shrec22_proteinLigandBenchmark.git the *participantResults* folder, and run for each participant the evaluation script as described above in the example.


### NOTE
The full database and PQR structure files of the contest is provided in https://github.com/concept-lab/shrec22_proteinLigandBenchmark.git

### Full paper

https://arxiv.org/pdf/2206.06035.pdf

### Cite
(*) L. Gagliardi et al, SHREC 2022: Protein-ligand binding site recognition, *Computers & Graphics* https://doi.org/10.1016/j.cag.2022.07.005 (2022)
