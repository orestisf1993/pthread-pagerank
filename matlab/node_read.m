function G = node_read(filename)

f = fopen(filename,'r');

A = fscanf(f, '%d %d\n', [2 Inf]);
A = A';
x = A(:,2) + 1;
y = A(:,1) + 1;
size = max(max(x),max(y));
G = sparse(x, y, true, size, size);

fclose(f);