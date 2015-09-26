% A simple demo of google's pagerank algorithm.
% It uses the pagerank implementation provided 
% by Cleve Moler and MathWorks
%
% author: Nikos Sismanis
% date: Jan 2014


% create a small data set of web pages
% [U, G] = surfer('http://www.amazon.com/', 100);

close all
G = node_read('nodes.txt');

% Eliminate any self-referential links
G = G - diag(diag(G));


% Use the power method to compute the eigenvector that correspond to the
% largest eigenvalue (Ranks)
[x,cnt] = pagerankpow(G);
x_c = get_results('../bin/result.txt');


% plot the ranks
figure
bar(x)
title('Page Ranks Matlab')

figure
bar(x_c)
title('Page Ranks C')

error = sum((x-x_c).^2)/size(G,1);
fprintf('The square error is this %f \n',error);
fprintf('The sum of the matlab results is %f \n' ,sum(x));
fprintf('the sum of the c results is %f \n' ,sum(x_c));
  