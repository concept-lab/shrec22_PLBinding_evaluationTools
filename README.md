# shrec22_PLBinding_evaluationTools
 Evalutaion tools of the Shrec 2022  contest on protein-ligand binding site recognition

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

### NOTE
The full database and PQR structure files of the contest is provided in https://github.com/concept-lab/shrec22_proteinLigandBenchmark.git

### Cite
(*) L. Gagliardi et al, SHREC 2022: Protein-ligand binding site recognition, *Computers & Graphics* [doi to be added] (2022
