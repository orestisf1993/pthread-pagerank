function [x,cnt] = pagerankpow(G)
% PAGERANKPOW  PageRank by power method with no matrix operations.
% x = pagerankpow(G) is the PageRank of the graph G.
% [x,cnt] = pagerankpow(G) also counts the number of iterations.
% There are no matrix operations.  Only the link structure
% of G is used with the power method.

%   Copyright 2012 Cleve Moler and The MathWorks, Inc.

% Link structure

[~,n] = size(G);
for j = 1:n
   L{j} = find(G(:,j));%Find all indexes of  the non zero values of G for each node
   c(j) = length(L{j});% the length of L(j) is the number of nodes it is connected to
end

% Power method

p = .85;
delta = (1-p)/n;
x = ones(n,1)/n;
z = x+0.1;
cnt = 0;
% This condition is never satisfied for big G (eg 1M x 1M).
% True if final iteration had a minimal effect on  the values of the p vector
while any(abs(x-z) > 0.000001) 
   z = x;
   x = zeros(n,1);
   for j = 1:n
      if c(j) == 0
         x = x + z(j)/n;
      else
         x(L{j}) = x(L{j}) + z(j)/c(j);
      end
   end
   x = p*x + delta;
   cnt = cnt+1;
end
