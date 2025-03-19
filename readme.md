Algorithms for the 0-1 Knapsack Problem
=======================================
This software implements several algorithms for the 0-1 knapsack problem.

Example on how to run the software:
`OR_knapsack --data="KP_10_uncorrelated.txt" --algorithm="BB" --verbose`

Parameters:
* `--algorithm`  The choice of algorithm. Possibilities:
  + "CE": complete enumeration
  + "IP": an integer programming model solved with SCIP 
  + "BB": a standard branch-and-bound approach
  + "PI": the branch-and-bound approach of Pisinger
  + "DP1": dynamic programming with a stage for every item
  + "DP2": dynamic programming with a stage for every possible residual capacity
* `--data`       Name of the file containing the problem data
* `--verbose`        Explain the various steps of the algorithm
* `--help`         Help on how to use the application