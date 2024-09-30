#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>
#include <pthread.h>
#include "../implem/load.h"
#include "../implem/tag.h"
#include "../implem/inout.h"
#include "../implem/work.h"
#include "../implem/struct.h"
#include "../implem/balancer.h"

#define MAXGPID 250

int gpid[MAXGPID];
pthread_mutex_t	mutex;
int end = 0;

void *smt_update(){  /* Machine passive avec une charge de travail nulle */
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

int main(int argc, char* argv[]) {
   
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   int length;
   MPI_Get_processor_name(hostname, &length);

   if (nb_proc < 2 || nb_proc > MAX_WORKERS) {
      printf("Nombre de machines incorrect !\n");
      MPI_Finalize();
      exit(2);
   }
    
   timer = time(NULL);

   if (rank == 0) {
    // Initialisation du tableau des identifiants globaux
    for(int j = 0; j < MAXGPID; j++){ gpid[j] = -1; }
      
    int size = 3;
    char **commandLine = (char**)malloc(sizeof(char *) * size);
    for(int i = 0; i < (size-1); i++){
        commandLine[i] = (char *)malloc(sizeof(char)*20);
    }

    strcpy(commandLine[0], "start");
    strcpy(commandLine[1], "../progs/loop");
    commandLine[2] = NULL;

    printf("\nRéception des charges des machines du réseau\n");
    init_load_master();
    printf("\n\n");

    /* thread */
    pthread_mutex_init(&mutex, NULL);
    pthread_t thread;
    pthread_create(&thread, NULL, smt_update, NULL);
    
    printf("Lancement des taches sur la machine la moins chargée du réseau\n\n");
    
    printf("!! Plusieurs sleep(>30) sont présents afin d'obtenir une activité suffisamment pertinente !!\n\n");
    
    
    int global_pid = 0;
    for (int i = 0; i < 10; i++){
        pthread_mutex_lock(&mutex);
        start(commandLine, 1, global_pid);
        pthread_mutex_unlock(&mutex);
        global_pid++;
    }
    sleep(35);
    printf("\n\n");

    
    pthread_mutex_lock(&mutex);
    printf("Affichage des taches en cours d'exécution à l'instant t\n\n");
    gps(1);
    update();
    pthread_mutex_unlock(&mutex);
    sleep(45);
    printf("\n\n");

    int to_remove = hardworker();

    printf("On retire la machine la plus chargée du réseau, soit la machine %d\n\n", to_remove);
    pthread_mutex_lock(&mutex);
    remove_worker(to_remove);
    pthread_mutex_unlock(&mutex);
    sleep(40);
    printf("\n\n");

    
    pthread_mutex_lock(&mutex);
    printf("Affichage des taches en cours d'exécution à l'instant t apres le retrait de %d\n\n", to_remove);
    gps(1);
    pthread_mutex_unlock(&mutex);
    sleep(25);
    printf("\n\n");

    printf("On insère la machine précedemment retirée dans le réseau\n\n");
    pthread_mutex_lock(&mutex);
    insert_worker(to_remove);
    pthread_mutex_unlock(&mutex);
    sleep(45);
    printf("\n\n");

    
    sleep(5);
    pthread_mutex_lock(&mutex);
    printf("Affichage des taches en cours d'exécution à l'instant t apres l'insertion de %d\n\n", to_remove);
    gps(1);
    pthread_mutex_unlock(&mutex);
    sleep(35);
    printf("\n\n");


    
    sleep(5);
    pthread_mutex_lock(&mutex);
    printf("Nouvel affichage des taches en cours d'exécution à l'instant t avant le retrait massif\n\n", to_remove);
    gps(1);
    pthread_mutex_unlock(&mutex);
    sleep(35);
    printf("\n\n");

    printf("On retire toutes les machines afin que toutes les tâches soient réunis sur une machine\n\n");
    for(int i=2; i<nb_proc; i++){
        pthread_mutex_lock(&mutex);
        remove_worker(i);
        pthread_mutex_unlock(&mutex);
    }
    sleep(35);
    printf("\n\n");

    
    pthread_mutex_lock(&mutex);
    printf("Affichage des taches en cours d'exécution à l'instant t apres le retrait de toutes les machines sauf une\n\n");
    gps(1);
    pthread_mutex_unlock(&mutex);
    sleep(25);
    printf("\n\n");

    printf("On tue la tâche avec le gpid 3\n");
    int res;
    pthread_mutex_lock(&mutex);
    while(!(res = killer(9, 1, 3))){ 
    	pthread_mutex_unlock(&mutex);
    	sleep(5); 
    	pthread_mutex_lock(&mutex);
    }
    pthread_mutex_unlock(&mutex);
    
    sleep(30);
    printf("\n\n");
    
    
    pthread_mutex_lock(&mutex);
    printf("Affichage des taches en cours d'exécution à l'instant t apres le gkill -9\n\n");
    gps(1);
    pthread_mutex_unlock(&mutex);
    sleep(25);
    printf("\n\n");

    printf("Fin de la simulation\n");

    end = 1;
    pthread_join(thread, NULL);

    for (int i=1; i<nb_proc; i++){
    	MPI_Send(&end, 1, MPI_INT, i, END, MPI_COMM_WORLD); // Demander la terminaison
    }

   } else {
      Worker();  
   }
  
   MPI_Finalize();
   return 0;
}
