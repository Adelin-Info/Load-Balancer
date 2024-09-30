#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "load.h"
#include "tag.h"
#include "inout.h"
#include "work.h"
#include "struct.h"

void work_transfer(int to, int opt){
    int torank = -1; // Initialisation de l'identifiant de la machine destination
    int pos; // Position de la tâche à transférer
    char str[256]; // Chaîne de caractères pour les informations sur le transfert de tâches

    // Si l'option opt est désactivée (opt = 0), sélectionner une tâche active de la machine locale et la transférer
    if (!opt){
        pos = busy_proc(); // Récupérer la position de la tâche active la plus chargée
        // Vérifier si une tâche active a été trouvée
        if (pos != -1){
            if (to){
                torank = to; // Utiliser la machine destination spécifiée
            } else {
                update(); // Mettre à jour les informations sur la charge des machines
                torank = lazyone();  // Récupérer la machine la moins chargée du réseau
                if (!torank){
                    return; // Aucune machine disponible, donc retourner
                }
            }

            // Envoyer la tâche à la machine destination
            MPI_Send(&procs[pos], sizeof(Process), MPI_BYTE, torank, TRANSFER, MPI_COMM_WORLD);
            for(int j = 0; j < procs[pos].size; j++){  
                MPI_Send(procs[pos].commandLine[j], strlen(procs[pos].commandLine[j])+1, MPI_CHAR, torank, CMD, MPI_COMM_WORLD);
            }

            // Terminer la tâche sur la machine locale
            end_task(pos);

            // Envoyer des informations sur le transfert de la tâche à la machine maître
            sprintf(str, "[TRANSFERT] La machine %d transfère une de ses tâches à %d\n", rank, torank);
            MPI_Send(str, strlen(str)+1, MPI_CHAR, 0, INFO, MPI_COMM_WORLD);
        }
        return;
    }

    // Si l'option opt est activée (opt = 1), transférer toutes les tâches de la machine locale
    // Envoyer des informations sur le transfert des tâches à la machine maître
    sprintf(str, "[TRANSFERT] La machine %d transfère toutes ses tâches\n", rank);
    MPI_Send(str, strlen(str)+1, MPI_CHAR, 0, INFO, MPI_COMM_WORLD);

    // Parcourir toutes les tâches de la machine locale
    for(int i = 0; i < NB_MAX_PROCS; i++){
        // Vérifier si une tâche est active
        if (procs[i].pid){
            if (to){
                torank = to; // Utiliser la machine destination spécifiée
            } else {
                update(); // Mettre à jour les informations sur la charge des machines
                torank = lazyone(); // Récuperer la machine la moins chargée du réseau
                if (!torank){
                    return; // Aucune machine disponible, donc retourner
                }
            }

            // Envoyer la tâche à la machine destination
            MPI_Send(&procs[i], sizeof(Process), MPI_BYTE, torank, TRANSFER, MPI_COMM_WORLD);
            for(int j = 0; j < procs[i].size; j++){  
                MPI_Send(procs[i].commandLine[j], strlen(procs[i].commandLine[j])+1, MPI_CHAR, torank, CMD, MPI_COMM_WORLD);
            }

            // Terminer la tâche sur la machine locale
            end_task(i);
        } 
    }
    return;
}

void bridge(int torank, Process *p){
    // Envoyer la structure de processus p à l'autre machine
    MPI_Send(p, sizeof(Process), MPI_BYTE, torank, TRANSFER, MPI_COMM_WORLD);

    // Envoyer chaque ligne de la ligne de commande de la tâche à l'autre machine
    for(int i = 0; i < p->size; i++){  
        MPI_Send(p->commandLine[i], strlen(p->commandLine[i])+1, MPI_CHAR, torank, CMD, MPI_COMM_WORLD);
    }

    // Libérer la mémoire allouée pour stocker la ligne de commande
    freer(p->commandLine);
}

void receive_transfer(){ 
    int len; // Longueur du message reçu

    // Recherche d'un emplacement libre dans le tableau des tâches locales
    int free_pos = 0;
    for(int i = 0; i < NB_MAX_PROCS; i++){
        if (!procs[i].pid){
            free_pos = i;
            break;
        }
    }

    // Si la machine locale est inactive
    if (idle){
        Process bin; // Structure temporaire pour stocker la tâche reçue
        MPI_Recv(&bin, sizeof(Process), MPI_BYTE, MPI_ANY_SOURCE, TRANSFER, MPI_COMM_WORLD, &status); // Réception de la tâche
        int sender = status.MPI_SOURCE; // Identification de la machine expéditrice
        bin.commandLine = (char **)malloc(sizeof(char *) * bin.size+1); // Allocation de mémoire pour les lignes de commande de la tâche

        // Réception de chaque ligne de la ligne de commande de la tâche
        for(int i = 0; i < bin.size; i++){  
            MPI_Probe(sender, CMD, MPI_COMM_WORLD, &status); // Sonde pour connaître la taille du message à recevoir
            MPI_Get_count(&status, MPI_CHAR, &len); // Obtention de la taille du message
            bin.commandLine[i] = (char*) malloc(sizeof(char)*len); // Allocation de mémoire pour stocker la ligne de commande
            MPI_Recv(bin.commandLine[i], len, MPI_CHAR, sender, CMD, MPI_COMM_WORLD, &status); // Réception de la ligne de commande
        }
        bin.commandLine[bin.size] = NULL; // Terminaison du tableau des lignes de commande avec NULL

        // Récupération de la machine la moins chargée
        int torank = lazyone();
        // Transfert de la tâche
        bridge(torank, &bin);

        return;
    }

    // Si la machine locale n'est pas en mode veille
    // Réception de la tâche transférée depuis un autre machine
    MPI_Recv(&procs[free_pos], sizeof(Process), MPI_BYTE, MPI_ANY_SOURCE, TRANSFER, MPI_COMM_WORLD, &status);
    int sender = status.MPI_SOURCE;  // Identification de la machine expéditrice
    int size = procs[free_pos].size; // Taille de la tâche transférée
    procs[free_pos].commandLine = (char **)malloc(sizeof(char *) * size+1); // Allocation de mémoire pour les lignes de commande de la tâche

    // Réception de chaque ligne de la ligne de commande de la tâche
    for(int i = 0; i < size; i++){  
    MPI_Probe(sender, CMD, MPI_COMM_WORLD, &status); // Sonde pour connaître la taille du message à recevoir
    MPI_Get_count(&status, MPI_CHAR, &len); // Obtention de la taille du message
    procs[free_pos].commandLine[i] = (char*) malloc(sizeof(char)*len); // Allocation de mémoire pour stocker la ligne de commande
    MPI_Recv(procs[free_pos].commandLine[i], len, MPI_CHAR, sender, CMD, MPI_COMM_WORLD, &status); // Réception de la ligne de commande
    }
    procs[free_pos].commandLine[size] = NULL; // Terminaison du tableau des lignes de commande avec NULL
    
    // Attribution de la tâche à un emplacement libre dans le tableau des tâches locales et démarrage de la tâche
    // Vérification de la réussite de l'attribution de la tâche
    char str_exec[256];
    if(start_task(free_pos) == 1){
        MPI_Send(&procs[free_pos].global_pid, 1, MPI_INT, 0, TRANSFERED, MPI_COMM_WORLD); // Envoi d'un signal de succès à la machine maître
        
        sprintf(str_exec, "[EXEC] La machine %d a exécuté la commande %s\n", rank, procs[free_pos].commandLine[0]);
        MPI_Send(str_exec, strlen(str_exec)+1, MPI_CHAR, 0, INFO, MPI_COMM_WORLD);
    } else {
    
    	sprintf(str_exec, "[FAIL] La machine %d n'a pas pu exécuter la commande %s\n", rank, procs[free_pos].commandLine[0]);
        MPI_Send(str_exec, strlen(str_exec)+1, MPI_CHAR, 0, INFO, MPI_COMM_WORLD);
        MPI_Send(&procs[free_pos].global_pid, 1, MPI_INT, 0, ERROR, MPI_COMM_WORLD); // Envoi d'un signal d'erreur à la machine maître
        procs[free_pos].pid = 0; // Réinitialisation de l'identifiant de processus
        procs[free_pos].global_pid = -1; // Réinitialisation de l'identifiant de processus global
        procs[free_pos].size = 0; // Réinitialisation de la taille de la tâche
        freer(procs[free_pos].commandLine); // Libération de la mémoire allouée pour stocker les lignes de commande 
    }
}

int start_task(int pos){
    // Vérifier si le fichier exécutable de la tâche existe
    FILE* file = fopen(procs[pos].commandLine[0], "r");
    if (!file) {
        printf("Start: file doesn't exist %s\n", procs[pos].commandLine[0]);
        return 0; // Le fichier exécutable de la tâche n'existe pas, échec de démarrage
    } else {
        fclose(file);
    }

    // Lancer la tâche dans un processus fils en utilisant execvp()
    int pid = fork();
    int s;
    if (pid == 0){ // Processus fils
        if (execvp(procs[pos].commandLine[0], procs[pos].commandLine) == -1){
            printf("ERR! : execvp failed\n");
            exit(1);
        }
    }

    // Attendre pendant un court laps de temps pour vérifier si le processus fils a démarré correctement
    sleep(3);
    if (waitpid(pid, &s, WNOHANG) == pid) {
        // Le processus fils n'a pas démarré correctement, execvp à échoué, renvoyer un échec de démarrage
        return 0;
    } 
    
    // Le processus fils a démarré, enregistrer son PID dans la structure de processus et renvoyer un succès au démarrage
    procs[pos].pid = pid;
    return 1;
}

int start(char **argv, int first, int gpid){

    // Mise à jour des informations sur les charges des machines du réseau
    update();
    
    // Recherche de la machine la moins chargée dans le réseau
    int torank = lazyone();
    
    // Si aucune machine disponible n'est trouvé, échec au démarrage de la tâche
    if (!torank){
        return 0;
    }

    // Test si le fichier exécutable de la tâche existe, sinon échec au démarrage de la tâche
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        printf("Start: file doesn't exist\n");
        return 0;
    } else {
        fclose(file);
    }

    // Calcul de la taille de la ligne de commande
    int size = 0;
    for(int i = first; argv[i] != NULL; i++){  
        size += 1;
    }
    
    // Envoi de la taille de la ligne de commande et de l'identifiant global du processus à la machine sélectionnée
    MPI_Send(&size, 1, MPI_INT, torank, START, MPI_COMM_WORLD);
    MPI_Send(&gpid, 1, MPI_INT, torank, CMD, MPI_COMM_WORLD);

    // Envoi de chaque argument de la ligne de commande à la machine pour exécution
    for(int i = first; argv[i] != NULL; i++){  
        MPI_Send(argv[i], strlen(argv[i])+1, MPI_CHAR, torank, CMD, MPI_COMM_WORLD);
    }

    // Succès de démarrage de la tâche
    return 1;
}

void freer(char **argv){
    int i;
    // Parcours du tableau jusqu'à rencontrer un pointeur NULL
    for(i = 0; argv[i] != NULL; i++){
        free(argv[i]); // Libération de la mémoire allouée pour chaque chaîne de caractères
    }
    free(argv); // Libération de la mémoire du tableau lui-même
}

void rcv_start(){
    int len;
    int sender = status.MPI_SOURCE;
    char str[256];

    // Recherche d'une position libre dans le tableau des processus
    int free_pos = 0;
    for(int i = 0; i < NB_MAX_PROCS; i++){
        if (!procs[i].pid){
            free_pos = i;
            break;
        }
    }
    
    // Réception de la taille de la ligne de commande et de l'identifiant global de processus
    MPI_Recv(&procs[free_pos].size, 1, MPI_INT, sender, START, MPI_COMM_WORLD, &status);
    procs[free_pos].commandLine = (char **)malloc(sizeof(char *) * (procs[free_pos].size+1));
    MPI_Recv(&procs[free_pos].global_pid, 1, MPI_INT, sender, CMD, MPI_COMM_WORLD, &status);
    
    // Réception de chaque argument de la ligne de commande
    for(int i = 0; i < procs[free_pos].size; i++){  
        MPI_Probe(sender, CMD, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_CHAR, &len);
        procs[free_pos].commandLine[i] = (char *)malloc(sizeof(char)*len);
        MPI_Recv(procs[free_pos].commandLine[i], len, MPI_CHAR, sender, CMD, MPI_COMM_WORLD, &status);
    }

    procs[free_pos].commandLine[procs[free_pos].size] = NULL;

    sprintf(str, "[START] La machine %d a reçu la commande %s\n",rank, procs[free_pos].commandLine[0]);
    MPI_Send(str, strlen(str)+1, MPI_CHAR, 0, INFO, MPI_COMM_WORLD);


    // Si aucun processus n'est en cours d'exécution sur la machine locale (machine inactive)
    if (idle){
        // Démarrage immédiat de la tâche
        start(procs[free_pos].commandLine, 0, procs[free_pos].global_pid);
        freer(procs[free_pos].commandLine); // Libération de la mémoire allouée pour la ligne de commande
        procs[free_pos].pid = 0;
        procs[free_pos].global_pid = -1;
        procs[free_pos].size = 0;
        return;
    }

    char str_exec[256];
    // Démarrage de la tâche en arrière-plan
    if(start_task(free_pos) == 1){
        // Envoi d'un accusé de réception de transfert réussi
        MPI_Send(&procs[free_pos].global_pid, 1, MPI_INT, 0, TRANSFERED, MPI_COMM_WORLD);

        sprintf(str_exec, "[EXEC] La machine %d a exécuté la commande %s\n", rank, procs[free_pos].commandLine[0]);
        MPI_Send(str_exec, strlen(str_exec)+1, MPI_CHAR, 0, INFO, MPI_COMM_WORLD);
    } else {
        sprintf(str_exec, "[FAIL] La machine %d n'a pas pu exécuter la commande %s\n", rank, procs[free_pos].commandLine[0]);
        MPI_Send(str_exec, strlen(str_exec)+1, MPI_CHAR, 0, INFO, MPI_COMM_WORLD);
        // En cas d'échec de démarrage de la tâche, réinitialisation de la structure de processus
        procs[free_pos].pid = 0;
        procs[free_pos].global_pid = -1;
        procs[free_pos].size = 0;
        freer(procs[free_pos].commandLine); // Libération de la mémoire allouée pour la ligne de commande
    } 

    return;
}

void end_task(int pos){
    int s;
    // Vérification si le processus associé à la tâche a terminé son exécution
    if (waitpid(procs[pos].pid, &s, WNOHANG) != procs[pos].pid) {
        // Envoi d'un signal SIGKILL pour forcer la terminaison du processus
        kill(procs[pos].pid, SIGKILL);
    } 

    // Réinitialisation des champs associés à la tâche dans la structure de processus
    procs[pos].pid = 0;
    procs[pos].global_pid = -1;
    procs[pos].size = 0;

    // Libération de la mémoire allouée pour la ligne de commande de la tâche
    freer(procs[pos].commandLine);
}

void gps(int opt){

    // Envoi d'une demande d'informations aux autres machines
    int nb_proc_not_idle = 0;
    for (int i = 1; i < nb_proc; i++){
        if (workers[i] != 0){
            MPI_Send(&opt, 1, MPI_INT, i, GPS, MPI_COMM_WORLD);
            nb_proc_not_idle++;
        }
    }
    
    

    // Réception des confirmations des autres machines

    int flag;
    int cpt = 0; // nombre total de processus en cours d'éxécution
    int curr = 0; // nombre d'affichage reçu
    int nb_gps = 1;
    int tmp;
    int len;
    
    char **gps_res = (char **)malloc(sizeof(char *) * ((NB_MAX_PROCS*nb_proc)+1));
    
    while(nb_gps <= nb_proc_not_idle){
    	// Vérification de la présence de messages GPS_INFO
    	MPI_Iprobe(MPI_ANY_SOURCE, GPS_INFO, MPI_COMM_WORLD, &flag, &status); 
        if (flag){
       	    // Réception d'un message d'information
            MPI_Get_count(&status, MPI_CHAR, &len); // Récupération de la longueur du message
            gps_res[curr] = (char *)malloc(sizeof(char)*len);
            // Réception du message GPS_INFO
            MPI_Recv(gps_res[curr], len, MPI_CHAR, status.MPI_SOURCE, GPS_INFO, MPI_COMM_WORLD, &status);             
            curr++;
        }
        
    	// Vérification de la présence de messages GPS
        MPI_Iprobe(MPI_ANY_SOURCE, GPS, MPI_COMM_WORLD, &flag, &status); 
        if (flag){
            // Réception du message GPS
            MPI_Recv(&tmp, 1, MPI_INT, status.MPI_SOURCE, GPS, MPI_COMM_WORLD, &status);
            cpt += tmp;
            nb_gps++;
        }
    }
    

    // reception des derniers GPS_INFO
    while(curr < cpt){
    	// Réception du message GPS_INFO
	MPI_Probe(MPI_ANY_SOURCE, GPS_INFO, MPI_COMM_WORLD, &status); 
        
	// Réception d'un message d'information
	MPI_Get_count(&status, MPI_CHAR, &len); // Récupération de la longueur du message
	gps_res[curr] = (char *)malloc(sizeof(char)*len);
	// Réception du message GPS_INFO
	MPI_Recv(gps_res[curr], len, MPI_CHAR, status.MPI_SOURCE, GPS_INFO, MPI_COMM_WORLD, &status);             
	curr++;
    }
    
    gps_res[curr] = NULL;
    
    // Affichage de l'en-tête en fonction de l'option opt
    if (opt){
        printf("Rank  |     HostName     |  Pid  |  Gpid  |  Programme  |  %%CPU (ticks) |  %%MEM (VmRSS) | Charge \n");
    } else {
        printf("Rank  |     HostName     |  Pid  |  Gpid  | Programme | Charge \n");
    }
    
    // Envoi de chaque argument de la ligne de commande à la machine pour exécution
    for(int i = 0; gps_res[i] != NULL; i++){  
        printf("%s", gps_res[i]);
    }
    
    
    freer(gps_res);
    
    
}

void rcv_gps(int opt){
    int cpu;
    int mem;

    int cpt = 0;

     // Si la machine est inactive
    if (idle){
        // Envoi du nombre de processus affichés à la machine maître
        MPI_Send(&cpt, 1, MPI_INT, 0, GPS, MPI_COMM_WORLD);
        return;
    }
    
    // Parcours des processus en cours d'exécution sur la machine locale
    for(int i = 0; i < NB_MAX_PROCS; i++){
        int s;
        int pid = procs[i].pid;
        
        // Vérification si le processus est en cours d'exécution
        if (pid != 0 && (waitpid(pid, &s, WNOHANG) != pid)){
            cpt++;
            
            // Affichage des informations sur le processus en fonction de l'option opt
            char str[256];
            if (opt == 0){
            	sprintf(str, "%d\t%s\t    %d    %d\t%s\t%d\n", rank, hostname, procs[i].pid, procs[i].global_pid, procs[i].commandLine[0], load_prec);
    		    MPI_Send(str, strlen(str)+1, MPI_CHAR, 0, GPS_INFO, MPI_COMM_WORLD);
                
            }else{
                cpu = cpu_proc(pid);
                mem = mem_proc(pid);
                sprintf(str, "%d\t%s\t    %d    %d\t%s\t\t%d\t\t%d\t%d\n", rank, hostname, procs[i].pid, procs[i].global_pid, procs[i].commandLine[0], cpu, mem, load_prec);
    		    MPI_Send(str, strlen(str)+1, MPI_CHAR, 0, GPS_INFO, MPI_COMM_WORLD);
            }
        }
        else{
            // Si le processus n'est plus en cours d'exécution, le terminer
            if (pid != 0){
                end_task(i);
            }
        }
    }

     // Si aucun processus n'est en cours d'exécution 
    if (!cpt){
    	char str2[256];
        sprintf(str2, "La machine %s avec le rang %d n'a pas encore de programme en cours d'éxécution et sa charge est de %d\n", hostname, rank, load_prec);
        MPI_Send(str2, strlen(str2)+1, MPI_CHAR, 0, GPS_INFO, MPI_COMM_WORLD);
        // Envoi du nombre de processus affichés à la machine maître
        cpt++; // message default
    	MPI_Send(&cpt, 1, MPI_INT, 0, GPS, MPI_COMM_WORLD);
    	return;
    }

    // Envoi du nombre de processus affichés à la machine maître
    MPI_Send(&cpt, 1, MPI_INT, 0, GPS, MPI_COMM_WORLD);
}

int killer(int sig, int torank, int pid){
    int res = 0;
    // Envoi du signal de terminaison à toutes les machines esclaves
    for (int i = 1; i < nb_proc; i++){
        MPI_Send(&pid, 1, MPI_INT, torank, KILL, MPI_COMM_WORLD);
        MPI_Send(&sig, 1, MPI_INT, torank, SIG, MPI_COMM_WORLD);
    }

    // Réception de la réponse de la machine esclave cible
    MPI_Recv(&res, 1, MPI_INT, torank, KILL, MPI_COMM_WORLD, &status); 
    return res;
}

void rcv_kill(int sig, int pid){
    int send;
    int master = 0;
    char str[256];

    // Si la machine est inactive, signaler que la terminaison a échoué en envoyant 0
    if (idle){
        send = 0;
        MPI_Send(&send, 1, MPI_INT, master, KILL, MPI_COMM_WORLD);
        return;
    }

    // Recherche du processus dans la liste des processus en cours d'exécution
    int pos = -1;
    for(int i = 0; i < NB_MAX_PROCS; i++){
        if (procs[i].global_pid == pid){
            pos = i;
            break;
        }
    }
    
    // Si le processus n'est pas trouvé, signaler que la terminaison a échoué en envoyant 0
    if (pos == -1){
        send = 0;
        MPI_Send(&send, 1, MPI_INT, master, KILL, MPI_COMM_WORLD);
        return;
    }

    // Vérifier si le processus est toujours en cours d'exécution
    int s;
    if (waitpid(procs[pos].pid, &s, WNOHANG) != procs[pos].pid) {
        // Terminer le processus en envoyant le signal approprié
        kill(procs[pos].pid, sig);
        sprintf(str, "[GKILL] La machine %d a envoyée le signal %d au processus dont le pid est %d et le gpid est %d\n", rank, sig, procs[pos].pid, procs[pos].global_pid);
        MPI_Send(str, strlen(str)+1, MPI_CHAR, 0, INFO, MPI_COMM_WORLD);
    }
    

    // Marquer le processus comme terminé, en réinitialisant la structure du processus
    procs[pos].pid = 0;
    procs[pos].global_pid = -1;
    procs[pos].size = 0;
    freer(procs[pos].commandLine);
    
    // Envoyer 1 à la machine maître pour indiquer que la terminaison a réussi
    send = 1;
    MPI_Send(&send, 1, MPI_INT, master, KILL, MPI_COMM_WORLD);
}

void claim(){
    int val = 1;
    char str[256];

    // Trouver la machine en surcharge la plus chargée
    int dest = hardworker();
    // Si une machine en surcharge est trouvée
    if (dest){
        // Envoyer une demande de réclamation à la machine en surcharge
        MPI_Send(&val, 1, MPI_INT, dest, CLAIM, MPI_COMM_WORLD);

        // Envoyer un message d'information à la machine maître pour notifier la demande de réclamation
        sprintf(str, "[CLAIM] La machine avec le rank %d demande une tache à %d\n", rank, dest);
        MPI_Send(str, strlen(str)+1, MPI_CHAR, 0, INFO, MPI_COMM_WORLD);
    }
}

void receive_claim(){
    int val;
    // Recevoir une demande de réclamation de tâche
    MPI_Recv(&val, 1, MPI_INT, MPI_ANY_SOURCE, CLAIM, MPI_COMM_WORLD, &status);
    // Vérifier si la machine n'est pas inactive ou si elle a au moins une tâche disponible
    if (!idle && available_proc()){
        // Transférer une tâche à la machine demandant la réclamation
        work_transfer(status.MPI_SOURCE, 0);
    }
}

void balancer(){
    update();

    // Vérifier si le délai pour la publication de la charge est écoulé
    time_t current_time = time(NULL);
    int diff = (int) difftime( current_time, timer );
    if (diff >= delay){
        // Publier la charge actuelle
        publish_load();

        // Évaluer l'état de charge de la machine
        int state = state_load();
        int per;
        int charge;
        char str[256];

        switch(state){
            case 1:
                // Cas de surcharge
                charge = load();
                per = (((charge-(float)netload)/(float)netload)*100);
                sprintf(str, "[SURCHARGE] %d est en surcharge avec une charge (%d) supérieur à la charge moyenne (%d) de %d %%\n", rank, charge, netload, per);
                MPI_Send(str, strlen(str)+1, MPI_CHAR, 0, INFO, MPI_COMM_WORLD);
                // Transférer des tâches à d'autres machines
                work_transfer(0, 0);
                break;

            case 0:
                // Cas d'équilibre
                break;

            case -1:
                // Cas de sous-charge
                charge = load();
                per = (((charge-(float)netload)/(float)netload)*100);
                sprintf(str, "[SOUSCHARGE] %d est en souscharge avec une charge (%d) inférieur à la charge moyenne (%d) de %d %%\n", rank, charge, netload, per);
                MPI_Send(str, strlen(str)+1, MPI_CHAR, 0, INFO, MPI_COMM_WORLD);
                // Demander des tâches à d'autres machines
                claim();
                break;
        }
    }
}

void balancer_test_surcharge(){
    int state = 1; // Simulation de l'état de surcharge

    switch(state){
        case 2:
             // Cas où la machine est en équilibre
            break;

        case 1:
            // Cas de surcharge : transférer des tâches à d'autres machines
            work_transfer(0, 0);
            break;

        case 0:
            // Cas où il n'y a rien à faire
            break;

        case -1:
            // Cas de sous-charge
            break;   
    }
}

void balancer_test_souscharge(){
    int state = 0; // État initial : rien à faire

    // Simulation d'une sous-charge si la charge actuelle est inférieure ou égale à 100
    if (workers[rank] <= 100){
        state = -1; // Sous-charge forcée
    }

    switch(state){
        case 2:
            // Cas où la machine est en équilibre
            break;

        case 1:
            // Cas de surcharge
            break;

        case 0:
            // Cas où il n'y a rien à faire
            break;

        case -1:
            // Cas de sous-charge : demander des tâches aux autres machines
            printf("souscharge........................................................................CLAIM\n");
            claim();
            
            // Augmenter artificiellement la charge pour limiter le nombre de demandes de tâches
            workers[rank] += 20;
            break;
    }
}



