#ifndef STRUCT_H
#define STRUCT_H

#define NB_MAX_PROCS 35
#define MAX_WORKERS 10

#include <mpi.h>
#include <time.h>

/*
 * struct Process - Représentation d'un processus.
 *
 * Cette structure définit les caractéristiques d'un processus.
 *
 * @pid : L'identifiant du processus.
 * @global_pid : L'identifiant global du processus.
 * @size : La taille du processus.
 * @commandLine : La ligne de commande associée au processus.
 */
typedef struct proc{
    
    int pid;
    int global_pid;
    int size;
    char **commandLine;

} Process;

// Toutes les variables importantes sont déclarés externes pour y accéder plus facilement dans le programme

/* tableau stockant les informations sur les processus*/
extern Process procs[NB_MAX_PROCS];

/* tableau de caractères stockant le nom de l'hôte */
extern char hostname[30];

/* compteur représentant le nombre total de processus */
extern int nb_proc;

/* entier indiquant le rang de la machine */
extern int rank; 

/* tableau des charges des machines */
extern int workers[MAX_WORKERS];

/* variable stockant l'état de la communication MPI */
extern MPI_Status status;

/* entier indiquant si la machine est active ou non */
extern int idle; 

/* entier indiquant la charge actuelle sur le réseau */
extern int netload;

/* variable stockant une valeur de temps pour une minuterie */
extern time_t timer;

/* entier représentant un délai dans le programme */
extern int delay;

extern int load_prec;

#endif
