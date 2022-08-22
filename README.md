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

**Note**: In the paper method 4 (NS-Volume) and the benchmark Fpocket have a rounding up of the evaluation scores at their advantage (neverthless, not relevant for the conclusions traced by the paper). With the same conditions of all methods the full comparative table is:

| method              | Top1 | Top3 | Top10| LC   | PC   | nPockets |
|---------------------|------|------|------|------|------|----------|
|M1--Point Transformer| 69.1 | 75.9 | 75.9 | 96.4 | 60.4 | 2.1 |
|M2--GNN-Pocket       | 53.4 | 54.6 | 55.4 | 94.7 | 47.5 | 1.9 |
|M3--DeepSurf         | 87.6 | 89.2 | 89.2 | 95.0 | 67.9 | 1.6 |
|M4--NS-Volume        | 59.0 | 76.7 | 83.9 | 88.8 | 74.8 | 11.6 |
| | | | | | | | 
|Fpocket              | 60.2 | 75.1 | 84.7 | 92.5 | 64.7 | 8.9 |
    
    


### NOTE
The full database and PQR structure files of the contest is provided in https://github.com/concept-lab/shrec22_proteinLigandBenchmark.git

### Full paper

https://arxiv.org/pdf/2206.06035.pdf

### Cite
(*) L. Gagliardi et al, SHREC 2022: Protein-ligand binding site recognition, *Computers & Graphics* https://doi.org/10.1016/j.cag.2022.07.005 (2022)
