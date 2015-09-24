f = fopen('nodes.txt','w');
[n1,n2] = size(G);
assert(n1 == n2);
n = n1;

for i=1:n
    for j=1:n
        if G(i,j)
            fprintf(f, '%d %d\n', j-1, i-1);
        end
    end
end

fclose(f);