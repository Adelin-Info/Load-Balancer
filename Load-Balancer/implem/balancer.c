#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "load.h"
#include "tag.h"
#include "inout.h"
#include "work.h"
#include "struct.h"

int Worker(){
    int len; // Longueur du nom de la machine
    int rcv; // Variable de réception
    int flag; // Indicateur de la présence de messages entrants
    int end = 0; // Indicateur de terminaison

    // Récupération du nom de la machine
    MPI_Get_processor_name(hostname, &len);

    // Initialisation des charges des machines sur machine esclave
    init_load_worker();
    
    while(!end){
        if (!idle) {
            update(); // Mise à jour des informations
            balancer(); // Équilibrage de la charge si nécessaire
        }

        // Vérification de la présence du message END indiquant la terminaison
        MPI_Iprobe(MPI_ANY_SOURCE, END, MPI_COMM_WORLD, &flag, &status); 
        if (flag){
            int end2;
            // Réception et traitement des messages END
            MPI_Recv(&end2, 1, MPI_INT, MPI_ANY_SOURCE, END, MPI_COMM_WORLD, &status); 
            end = 1;
        }

        // Vérification de la présence de messages GPS
        MPI_Iprobe(MPI_ANY_SOURCE, GPS, MPI_COMM_WORLD, &flag, &status); 
        if (flag){
            int opt;
            // Réception et traitement des messages GPS
            MPI_Recv(&opt, 1, MPI_INT, MPI_ANY_SOURCE, GPS, MPI_COMM_WORLD, &status); 
            rcv_gps(opt);
            update(); // Mise à jour des informations
        }

        // Vérification de la présence de messages entrants
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
        if (flag){
            // Traitement des différents types de messages avec une mise à jour des informations après chaque traitement
            switch(status.MPI_TAG){
                case START:
                    // Réception et traitement du démarrage d'une tâche
                    rcv_start();
                    update();
                    break;

                case GPS:
                    // Réception et traitement des messages GPS
                    int opt;
                    MPI_Recv(&opt, 1, MPI_INT, MPI_ANY_SOURCE, GPS, MPI_COMM_WORLD, &status); 
                    rcv_gps(opt);
                    update();
                    break;

                case KILL:
                    // Réception et traitement des demandes de terminaison de processus
                    int sig, glbpid;
                    MPI_Recv(&glbpid, 1, MPI_INT, MPI_ANY_SOURCE, KILL, MPI_COMM_WORLD, &status); 
                    MPI_Recv(&sig, 1, MPI_INT, MPI_ANY_SOURCE, SIG, MPI_COMM_WORLD, &status); 
                    rcv_kill(sig, glbpid);
                    update();
                    break;


                case TRANSFER:
                    // Réception et traitement des transferts de tâches
                    receive_transfer();
                    update();
                    break;

                case INSERT:
                    // Réception et traitement des insertions de nouvelles machines
                    receive_insert();
                    update();
                    break;

                case REMOVE:
                    // Réception et traitement des suppressions de machines
                    MPI_Recv(&rcv, 1, MPI_INT, MPI_ANY_SOURCE, REMOVE, MPI_COMM_WORLD, &status);
                    receive_remove();
                    update();
                    break;

                case LOAD:
                    // Réception et traitement des mises à jour des charges de travail
                    MPI_Recv(&workers[status.MPI_SOURCE], 1, MPI_INT, MPI_ANY_SOURCE, LOAD, MPI_COMM_WORLD, &status);
                    update();
                    break;

                case CLAIM:
                    // Réception et traitement des demandes de tâches
                    receive_claim();
                    update();
                    break;
            }
        }
    }



    // Parcourir toutes les tâches de la machine locale
    for(int i = 0; i < NB_MAX_PROCS; i++){
        // Vérifier si une tâche est active
        if (procs[i].pid){
            end_task(i); // terminer les taches
        }
    }
    
    return 0; 
}