#ifndef CLIENT_H_
#define CLIENT_H_

/**
 * master_update: - Mise à jour de la machine maître.
 *
 * Cette fonction est exécutée dans un thread pour mettre à jour les informations de la machine maître.
 * Elle surveille les messages entrants provenant des machines esclaves et met à jour les données en conséquence.
 * Les messages qu'elle traite comprennent les informations de charge de travail (LOAD), les transferts de processus (TRANSFERED) 
 * et les messages d'information (INFO).
 * Les informations sur la charge de travail sont utilisées pour mettre à jour le tableau des machines (`workers`), 
 * tandis que les transferts de processus sont utilisés pour mettre à jour la table des identifiants globaux (`gpid`).
 * Les messages d'information sont écrits dans un fichier journal log.txt.
 */
void *master_update();

/*
 * CLI: - Interface en ligne de commande pour l'utilisateur, exécutée par la machine maitre
 *
 * Cette fonction gère une interface en ligne de commande pour l'utilisateur.
 * Elle permet de démarrer de nouvelles tâches (start), d'afficher les informations du réseau (gps), 
 * de tuer des processus par leur identifiant global (gkill), d'insérer de nouvelles machines 
 * dans le réseau (insert) et de supprimer des machines du réseau (remove).
 * La fonction utilise un thread pour mettre à jour les informations du réseau en arrière-plan.
 * Elle utilise également un mutex pour éviter les conflits d'accès aux données partagées.
 *
 * @return: 0 lorsque la fonction se termine
 */
int CLI();

#endif