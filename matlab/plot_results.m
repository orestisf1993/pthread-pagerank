clear all
close all
clear

nFolder = '../nodeFiles/';
rFolder = '../gen_results/';

nwiki = [nFolder, 'Wiki-Vote_8298.txt'];
rwiki = [rFolder ,'8298_4_results.bin'];
nsimple = [nFolder,'nodes.txt'];
rsimple = [rFolder,'100_4_results.bin'];

pagerank_demo(nwiki,rwiki,'Wiki-vote');
pagerank_demo(nsimple,rsimple);