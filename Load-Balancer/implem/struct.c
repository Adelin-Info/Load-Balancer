#include "struct.h"
#include <time.h>

// Initialisation de certaines variables

int workers[MAX_WORKERS] = {0};
int idle = 0;

Process procs[NB_MAX_PROCS];
char hostname[30];
int nb_proc;
int rank;
MPI_Status status;
int netload;
time_t timer;
int delay = 35;
int load_prec = 0;
