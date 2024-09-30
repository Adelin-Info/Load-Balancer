#ifndef TAG_H
#define TAG_H

// Définition de l'ensemble des tags pour les communications MPI selon les actions a effectuées

/* valeur associée au lancement de processus */
#define START 0

/* valeur associée à l'affichage des informations sur le réseau */
#define GPS 1

/* valeur associée à l'insertion d'une machine */
#define INSERT 2

/* valeur associée au retrait d'une machine */
#define REMOVE 3

/* valeur associée à la réception des différentes informations envoyées par les machines */
#define GPS_INFO 4

/* valeur associée à une réclamation de processus */
#define CLAIM 5

/* valeur associée à un chargement */
#define LOAD 6

/* valeur associée à une terminaison de processus */
#define KILL 7

/* valeur associée à un transfert de processus*/
#define TRANSFER 8

/* valeur associée à un transfert effectué */
#define TRANSFERED 9

/* valeur associée à un signal */
#define SIG 10

/* valeur associée à une commande */
#define CMD 11

/* valeur associée à une confirmation */
#define ACK 12

/* valeur associée à une journalisation */
#define INFO 13

/* valeur associée à l'arrêt du programme */
#define END 14

/* valeur associée à l'erreur */
#define ERROR 15

#endif
