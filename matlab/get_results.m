function P = get_results(results_filepath)

f = fopen(results_filepath,'r');
f
P = fscanf(f,'%f',[1 Inf]);
P = P';
