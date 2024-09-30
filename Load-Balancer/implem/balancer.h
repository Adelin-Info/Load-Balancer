#ifndef BALANCER_H_
#define BALANCER_H_

/*
 * Worker: - Exécutée par les machines esclaves.
 *
 * Cette fonction est responsable de la gestion des tâches côté machines esclaves.
 * Elle surveille les messages entrants provenant de la machine maître ou d'autres machines esclaves et agit en conséquence.
 * Elle traite les messages liés au démarrage de tâches (START), à la réception de demandes d'informations du réseau (GPS), 
 * à la réception de demandes de terminaison (GKILL), aux transferts de tâches (TRANSFER),
 * à l'insertion de nouvelles machines (INSERT), à la suppression de machines (REMOVE),
 * à la mise à jour des charges de travail (LOAD), et à la réception de demandes de tâches (CLAIM).
 * Elle met également à jour les informations sur la charge de travail et effectue un équilibrage de charge si nécessaire.
 *
 * @return: 0 lorsque la fonction se termine.
 */
int Worker();

#endif