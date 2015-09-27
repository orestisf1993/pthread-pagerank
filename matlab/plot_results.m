clear all
close all
clear

nFolder = '../nodeFiles/';
rFolder = '../gen_results/';

nwiki = [nFolder, 'Wiki-Vote_8298.txt'];
rwiki = [rFolder ,'8298_4_results.bin'];
nstack = [nFolder,'stackOverFlow_100.txt'];
rstack = [rFolder,'100_4_results.bin'];
nshlashdot = [nFolder,'Slashdot0811_77360.txt'];
rshlashdot = [rFolder,'77360_4_results.bin'];


% pagerank_demo(nwiki,rwiki,'WikiVote-8298');
% pagerank_demo(nstack,rstack,'StackOveFlow-100');
pagerank_demo(nshlashdot,rshlashdot,'SlashDot-77360');