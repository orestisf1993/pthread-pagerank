function P = get_results(results_filepath, N)

f = fopen(results_filepath);
P = fread(f, N, 'float');
fclose(f);
