#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../implem/client.h"
#include "../implem/balancer.h"
#include "../implem/struct.h"


int main(int argc, char* argv[]) {
   
   // Initialisation de MPI
   MPI_Init(&argc, &argv);

   // Obtention du nombre de processus et du rang de chaque processus dans le communicateur MPI_COMM_WORLD
   MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   // Vérification si le nombre de machines est correct
   if (nb_proc > MAX_WORKERS || nb_proc < 2) {
      printf("Nombre de machines incorrect !\n");
      MPI_Finalize();
      return -1;
   }

   // Initialisation du timer avec l'heure actuelle
   timer = time(NULL);
   
   // Si le processus a un rang de 0, il s'agit du processus maître
   if (rank == 0) {
      CLI();
   // Sinon, il s'agit de processus esclaves
   } else {
      Worker();
   }

   // Terminaison de MPI
   MPI_Finalize();
   
   return 0;
}