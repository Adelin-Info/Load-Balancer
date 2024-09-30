#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "load.h"
#include "tag.h"
#include "inout.h"
#include "work.h"
#include "struct.h"
#include "utils.h"
#include <mpi.h>
#include <pthread.h>

#define MAXGPID 250

int gpid[MAXGPID];

int end = 0;
pthread_mutex_t	mutex;

void *master_update(){  /* Machine passive avec une charge de travail nulle */
  int charge; // Charge de travail reçue
  int flag; // Indicateur de la présence de messages entrants
  int res; // Résultat de la réception
  int len;
 
  while(!end){
    pthread_mutex_lock(&mutex); // Verrouillage du mutex

    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
    // Vérification de la présence de messages
    if (flag){

      switch(status.MPI_TAG){
        case LOAD:
          // Réception de la charge de travail
          MPI_Recv(&charge, 1, MPI_INT, MPI_ANY_SOURCE, LOAD, MPI_COMM_WORLD, &status);
          workers[status.MPI_SOURCE] = charge; // Mise à jour de la charge de travail de la machine correspondante
          break;

        case TRANSFERED:
          // Réception de l'identifiant de processus transféré
          MPI_Recv(&res, 1, MPI_INT, MPI_ANY_SOURCE, TRANSFERED, MPI_COMM_WORLD, &status); 
          gpid[res] = status.MPI_SOURCE; // Mise à jour de la table des identifiants globaux
          break;

        case INFO:
          // Réception d'un message d'information
          MPI_Get_count(&status, MPI_CHAR, &len); // Récupération de la longueur du message
          char * infor = (char *)malloc(sizeof(char)*len);
          MPI_Recv(infor, len, MPI_CHAR, status.MPI_SOURCE, INFO, MPI_COMM_WORLD, &status); // Réception de la ligne de commande
          FILE *file = fopen("log.txt", "a"); // Ouvre le fichier en mode ajout (append)
          if (file == NULL) {
              // Gestion de l'erreur si le fichier ne peut pas être ouvert
              perror("Erreur lors de l'ouverture du fichier");
              exit(EXIT_FAILURE);
          }

          // Écriture du message dans le fichier journal
          fprintf(file, "%s", infor);
          
          // Fermeture du fichier
          fclose(file);
          // Libération de la mémoire allouée pour le message
          free(infor);
          break;
      }
    }
    pthread_mutex_unlock(&mutex); // Déverrouillage du mutex
  }
} 

int CLI() {

  // Initialisation du tableau des identifiants globaux
  for(int j = 0; j < MAXGPID; j++){ gpid[j] = -1; }

  // Initialisation des charges des machines esclaves sur la machine maître
  init_load_master();

  // Initialisation du mutex pour la synchronisation des threads
  pthread_mutex_init(&mutex, NULL);
  // Déclaration du thread
  pthread_t thread;
  // Création du thread d'écoute pour la mise à jour du système
  pthread_create(&thread, NULL, master_update, NULL);

  while(1){

    // Lecture d'une ligne de commande
    char** ligneLue;
    printf("\nTaper la commande: \n");
    ligneLue = readLine();

    // Vérification de la lecture réussie
    if (ligneLue == NULL) {
      printf("Error reading the line.\n");
      return 1;
    }

    // Traitement des commandes
	  if (!strcmp(ligneLue[0], "start")) {
      // Démarrage d'un nouveau processus
      if (ligneLue[1] != NULL){

        int global_pid;
        for(int j = 0; j < MAXGPID; j++){
          if (gpid[j] == -1){
            global_pid = j;
            gpid[j] = -2;
            break;
          }
        }

        int res;    
        if (global_pid > -1){
          pthread_mutex_lock(&mutex);
          res = start(ligneLue, 1, global_pid);
          pthread_mutex_unlock(&mutex);
        }

        if (!res){
            printf("Start: error\n");
        }
      
      } else {
        printf("Start: missing parameters\n");
      }

	  } else if (!strcmp(ligneLue[0], "gps")) {
      // Affichage des informations réseau
      pthread_mutex_lock(&mutex);
			if (ligneLue[1]){
				gps(1);

			} else{
				gps(0);
			}
			pthread_mutex_unlock(&mutex);


  	} else if (!strcmp(ligneLue[0], "gkill")) {
      // Terminaison d'un processus par son identifiant global
      if (ligneLue[1] != NULL && ligneLue[2] != NULL) {
        
        int globalpid_proc = atoi(ligneLue[2]);
        if (gpid[globalpid_proc] > 0){
          // Extraction du signal de la commande gkill
          char tmp[strlen(ligneLue[1])];
          strcpy(tmp, ligneLue[1]);
          int i = 0;
          while(tmp[i] != '\0'){
            tmp[i] = tmp[i+1];
            i++;
          }

          int sig = atoi(ligneLue[1]); 
          int torank = gpid[globalpid_proc];



          int res;
          pthread_mutex_lock(&mutex);
          
          res = killer(sig, torank, globalpid_proc);

          pthread_mutex_unlock(&mutex);

          if (res){
            gpid[globalpid_proc] = -1;
            printf("Gkill: process is killed\n");
          }
          else{
            // Cas où la tâche n'a pas encore été exécuté sur la nouvelle machine
            printf("Gkill: gpid not updated, try later\n");
          }
        } else {
          printf("Gkill: gpid doesn't exist\n");
        }

      } else {
        printf("Gkill: missing parameters\n");
      }

    } else if (!strcmp(ligneLue[0], "insert")) {
      // Insertion d'une nouvelle machine dans le réseau
      if (ligneLue[1] != NULL){

        pthread_mutex_lock(&mutex);
        int to = atoi(ligneLue[1]);
        if (workers[to] == 0 && to < nb_proc) {
          insert_worker(to);
        } else {
          printf("Insert: rank is already working or doesn't exist\n");
        }
        pthread_mutex_unlock(&mutex);

      } else {
        printf("Insert: missing parameters\n");
      }

    } else if (!strcmp(ligneLue[0], "remove")) {
      // Suppression d'une machine du réseau
      if (ligneLue[1] != NULL){
        pthread_mutex_lock(&mutex);
        int rank_to_remove = atoi(ligneLue[1]);
        if (workers[rank_to_remove] != 0 && rank_to_remove < nb_proc) {
          remove_worker(rank_to_remove);
        } else {
          printf("Remove: rank already removed or doesn't exist\n");
        }
        pthread_mutex_unlock(&mutex);

      } else {
        printf("Remove: missing parameters\n");
      }

    } else if (!strcmp(ligneLue[0], "quit")) {
        end = 1;
        pthread_join(thread, NULL);

        for (int i=1; i<nb_proc; i++){
            MPI_Send(&end, 1, MPI_INT, i, END, MPI_COMM_WORLD); // Demander la terminaison
        }

        // Libération de la mémoire allouée pour les mots de la ligne de commande
        clearLine(ligneLue);
        break;

    } else {
      printf("Error: command %s not found\n", ligneLue[0]);
    }
    
    // Libération de la mémoire allouée pour les mots de la ligne de commande
    clearLine(ligneLue);
    printf("\n");
  }

  return 0;
}
