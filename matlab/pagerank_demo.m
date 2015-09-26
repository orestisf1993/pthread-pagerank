% A simple demo of google's pagerank algorithm.
% It uses the pagerank implementation provided 
% by Cleve Moler and MathWorks
%
% author: Nikos Sismanis
% date: Jan 2014


% create a small data set of web pages
% [U, G] = surfer('http://www.amazon.com/', 100);


G = node_read('nodes.txt');

% Eliminate any self-referential links
G = G - diag(diag(G));


% Use the power method to compute the eigenvector that correspond to the
% largest eigenvalue (Ranks)
[x,cnt] = pagerankpow(G);


% plot the ranks
figure
bar(x)
title('Page Ranks')


