function G = node_read(filename)

f = fopen(filename,'r');

A = fscanf(f, '%d %d\n', [2 Inf]);
A = A';
x = A(:,1) + 1;
y = A(:,2) + 1;
G = sparse(x, y, true, length(x), length(y));

fclose(f);