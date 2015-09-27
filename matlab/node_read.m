function G = node_read(filename)

f = fopen(filename,'r');

A = textscan(f, '%f %f','CommentStyle','#');
A = [A{1} , A{2}];
x = A(:,2) + 1;
y = A(:,1) + 1;
size = max(max(x),max(y));
G = sparse(x, y, true, size, size);

fclose(f);