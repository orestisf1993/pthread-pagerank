#ifndef ASSIGNMENT_4_UTILS_H
#define ASSIGNMENT_4_UTILS_H

#include <limits.h>
#include <stdbool.h>

#define MAX_GENERATIONS 10000
#define DEFAULT_NTHREADS 4
#define MILLION 1000000
#define MAX_ERROR 0.000001f
#define DEFAULT_NODES_FILENAME "nodes.txt"
#define RESULTS_FILENAME "time_results.txt"

#define max(a, b) \
    ({ typeof(a) _a = (a); \
    typeof(b) _b = (b); \
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
typedef uint32_t node_id;
#endif
typedef node_id **graph;

#ifdef USE_DOUBLE_PRECISION
#define prob_type double
#else
#define prob_type float
#endif

typedef struct {
    unsigned int tid;
    node_id start;
    node_id end;
    unsigned int gen;
} parm;

typedef struct {
    const char *filename;
    bool smart_split;
    char *custom_F;
    char *custom_E;
    unsigned int nthreads;
} main_options;

void *calculate_gen(void *_args);
void read_from_file(const char *filename);
void print_gen(unsigned int nthreads);
void init_prob(char *custom_F, char *custom_E);
void print_usage(char **argv);
void save_res(int size, int threads, unsigned int final_gen, double time);
main_options argument_parser(int argc, char **argv);
float calculate_const_add(void);
parm *split_work(int smart_split);

extern graph L;
extern node_id *n_inbound;
extern node_id *n_outbound;
extern node_id *no_outbounds;
extern node_id N;
extern node_id size_no_out;
extern node_id size_no_in;
extern uint64_t n_vertices;

extern prob_type *P;
extern prob_type *E;
extern prob_type *P_new;
extern prob_type constant_add;

#endif // ASSIGNMENT_4_UTILS_H
