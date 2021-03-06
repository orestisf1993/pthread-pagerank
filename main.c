#define _GNU_SOURCE
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include "utils.h"

static unsigned int nthreads;

static pthread_barrier_t barrier;

static bool running = true;
static bool *local_terminate_flag;

float calculate_const_add(void) {
    // calculate the constant for links without outbound links.
    float res = 0.0f;
    for (node_id x = 0; x < size_no_out; x++) {
        const node_id j = no_outbounds[x];
        res += P[j];
    }
    res /= (prob_type) N;
    return res;
}

#define D 0.85f
static void *calculate_pagerank(void *_args) {
    const parm *args = (parm *) _args;
    const unsigned int tid = args->tid;
    const node_id start = args->start;
    const node_id end = args->end;

    const prob_type const_E = (1 - D) / (prob_type) N;

    #ifdef DEBUG
    fprintf(stderr, "tid %u, start %u, end %u chunk %u\n", tid, start, end, end - start);
    #endif

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

            P_new[i] = D * link_prob + (args->custom_E ? (1 - D) * E[i] : const_E);
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

            running = false;
            for (unsigned int i = 0; i < nthreads; i++) {
                if (!local_terminate_flag[i]) {
                    running = true;
                    break;
                }
            }
            if (running) {
                constant_add = calculate_const_add();
            }
        }

        pthread_barrier_wait(&barrier);
        if (!running) break;
    }

    if (!tid) {
        unsigned int *ret = malloc(sizeof(unsigned int));
        *ret = gen + 1;
        return ret;
    }
    else {
        return NULL;
    }
}

static parm *split_work(int smart_split) {
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
            args[tid].start = (node_id) (tid) * chunk;
            args[tid].end = args[tid].start + chunk + (tid == nthreads - 1) * (N % nthreads);
        }
    }
    return args;
}

int main(int argc, char **argv) {
    main_options arg_options = argument_parser(argc, argv);
    nthreads = arg_options.nthreads;
    fprintf(stderr, "opening file %s, operating with %u threads\n", arg_options.filename, nthreads);
    read_from_file(arg_options.filename);
    init_prob(arg_options.custom_F, arg_options.custom_E);

    fprintf(stderr, "Read %ux%u graph with:\n"
                    "\t%"PRIu64" vertices\n"
                    "\t%u nodes without outbound links\n",
            N, N, n_vertices, size_no_out);

    pthread_barrier_init(&barrier, NULL, nthreads);
    pthread_t *threads = malloc(nthreads * sizeof(pthread_t));
    local_terminate_flag = malloc(nthreads * sizeof(int));

    parm *args = split_work(arg_options.smart_split);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    for (unsigned int i = 0; i < nthreads; i++) {
        args[i].tid = i;
        args[i].custom_E = (arg_options.custom_E != NULL);
        pthread_create(&threads[i], NULL, calculate_pagerank, (void *) &args[i]);
    }
    unsigned int *final_gen;
    pthread_join(threads[0], (void **) &final_gen);
    for (unsigned int i = 1; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) +
            ((end.tv_usec - start.tv_usec) / 1000000.0);

    free(args);
    free(threads);

    printf("finished on generation %u after %g sec\n", *final_gen, elapsed);
    save_res(N, nthreads, *final_gen, elapsed);
    print_gen(nthreads);

    prob_type sum = 0.0;
    for (node_id i = 0; i < N; i++) sum += P[i];
    printf("sum=%f\n", sum);

    free(final_gen);

    return EXIT_SUCCESS;
}
