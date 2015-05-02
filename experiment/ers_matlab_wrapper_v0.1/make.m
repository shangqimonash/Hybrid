% make erc matlab wrapper
% Ming-Yu Liu 04/12/2010
clear all;
clc;
restoredefaultpath;

mex -v -c /Users/yuan/ers_matlab_wrapper_v0.1/ERS/MERCCInput.cpp
mex -v -c /Users/yuan/ers_matlab_wrapper_v0.1/ERS/MERCOutput.cpp
mex -v -c /Users/yuan/ers_matlab_wrapper_v0.1/ERS/MERCDisjointSet.cpp
mex -v -c /Users/yuan/ers_matlab_wrapper_v0.1/ERS/MERCFunctions.cpp
mex -v -c /Users/yuan/ers_matlab_wrapper_v0.1/ERS/MERCLazyGreedy.cpp
mex /Users/yuan/ers_matlab_wrapper_v0.1/ERS/mex_ers.cpp MERCCInput.o* MERCOutput.o* MERCDisjointSet.o* MERCFunctions.o* MERCLazyGreedy.o*

delete *.o*

