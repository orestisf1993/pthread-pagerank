#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <getopt.h>
#include "utils.h"

graph L = NULL;
node_id *n_inbound = NULL;
node_id *n_outbound = NULL;
node_id *no_outbounds = NULL;
node_id N = 0;
node_id size_no_out = 0;
node_id size_no_in = 0;
uint64_t n_vertices = 0;

void read_from_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) exit(E_FILE_ERROR);
    #ifdef DEBUG
    fprintf(stderr, "opened %s\n", filename);
    #endif

    char *line = NULL;
    size_t len = 0;
    // must initialize first element because idx = 0 is never larger than N.
    L = malloc(sizeof(*L));
    n_inbound = malloc(sizeof(node_id));
    n_outbound = malloc(sizeof(node_id));
    L[0] = NULL;
    n_inbound[0] = 0;
    n_outbound[0] = 0;

    while (getline(&line, &len, fp) != -1) {
        // skip comments in nodes file.
        if (line[0] == '#') continue;
        // assuming that the first token is the node index
        char *token = strtok(line, "\t ");
        node_id target = (unsigned int) atol(token);
        // assuming one pair for each line
        token = strtok(NULL, "\t ");
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
        n_vertices++;
    }

    fclose(fp);
    free(line);
    N++;
}

void print_gen(unsigned int nthreads) {
    char size[15];
    sprintf(size, "%d_%d", N, nthreads);
    char *filename = malloc(strlen(size) + strlen("_results.bin") + 1);
    strcpy(filename, size);
    strcat(filename, "_results.bin");

    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) exit(E_FILE_ERROR);
    fwrite(P, sizeof(prob_type), N, fp);

    free(filename);
    fclose(fp);
}

void print_usage(char **argv) {
    printf("usage: %s [options]\n\n"
                    "    -h, --help: This help.\n"
                   "    -n, --nodes-file=FILENAME: File to use for input graph. Default is nodes.txt\n"
                    "    -t, --nthreads=NUM: Number of threads used to run pagerank.\n"
                   "    -f, --custom-f: Binary file to use for initial P. Default is uniform distribution.\n"
                   "    -e, --custom-e: Binary file to use for initial E. Default is uniform distribution.\n"
                    "    -s, --smart-split: Split work between threads based on workload, not nodes.\n",
            argv[0]);
}

void save_res(int size, int threads, unsigned int final_gen, double time) {
    FILE *fp = fopen(RESULTS_FILENAME, "a");  // Append the time results in the end
    if (fp == NULL) exit(E_FILE_ERROR);
    fprintf(fp, "%d %d %u %g\n", size, threads, final_gen, time);
    fclose(fp);
}

prob_type *P;
prob_type *E;
prob_type *P_new;
prob_type constant_add;

void init_prob(char *custom_F, char *custom_E) {
    P = malloc(N * sizeof(prob_type));
    P_new = malloc(N * sizeof(prob_type));

    if (custom_F != NULL) {
        FILE *fp = fopen(custom_F, "r");
        if (fp == NULL) exit(E_FILE_ERROR);
        fseek(fp, SEEK_SET, 0);
        fread(P, sizeof(prob_type), N, fp);
        fclose(fp);
        constant_add = calculate_const_add();
    }
    else {
        constant_add = (prob_type) size_no_out / (prob_type) (N) / (prob_type) N;
    }

    if (custom_E != NULL) {
        E = malloc(N * sizeof(prob_type));
        FILE *fp = fopen(custom_E, "r");
        if (fp == NULL) exit(E_FILE_ERROR);
        fseek(fp, SEEK_SET, 0);
        fread(E, sizeof(prob_type), N, fp);
        fclose(fp);
    }
    else {
        E = NULL;
    }

    for (node_id i = 0; i < N; i++) {
        // uniform distribution
        if (custom_F == NULL) P[i] = 1 / (prob_type) N;

        // Any node with no out links is linked to all nodes to emulate the matlab script.
        if (!n_outbound[i]) {
            no_outbounds = realloc(no_outbounds, (++size_no_out) * sizeof(node_id));
            no_outbounds[size_no_out - 1] = i;
        }
        if (!n_inbound[i]) size_no_in++;
    }
}

main_options argument_parser(int argc, char **argv) {
    main_options result;
    result.filename = DEFAULT_NODES_FILENAME;
    result.smart_split = false;
    result.custom_F = NULL;
    result.custom_E = NULL;
    result.nthreads = DEFAULT_NTHREADS;

    static struct option long_options[] = {
            {"nodes-file", required_argument, NULL, 'n'},
            {"nthreads", required_argument, NULL, 't'},
            {"custom-f", required_argument, NULL, 'f'},
            {"custom-e", required_argument, NULL, 'e'},
            {"smart-split", no_argument, NULL, 's'},
            {"help", no_argument, NULL, 'h'},
            {NULL, 0, NULL, 0}
    };

    while (1) {
        int opt = getopt_long(argc, argv, "n:t:hsf:e:", long_options, NULL);
        if (opt == -1) break;
        switch (opt) {
            case 'n':
                result.filename = optarg;
                break;
            case 't':
                result.nthreads = (unsigned int) strtoul(optarg, NULL, 0);
                break;
            case 'h':
                print_usage(argv);
                exit(EXIT_SUCCESS);
            case 's':
                result.smart_split = true;
                break;
            case 'f':
                result.custom_F = optarg;
                break;
            case 'e':
                result.custom_E = optarg;
                break;
            default:
                print_usage(argv);
                exit(E_UNRECOGNISED_ARGUMENT);
        }
    }
    return result;
}