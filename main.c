#include <stdio.h>
//#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
//#include <time.h>

//#define NTHREADS 6
#define MILLION 1000000
#define NODES_FILENAME "nodes.txt"
#define RESULTS_FILENAME "result.txt"

#define max(a,b) \
   ({ typeof (a) _a = (a); \
       typeof (b) _b = (b); \
     _a > _b ? _a : _b; })

enum conf_errors {
    E_FILE_ERROR = 200,
    E_MALLOC_ERROR
};

// must fit 10^6 elements
#if UINT_MAX >= MILLION
typedef unsigned int node_id;
#else
#include <stdint.h>
typedef uint32_t node_id;
#endif
typedef node_id **graph;

void calculate_gen(void);
void print_nodes(node_id **array, node_id *sizes, node_id size);
void read_from_file(const char *filename);
void print_gen(void);
void init_prob(void);

graph L = NULL;
node_id *n_inbound = NULL;
node_id *n_outbound = NULL;
node_id *no_outbounds = NULL;
node_id N = 0;
node_id size_no_out = 0;

void read_from_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) exit(E_FILE_ERROR);
    fprintf(stderr, "opened %s\n", filename);

    char *line = NULL;
    size_t len = 0;
    // must initialize first element because idx = 0 is never larger than N.
    L = malloc(sizeof(*L));
    n_inbound = malloc(sizeof(node_id));
    n_outbound = malloc(sizeof(node_id));
    L[0] = NULL;
    n_inbound[0] = 0;

    while (getline(&line, &len, fp) != -1) {
        // skip comments in nodes file.
        if (line[0] == '#') continue;
        // assuming that the first token is the node index
        char *token = strtok(line, " ");
        node_id target = (unsigned int) atol(token);
        // assuming one pair for each line
        token = strtok(NULL, " ");
        node_id idx = (unsigned int) atol(token);

        node_id biggest = max(idx, target);
        if (biggest > N) {
            L = realloc(L, (biggest + 1) * sizeof(*L));
            n_inbound = realloc(n_inbound, (biggest + 1) * sizeof(*n_inbound));
            n_outbound = realloc(n_outbound, (biggest + 1) * sizeof(*n_outbound));
            if (L == NULL || n_inbound == NULL) exit(E_MALLOC_ERROR);

            for (node_id i = N + 1; i <= biggest; i++) {
                // initialize new array elements
                L[i] = NULL;
                n_inbound[i] = 0;
                n_outbound[i] = 0;
            }
            N = biggest;
        }
        n_inbound[idx]++;
        n_outbound[target]++;
        L[idx] = realloc(L[idx], (n_inbound[idx]) * sizeof(node_id));
        if (L[idx] == NULL) exit(E_MALLOC_ERROR);

        L[idx][n_inbound[idx] - 1] = target;
    }

    fclose(fp);
    N++;
}

void print_nodes(node_id **array, node_id *sizes, node_id size) {
    for (unsigned int i = 0; i < size; i++) {
        printf("%u<- ", i);
        for (unsigned int j = 0; j < sizes[i]; j++) {
            printf("%u ", array[i][j]);
        }
        printf("\n%u incoming %u outbound\n", n_inbound[i], n_outbound[i]);
    }
}

int main(void) {

    read_from_file(NODES_FILENAME);
    print_nodes(L, n_inbound, N);

    fprintf(stderr, "Read %ux%u graph\n", N, N);
    return EXIT_SUCCESS;
}
