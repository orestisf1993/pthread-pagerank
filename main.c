#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#include <stdint.h>
#include <sys/time.h>
#include <math.h>
//#include <time.h>

#define MAX_GENERATIONS 10000
#define DEFAULT_NTHREADS 4
#define MILLION 1000000
#define MAX_ERROR 0.000001f
#define DEFAULT_NODES_FILENAME "nodes.txt"
#define RESULTS_FILENAME "result.txt"

#define max(a, b) \
   ({ typeof (a) _a = (a); \
       typeof (b) _b = (b); \
     _a > _b ? _a : _b; })

enum conf_errors {
    E_FILE_ERROR = 200,
    E_MALLOC_ERROR,
    E_UNRECOGNISED_ARGUMENT
};

// must fit 10^6 elements
#if UINT_MAX >= MILLION
typedef unsigned int node_id;
#else
#include <stdint.h>
typedef uint32_t node_id;
#endif
typedef node_id **graph;

#ifdef USE_DOUBLE_PRECISION
#define prob_type double
#else
#define prob_type float
#endif

void *calculate_gen(void *_args);
void read_from_file(const char *filename);
void print_gen(void);
void init_prob(void);
void print_usage(char **argv);

graph L = NULL;
node_id *n_inbound = NULL;
node_id *n_outbound = NULL;
node_id *no_outbounds = NULL;
node_id N = 0;
node_id size_no_out = 0;

unsigned int nthreads = DEFAULT_NTHREADS;

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
    }

    fclose(fp);
    N++;
}

prob_type *P;
prob_type *E;
prob_type *P_new;

void init_prob(void) {
    P = malloc(N * sizeof(prob_type));
    P_new = malloc(N * sizeof(prob_type));
    E = malloc(N * sizeof(prob_type));
//    prob_type sumP = 0.0f;
//    prob_type sumE = 0.0f;
//    srand(time(NULL));
//    for (node_id i = 0; i < N; i++){
//        int r = rand();
//        P[i] = (prob_type)r/(prob_type)(RAND_MAX);
//        r = rand();
//        E[i] = (prob_type)r/(prob_type)(RAND_MAX);
//        sumP += P[i];
//        sumE += E[i];
//    }
    for (node_id i = 0; i < N; i++) {
//        P[i] /= sumP;
//        E[i] /= sumE;
        P[i] = E[i] = 1 / (prob_type) N;   // uniform distribution
        // Any node with no out links is linked to all nodes to emulate the matlab script.
        if (!n_outbound[i]) {
            no_outbounds = realloc(no_outbounds, (++size_no_out) * sizeof(node_id));
            no_outbounds[size_no_out - 1] = i;
        }
    }
}

pthread_barrier_t barrier;
typedef struct {
    unsigned int tid;
} parm;

int running = 1;
int *local_terminate_flag;

#define D 0.85f
void *calculate_gen(void *_args) {
    const parm *args = (parm *) _args;
    const node_id chunk = N / nthreads;
    const unsigned int tid = args->tid;
    const node_id start = (node_id) (tid) * chunk;
    const node_id end = start + chunk + (tid == nthreads - 1) * (N % nthreads);
    fprintf(stderr, "tid %u, start %u, end %u\n", tid, start, end);

    // initialize for uniform distribution.
    //TODO: test make it global/shared between threads.
    prob_type constant_add = (prob_type) size_no_out / (prob_type) (N) / (prob_type) N;
    unsigned int gen;
    for (gen = 0; gen < MAX_GENERATIONS ; gen++) {
        local_terminate_flag[tid] = 1;
        for (node_id i = start; i < end; i++) {
            prob_type link_prob = 0;
            for (node_id x = 0; x < n_inbound[i]; x++) {
                const node_id j = L[i][x];
                link_prob += P[j] / n_outbound[j];
            }
            link_prob += constant_add;

            P_new[i] = D * link_prob + (1 - D) * E[i];
            if (local_terminate_flag[tid]) {
                if (fabsf(P_new[i] - P[i]) > MAX_ERROR) {
                    local_terminate_flag[tid] = 0;
                }
            }
        }
        const int res = pthread_barrier_wait(&barrier);
        if (res == PTHREAD_BARRIER_SERIAL_THREAD) {
            //swap P_new with P.
            prob_type *tmp;
            tmp = P;
            P = P_new;
            P_new = tmp;

            running = 0;
            for (unsigned int i = 0; i < nthreads; i++) {
                if (!local_terminate_flag[i]) {
                    running = 1;
                    break;
                }
            }
        }

        pthread_barrier_wait(&barrier); //TODO: is this needed?
        if (!running) break;

        // calculate the constant for links without outbound links.
        constant_add = 0.0f;
        for (node_id x = 0; x < size_no_out; x++) {
            const node_id j = no_outbounds[x];
            constant_add += P[j];
        }
        constant_add /= (prob_type) N;
    }
    pthread_exit(tid ? NULL : (void *) (uintptr_t) gen);
}

void print_gen(void) {
    FILE *fp = fopen(RESULTS_FILENAME, "w");
    for (node_id i = 0; i < N; i++) fprintf(fp, "%f ", P[i]);
    fprintf(fp, "\n");
    fclose(fp);
}

void print_usage(char **argv) {
    fprintf(stderr, "usage: %s [options]\n\n"
                    "    -n, --nodesfile=FILENAME: File to use for input graph.\n"
                    "    -t, --nthreads=NUM: Number of threads used to run pagerank.\n",
            argv[0]);
}

int main(int argc, char **argv) {

    const char *filename = DEFAULT_NODES_FILENAME;

    static struct option long_options[] = {
            {"nodes-file", required_argument, 0, 'n'},
            {"nthreads", required_argument, 0, 't'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
    };

    while (1) {
        int opt = getopt_long(argc, argv, "n:t:h", long_options, NULL);
        if (opt == -1) break;
        switch (opt) {
            case 'n':
                filename = optarg;
                break;
            case 't':
                nthreads = (unsigned int) strtoul(optarg, NULL, 0);
                break;
            case 'h':
                print_usage(argv);
                exit(EXIT_SUCCESS);
            default:
                print_usage(argv);
                exit(E_UNRECOGNISED_ARGUMENT);
        }
    }

    fprintf(stderr, "opening file %s, operating with %u threads\n", filename, nthreads);
    read_from_file(filename);
    init_prob();

    fprintf(stderr, "Read %ux%u graph\n", N, N);
    pthread_barrier_init(&barrier, NULL, nthreads);

    pthread_t *threads = malloc(nthreads * sizeof(pthread_t));
    local_terminate_flag = malloc(nthreads * sizeof(int));
    parm *args = malloc(nthreads * sizeof(parm));

    struct timeval start, end;
    gettimeofday(&start, NULL);

    for (unsigned int i = 0; i < nthreads; i++) {
        args[i].tid = i;
        pthread_create(&threads[i], NULL, calculate_gen, (void *) &args[i]);
    }
    void *final_gen = NULL;
    pthread_join(threads[0], &final_gen);
    for (unsigned int i = 1; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) +
            ((end.tv_usec - start.tv_usec) / 1000000.0);
    fprintf(stderr, "finished on generation %lu after %g sec\n", (uintptr_t) final_gen, elapsed);
    print_gen();
    prob_type sum = 0;
    for (node_id i = 0; i < N; i++) sum += P[i];
    fprintf(stderr, "sum=%f\n", sum);
    return EXIT_SUCCESS;
}
