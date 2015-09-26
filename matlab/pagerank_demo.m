function pagerank_demo(nodes_filename, results_filename)

G = node_read(nodes_filename);
[~, n] = size(G);

% Use the power method to compute the eigenvector that correspond to the
% largest eigenvalue (Ranks)
[x,cnt] = pagerankpow(G);
x_c = get_results(results_filename, n);

% plot the ranks
figure
bar(x)
title('Page Ranks Matlab')
print -depsc2 plotm.eps

figure
bar(x_c)
title('Page Ranks C')
print -depsc2 plotc.eps

error = sum((x-x_c).^2)/size(G,1);
fprintf('The square error is %f \n',error);
fprintf('The sum of the matlab results is %f \n', sum(x));
fprintf('the sum of the c results is %f \n', sum(x_c));
fprintf('MATLAB converges in %d iterations\n', cnt);
  