#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "load.h"
#include "tag.h"
#include "inout.h"
#include "work.h"
#include "struct.h"


void insert_worker(int to){
    // Vérifier si la charge de la machine destination est nulle
    if (workers[to] == 0){
        // Envoyer les charges des machines à la machine destination
        MPI_Send(workers, MAX_WORKERS, MPI_INT, to, INSERT, MPI_COMM_WORLD);    
    }
}

void receive_insert(){
    idle = 0; // Marquer la nouvelle machine comme active
    char str[256];

    sprintf(str, "[INSERT] La machine %d a bien été insérer\n",rank);
    MPI_Send(str, strlen(str)+1, MPI_CHAR, 0, INFO, MPI_COMM_WORLD);
    
    // Recevoir les charges des machines depuis n'importe quel machine
    MPI_Recv(workers, MAX_WORKERS, MPI_INT, MPI_ANY_SOURCE, INSERT, MPI_COMM_WORLD, &status);
    // Calculer la charge locale de la nouvelle machine en utilisant la fonction load()
    int loader = load();
    workers[rank] = loader; // Mettre à jour la charge de la machine local dans le tableau des machines
    load_prec = loader;
    // Envoyer la charge de la nouvelle machine à tous les autres machines pour mise à jour
    for (int i = 0; i < nb_proc; i++){
        if (i != rank){
            MPI_Send(&loader, 1, MPI_INT, i, LOAD, MPI_COMM_WORLD);
        }
    }
}

void remove_worker(int rank_to_remove){
    int ul = 1; // Valeur pour signaler la suppression d'une machine

    // Envoyer le signal de suppression à la machine spécifiée
    MPI_Send(&ul, 1, MPI_INT, rank_to_remove, REMOVE, MPI_COMM_WORLD);
}

void receive_remove(){
    char str[256];
    // Exécuter les opérations nécessaires pour mettre la machine en mode veille
    gobacktosleep();
    
    sprintf(str, "[REMOVE] La machine %d a bien été retirer\n",rank);
    MPI_Send(str, strlen(str)+1, MPI_CHAR, 0, INFO, MPI_COMM_WORLD);
}

int nb_workers(){
    int cpt = 0; // Compteur pour le nombre de machines actives

    // Parcourir le tableau des machines
    for (int i = 1; i < MAX_WORKERS; i++){
        // Vérifier si la charge de la machine est différente de zéro
        if (workers[i] != 0){
            cpt++; // Incrémenter le compteur si la machine est active
        }
    }
    return cpt; // Retourner le nombre de machines actives
}

int lazyone(){
    int min_rank_underload = nb_proc+1; // Initialisation avec une valeur plus grande que le nombre de machines
    int min_load = 100; // Initialisation avec une valeur arbitrairement grande

    // Parcourir le tableau des machines pour trouver la moins chargée
    for (int i = 1; i < nb_proc; i++){
        // Vérifier si la machine est active, et si sa charge est inférieure ou égale à la charge minimale actuelle
        if ((workers[i] <= min_load) && (workers[i] != 0) && (i != rank)){
            min_load = workers[i]; // Mettre à jour la charge minimale
            min_rank_underload = i; // Mettre à jour l'identifiant de la machine la moins chargée
        }
    }

    // Vérifier si toutes les machines sont inactives, si c'est le cas retourner 0
    if (min_rank_underload == nb_proc+1){
        return 0;
    }
    return min_rank_underload; // Retourner l'identifiant de la machine la moins chargée parmi celles actives
}


int hardworker(){
    int rank_overload = nb_proc+1; // Initialisation avec une valeur plus grande que le nombre de machines
    int max_load = 0; // Initialisation avec une valeur arbitrairement petite

    // Parcourir le tableau des machines pour trouver la plus chargée
    for (int i = 1; i < nb_proc; i++){
        // Vérifier si la machine est active, et si sa charge est supérieure ou égale à la charge maximale actuelle
        if ((workers[i] >= max_load) && (workers[i] != 0) && (i != rank)){
            max_load = workers[i]; // Mettre à jour la charge maximale
            rank_overload = i; // Mettre à jour l'identifiant de la machine la plus chargée
        }
    }

    // Vérifier si toutes les machines sont inactives, si c'est la cas retourner 0
    if (rank_overload == nb_proc+1){
        return 0;
    }

    return rank_overload; // Retourner l'identifiant de la machine la plus chargée parmi celles actives
}


void gobacktosleep(){
    idle = 1; // Mettre la machine locale en mode veille
    
    // Envoyer un signal de charge nulle à toutes les autres machines pour mettre à jour leurs informations sur la charge de la machine locale
    for (int i = 0; i < nb_proc; i++){
        if (i != rank){
            int load = 0;
            MPI_Send(&load, 1, MPI_INT, i, LOAD, MPI_COMM_WORLD);
        }
    }
    workers[rank] = 0;  // Mettre la charge de la machine locale à 0
    
    // Transférer toutes les tâches de la machine locale à une autre machine
    work_transfer(0, 1);
} 
