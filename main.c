#include <stdio.h>
//#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

//#define NTHREADS 6
#define MILLION 1000000

enum conf_errors {
    E_SUCCESS = 0,
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

void print_nodes(node_id **array, size_t *sizes, size_t N);
void read_from_file(const char *filename, graph *_array, size_t **_sizes, size_t *total_lines);

void read_from_file(const char *filename, graph *_array, size_t **_sizes, size_t *total_lines) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) exit(E_FILE_ERROR);
    fprintf(stderr, "opened %s\n", filename);

    char *line = NULL;
    size_t len = 0;
    node_id idx;
    node_id largest_idx = 0;
    // must initialize first element because idx = 0 is never > largest_idx
    graph array = malloc(sizeof(*array));
    size_t *sizes = malloc(sizeof(size_t));
    array[0] = NULL;
    sizes[0] = 0;

    while (getline(&line, &len, fp) != -1) {
        // skip comments
        if (line[0] == '#') continue;
        // assuming that the first token is the node index
        char *token = strtok(line, " ");
        idx = (unsigned int) atol(token);

        if (idx > largest_idx) {
            array = realloc(array, (idx + 1) * sizeof(*array));
            sizes = realloc(sizes, (idx + 1) * sizeof(*sizes));
            if (array == NULL || sizes == NULL) exit(E_MALLOC_ERROR);

            for (node_id i = largest_idx + 1; i <= idx; i++) {
                // initialize new array elements
                array[i] = NULL;
                sizes[i] = 0;
            }

            largest_idx = idx;
        }

        array[idx] = realloc(array[idx], (++sizes[idx]) * sizeof(node_id));
        if (array[idx] == NULL) exit(E_MALLOC_ERROR);

        // assuming that only one
        token = strtok(NULL, " ");
        array[idx][sizes[idx] - 1] = (unsigned int) atol(token);
    }

    fclose(fp);
    *_array = array;
    *_sizes = sizes;
    *total_lines = largest_idx + 1;
}

void print_nodes(node_id **array, size_t *sizes, size_t N) {
    for (unsigned int i = 0; i < N; i++) {
        printf("%u-> ", i);
        for (unsigned int j = 0; j < sizes[i]; j++) {
            printf("%u ", array[i][j]);
        }
        printf("\n");
    }
}

int main(void) {
    node_id **array = NULL;
    size_t *sizes = NULL;
    size_t N;
    read_from_file("nodes.txt", &array, &sizes, &N);
    print_nodes(array, sizes, N);

    return 0;
}
