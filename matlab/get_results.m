function P = get_results(results_filepath)

f = fopen(results_filepath,'r');
P = fscanf(f,'%f',[1 Inf]);
P = P';
