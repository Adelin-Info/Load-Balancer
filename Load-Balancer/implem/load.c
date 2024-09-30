#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "load.h"
#include "tag.h"
#include "inout.h"
#include "struct.h"
#include "utils.h"

int load(){
    float load; // Variable pour stocker la charge système
    char buffer[64]; // Tampon pour stocker les données lues depuis le fichier

    // Ouvrir le fichier "/proc/loadavg" en mode lecture
    FILE *loadavg = fopen("/proc/loadavg", "r");
    if (!loadavg){
        // Afficher un message d'erreur en cas d'échec de l'ouverture du fichier et quitter le programme
        printf("Error! : opening file\n");   
        exit(1);  
    }

    // Lire la première ligne du fichier (contenant la charge système)
    fgets(buffer, 64, loadavg);

    // Analyser la ligne lue pour extraire la charge système
    sscanf(buffer, "%f", &load);

    // Fermer le fichier après lecture
    fclose(loadavg);

    // Calculer la charge en pourcentage (par rapport à un maximum théorique de 12)
    int pourcent = (load / 12)*100;

    // Assurer que la charge est comprise entre 1 et 100
    return pourcent > 0 ? pourcent : 1;
}

void publish_load(){
    // Obtenir l'heure actuelle
    time_t current_time = time(NULL);

    // Calculer la différence de temps depuis la dernière publication
    unsigned int diff = (unsigned int) difftime( current_time, timer );

    // Vérifier si la différence de temps dépasse le délai spécifié
    if (diff >= delay){
        // Mettre à jour le temps de la dernière publication
        timer = current_time;
        // Vérifier si la machine est inactive
        if (idle){
            return;
        } else {
            // Calculer la charge de la machine locale
            int myload = load();
            // Mettre à jour la charge de la machine locale dans le tableau des machines
            workers[rank] = myload;
            load_prec = myload;
            // Envoyer la charge de la machine locale aux autres machines
            for(int i = 0; i < nb_proc; i++){
                if ((i != rank)){
                    // Envoyer la charge avec le type de message LOAD
                    MPI_Send(&myload, 1, MPI_INT, i, LOAD, MPI_COMM_WORLD);
                }
            }
        }
    }
}

int state_load(){
    // Récupérer la charge locale de la machine
    int myload = workers[rank];

    // Récupérer la charge réseau globale
    int net = netload;

    // Comparer la charge locale avec les seuils ajustés selon la charge réseau
    if ((myload > (MAX_LOAD * net))){
        // Si la charge locale dépasse le seuil maximal ajusté selon la charge réseau
        return 1; // La machine locale est surchargée par rapport au réseau
    } else {
        if ((myload < (MIN_LOAD * net))){
            // Si la charge locale est inférieure au seuil minimal ajusté selon la charge réseau
            return -1; // La machine locale est sous-chargée par rapport au réseau
        } else {
            // Si la charge locale est dans une plage acceptable
            return 0; // La machine locale est dans une plage de charge acceptable par rapport au réseau
        }
    }
}


void update(){
    int charge; // Variable pour stocker la charge reçue
    int flag; // Variable pour indiquer la présence de messages
    int nb = delay * 10; // Nombre d'itérations pour vérifier la réception de messages
    int i = 0; // Compteur d'itérations

    // Parcourir les itérations pour vérifier la réception de messages
    while(i < nb){
        // Vérifier la présence de messages
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status); 
        // Si un message est présent
        if(flag){
            // Traiter le message en fonction de son tag
            switch(status.MPI_TAG){
                case LOAD:
                    // Recevoir la charge envoyée par une autre machine 
                    MPI_Recv(&charge, 1, MPI_INT, MPI_ANY_SOURCE, LOAD, MPI_COMM_WORLD, &status);
                    // Mettre à jour la charge de la machine correspondante dans le tableau workers
                    workers[status.MPI_SOURCE] = charge;
                    break;
            }
        }
        i++; // Passer à l'itération suivante
    }

    // Calculer le nombre de machines actives
    int nb_worker = nb_workers(workers);

    // Si aucune machine active n'est détectée, retourner sans mettre à jour la charge réseau
    if (!nb_worker){
        return;
    }

    int net = 0; // Variable pour calculer la charge réseau globale

    // Calculer la somme des charges des machines pour obtenir la charge réseau globale
    for(int i = 1; i < nb_proc; i++){
        net += workers[i]; 
    }
    
    // Mettre à jour la charge réseau globale en prenant la moyenne des charges des machines actives
    if (nb_worker != 0){
        netload = (int) net / nb_worker;
        
    }
}

int available_proc(){
    // Parcourir le tableau de processus
    for (int i=0; i<NB_MAX_PROCS; i++){
        // Vérifier si l'identifiant du processus est différent de zéro
        if (procs[i].pid != 0){
            return 1; // Retourner 1 si au moins un processus est disponible
        }
    }
    return 0; // Retourner 0 si aucun processus n'est disponible
}

int busy_proc(){
    int max = 0; // Variable pour stocker la charge CPU maximale trouvée
    int pos = -1; // Variable pour stocker la position du processus le plus chargé
    int cpu; // Variable pour stocker la charge CPU d'un processus

    // Parcourir le tableau de processus
    for (int i=0; i<NB_MAX_PROCS; i++){
        // Vérifier si l'identifiant du processus est différent de zéro (c'est-à-dire si le processus est actif)
        if (procs[i].pid != 0){
            // Obtenir la charge CPU du processus en utilisant la fonction cpu_proc
            cpu = cpu_proc(procs[i].pid);
            // Comparer la charge CPU avec la charge maximale trouvée jusqu'à présent
            if (cpu >= max){
                // Si la charge CPU est supérieure ou égale à la charge maximale trouvée jusqu'à présent
                max = cpu; // Mettre à jour la charge maximale
                pos = i; // Mettre à jour la position du processus le plus chargé
            }
        }
    }
    return pos; // Retourner la position du processus le plus chargé dans le tableau de processus
}

int cpu_proc(int pid){
    char** tab_stat = NULL; // Tableau de chaînes de caractères pour stocker les statistiques du processus
    char line[256]; // Tampon pour stocker une ligne du fichier /proc/pid/stat
    int size = 0; // Taille du tableau de statistiques
    char path[50]; // Chemin du fichier /proc/pid/stat
    FILE* file; // Pointeur vers le fichier /proc/pid/stat

    int utime, stime, process_usage; // Variables pour stocker le temps utilisateur, le temps système et l'utilisation CPU du processus

    // Construire le chemin du fichier /proc/pid/stat
    sprintf(path, "/proc/%d/stat", pid);

    // Ouvrir le fichier /proc/pid/stat en mode lecture
    file = fopen(path, "r");
    if(!file){
        // Afficher un message d'erreur en cas d'échec de l'ouverture du fichier et retourner -1
        perror("Erreur lors de l'ouverture du fichier\n");
        return -1;
    }

    // Lire la première ligne du fichier /proc/pid/stat
    if (!fgets(line, 256, file)){
        fclose(file); // Fermer le fichier
        return -1; // Retourner -1 en cas d'échec de lecture
    }
    fclose(file); // Fermer le fichier après la lecture
    
    // Découper la ligne en un tableau de chaînes de caractères pour extraire les statistiques
    char* token = strtok(line, " ");
    while (token) {
        tab_stat = realloc(tab_stat, sizeof(char*) * (size + 1));
        tab_stat[size++] = malloc(sizeof(char) * (strlen(token) + 1));
        strcpy(tab_stat[size - 1], token);
        token = strtok(NULL, " ");
    }
    tab_stat = realloc(tab_stat, sizeof(char*) * (size + 1));
    tab_stat[size++] = NULL;

    // Extraire les valeurs de temps utilisateur (utime) et système (stime)
    utime = atoi(tab_stat[13]);
    stime = atoi(tab_stat[14]);

    // Calculer l'utilisation CPU totale en additionnant les temps utilisateur et système
    process_usage = utime + stime;

    // Libérer la mémoire allouée pour le tableau de statistiques
    clearLine(tab_stat);

    return process_usage; // Retourner l'utilisation CPU du processus
}

int mem_proc(int pid){
    FILE* file; // Pointeur vers le fichier /proc/pid/status
    char path[50]; // Chemin du fichier /proc/pid/status
    char line[256]; // Tampon pour stocker une ligne du fichier
    int mem; // Variable pour stocker la quantité de mémoire utilisée par le processus
    
    // Construction du chemin du fichier /proc/pid/status
    sprintf(path, "/proc/%d/status", pid);

    // Ouverture du fichier /proc/pid/status en mode lecture
    file = fopen(path, "r");
    if(!file){
        // Afficher un message d'erreur en cas d'échec d'ouverture du fichier et retourner -1
        perror("Erreur ouverture fichier\n");
        return -1;
    }

    // Parcourir les lignes du fichier jusqu'à trouver la ligne contenant l'information sur la mémoire résidente (VmRSS)
    while(fgets(line, 256, file) != NULL){
        if (strstr(line, "VmRSS:") != NULL){
            // Découper la ligne pour obtenir la quantité de mémoire utilisée
            char* token = strtok(line, " ");
            token = strtok(NULL, " ");
            mem = atoi(token); // Convertir la chaîne de caractères en entier
            break; // Sortir de la boucle une fois la quantité de mémoire trouvée
        }
    }
    fclose(file); // Fermer le fichier après lecture

    return mem; // Retourner la quantité de mémoire utilisée par le processus
}

void init_load_master(){
    // Parcourir les machines à partir de 1, sauf machine maître
    for(int i=1; i<nb_proc; i++){
        // Recevoir la charge de la machine correspondante
        MPI_Recv(&workers[i], 1, MPI_INT, i, LOAD, MPI_COMM_WORLD, &status);
    }
}

void init_load_worker(){
    for (int i=0; i < NB_MAX_PROCS; i++){
	    procs[i].global_pid = -1;
    }

     // Calculer la charge de la machine local
    int myload = load();
    load_prec = myload;

    // Envoyer la charge de la machine local à tous les autres machines (sauf lui-même)
    for(int i=0; i<nb_proc; i++){
        if(i != rank){
            MPI_Send(&myload, 1, MPI_INT, i, LOAD, MPI_COMM_WORLD);
        }
        
    }
    
    // Recevoir les charges des autres machines
    for(int i=1; i<nb_proc; i++){
        if(i != rank){
            MPI_Recv(&workers[i], 1, MPI_INT, i, LOAD, MPI_COMM_WORLD, &status);
        }
    }
}
