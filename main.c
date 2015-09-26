#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <sys/time.h>
#include <math.h>
#include "utils.h"

static unsigned int nthreads = DEFAULT_NTHREADS;

prob_type *P;
static prob_type *E;
static prob_type *P_new;

void init_prob(void) {
    P = malloc(N * sizeof(prob_type));
    P_new = malloc(N * sizeof(prob_type));
    E = malloc(N * sizeof(prob_type));

    for (node_id i = 0; i < N; i++) {
        P[i] = E[i] = 1 / (prob_type) N;   // uniform distribution

        // Any node with no out links is linked to all nodes to emulate the matlab script.
        if (!n_outbound[i]) {
            no_outbounds = realloc(no_outbounds, (++size_no_out) * sizeof(node_id));
            no_outbounds[size_no_out - 1] = i;
        }
        if (!n_inbound[i]) size_no_in++;
    }
}

static pthread_barrier_t barrier;

static int running = 1;
static int *local_terminate_flag;

#define D 0.85f
void *calculate_gen(void *_args) {
    const parm *args = (parm *) _args;
    const unsigned int tid = args->tid;
    const node_id start = args->start;
    const node_id end = args->end;

    #ifdef DEBUG
    fprintf(stderr, "tid %u, start %u, end %u chunk %u\n", tid, start, end, end - start);
    #endif

    // initialize for uniform distribution.
    // TODO: test make it global/shared between threads.
    prob_type constant_add = (prob_type) size_no_out / (prob_type) (N) / (prob_type) N;
    unsigned int gen;
    for (gen = 0; gen < MAX_GENERATIONS; gen++) {
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
            // swap P_new with P.
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

        pthread_barrier_wait(&barrier);  // TODO: is this needed?
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

parm *split_work(int smart_split) {
    parm *args = malloc(nthreads * sizeof(parm));

    if (smart_split) {
        // split work evenly.
        unsigned int tid = 0;
        double split = (double) (n_vertices + size_no_in) / (double) nthreads;
        double chunk = n_inbound[0] + 1;
        args[0].start = 0;
        args[0].tid = 0;
        for (node_id i = 1; i < N; i++) {
            chunk += n_inbound[i] + 1;
            if (chunk > split) {
                split -= (chunk - split) / (double) nthreads;
                chunk = 0;
                args[tid++].end = i;
                args[tid].start = i;
                args[tid].tid = tid;
                if (tid == nthreads - 1) break;
            }
        }
        if (tid != nthreads - 1) {
            smart_split = 0;
            fprintf(stderr, "Failed to apply smart split, reverting to normal\n");
        }
        else {
            args[nthreads - 1].end = N;
        }
    }
    if (!smart_split) {
        const node_id chunk = N / nthreads;
        for (unsigned int tid = 0; tid < nthreads; tid++) {
            args[tid].tid = tid;
            args[tid].start = (node_id) (tid) * chunk;
            args[tid].end = args[tid].start + chunk + (tid == nthreads - 1) * (N % nthreads);
        }
    }
    return args;
}

int main(int argc, char **argv) {
    const char *filename = DEFAULT_NODES_FILENAME;
    int smart_split = 0;

    static struct option long_options[] = {
            {"nodes-file", required_argument, NULL, 'n'},
            {"nthreads", required_argument, NULL, 't'},
            {"smart-split", no_argument, NULL, 's'},
            {"help", no_argument, NULL, 'h'},
            {NULL, NULL, NULL, NULL}
    };

    while (1) {
        int opt = getopt_long(argc, argv, "n:t:hs", long_options, NULL);
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
            case 's':
                smart_split = 1;
                break;
            default:
                print_usage(argv);
                exit(E_UNRECOGNISED_ARGUMENT);
        }
    }

    #ifdef DEBUG
    fprintf(stderr, "opening file %s, operating with %u threads\n", filename, nthreads);
    #endif

    read_from_file(filename);
    init_prob();

    fprintf(stderr, "Read %ux%u graph with:\n"
                    "\t%lu vertices\n"
                    "\t%u nodes without outbound links\n",
            N, N, n_vertices, size_no_out);

    pthread_barrier_init(&barrier, NULL, nthreads);
    pthread_t *threads = malloc(nthreads * sizeof(pthread_t));
    local_terminate_flag = malloc(nthreads * sizeof(int));

    parm *args = split_work(smart_split);

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

    free(args);
    free(threads);

    fprintf(stderr, "finished on generation %lu after %g sec\n", (uintptr_t) final_gen, elapsed);
    save_res(N , nthreads, (uintptr_t) final_gen,  elapsed);
    print_gen(nthreads);

    prob_type sum = 0.0;
    for (node_id i = 0; i < N; i++) sum += P[i];
    printf("sum=%f\n", sum);

    return EXIT_SUCCESS;
}
