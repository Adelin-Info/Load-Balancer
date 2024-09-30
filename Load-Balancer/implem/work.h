#ifndef WORK_H
#define WORK_H

#include "struct.h"

/*
 * work_transfer: - Transfère des tâches vers une autre machine.
 * @to: Le numéro de la machine destination où transférer les tâches, 0 si non spécifié.
 * @opt: L'option de transfert de tâches, 1 pour transférer toutes les tâches ou 0 pour seulement une tâche.
 *
 * Cette fonction transfère des tâches vers une autre machine. Si l'option opt est activée,
 * elle transfère toutes les tâches de la machine locale vers la machine spécifiée. Sinon, elle sélectionne
 * une tâche active de la machine locale, si disponible, et la transfère vers une autre machine moins chargée.
 * Elle envoie également des informations sur le transfert de tâches à la machine maître.
 */
void work_transfer(int to, int opt);

/*
 * bridge: - Transfère une tâche vers une autre machine.
 * @torank: L'identifiant de la machine destination où transférer la tâche.
 * @p: Un pointeur vers la structure de processus contenant la tâche à transférer.
 *
 * Cette fonction envoie une tâche spécifiée vers une autre machine identifiée par son identifiant torank.
 * Elle envoie d'abord la structure de processus p à l'autre machine, puis envoie chaque ligne de la
 * ligne de commande de la tâche. Enfin, elle libère la mémoire allouée pour stocker la ligne de commande.
 */
void bridge(int torank, Process *p);

/*
 * receive_transfer: - Réception et traitement du transfert de tâches.
 *
 * Cette fonction reçoit une tâche transférée depuis une autre machine et la traite. Si la machine locale
 * est en mode veille, il reçoit la tâche et la transfère à une autre machine moins chargée.
 * Sinon, il récupère la tâche et l'assigne à un emplacement libre dans le tableau des tâches locales.
 * Ensuite, il démarre la tâche et envoie un signal de succès ou d'erreur à la machine maître en fonction
 * du résultat de l'attribution de la tâche.
 */
void receive_transfer();

/*
 * start_task: - Démarrage d'une tâche.
 * @pos: La position de la tâche dans le tableau des tâches locales.
 *
 * Cette fonction démarre une tâche à partir de la ligne de commande spécifiée dans la structure
 * de processus à la position pos dans le tableau des tâches. Elle vérifie d'abord si le fichier
 * exécutable de la tâche existe. Si le fichier existe, elle lance la tâche dans un processus fils
 * en utilisant la fonction execvp(). Elle attend ensuite pendant un court laps de temps pour vérifier
 * si le processus fils a démarré correctement. Si le processus a démarré avec succès, son PID est
 * enregistré dans la structure de processus. Sinon, la fonction renvoie 0 pour indiquer un échec de
 * démarrage de la tâche.
 *
 * @return: 1 en cas de succès, 0 en cas d'échec de démarrage de la tâche.
 */
int start_task(int pos);

/*
 * start: - Démarrage d'une tâche sur une machine disponible.
 * @argv: Tableau d'arguments contenant la ligne de commande de la tâche.
 * @first: Indice du premier argument de la ligne de commande dans le tableau argv.
 * @gpid: Identifiant global du processus de la tâche.
 *
 * Cette fonction permet de démarrer une tâche sur une machine disponible dans le réseau. Elle
 * détermine d'abord la machine la moins chargée. Ensuite, elle
 * vérifie si le fichier exécutable de la tâche existe. Si le fichier existe, elle envoie la taille
 * de la ligne de commande et l'identifiant global de processus à la machine sélectionnée, puis envoie
 * chaque argument de la ligne de commande à la machine pour exécution.
 *
 * @return: 1 en cas de succès, 0 en cas d'échec de démarrage de la tâche.
 */
int start(char *argv[], int first, int gpid);

/*
 * freer: - Libération de la mémoire allouée pour un tableau de chaînes de caractères.
 * @argv: Tableau de chaînes de caractères à libérer.
 *
 * Cette fonction libère la mémoire allouée pour un tableau de chaînes de caractères. Elle parcourt
 * le tableau jusqu'à rencontrer un pointeur NULL, puis libère la mémoire allouée pour chaque chaîne
 * de caractères et enfin libère la mémoire du tableau lui-même. 
 */
void freer(char **argv);

/*
 * rcv_start: - Réception et traitement d'une commande de démarrage de tâche.
 *
 * Cette fonction reçoit une commande de démarrage de tâche en provenance d'une autre machine du réseau.
 * Elle reçoit d'abord la taille de la ligne de commande et l'identifiant global du processus envoyés
 * par la machine maitre. Ensuite, elle reçoit chaque argument de la ligne de commande et les stocke dans
 * une structure de processus libre. Si aucun processus n'est en cours d'exécution sur la machine locale,
 * elle démarre immédiatement la tâche en appelant la fonction start(). Sinon, elle appelle la fonction
 * start_task() pour démarrer la tâche en arrière-plan. Elle renvoie un accusé de réception à la machine
 * maitre indiquant le succès ou l'échec du démarrage de la tâche.
 */
void rcv_start();

/*
 * end_task: - Terminaison d'une tâche.
 * @pos: Position de la tâche dans le tableau des processus.
 *
 * Cette fonction termine une tâche en spécifiant sa position dans le tableau des processus.
 * Elle vérifie d'abord si le processus associé à la tâche a terminé son exécution. Si ce n'est
 * pas le cas, elle envoie un signal SIGKILL pour forcer la terminaison du processus. Ensuite, elle
 * réinitialise les champs associés à la tâche dans la structure de processus correspondante et libère
 * la mémoire allouée pour la ligne de commande de la tâche.
 */
void end_task(int pos);

/*
 * gps: - Afficher des informations sur les processus en cours d'exécution sur les différentes machines.
 * @opt: Option pour afficher les informations supplémentaires sur l'utilisation du CPU et de la mémoire.
 *
 * Cette fonction affiche des informations sur les processus en cours d'exécution sur les différentes machines du réseau.
 * Si l'option opt est activée, elle affiche également l'utilisation du CPU, en ticks horloge, et de la mémoire (VmRSS), en kilobyte, pour chaque processus.
 * Elle envoie ensuite une demande d'informations aux autres machines pour récupérer les informations sur leurs processus.
 * Enfin, elle reçoit les confirmations des autres machines indiquant le nombre de processus qu'ils ont affichés.
 */
void gps(int opt);

/*
 * rcv_gps: - Recevoir et afficher des informations sur les processus en cours d'exécution sur la machine.
 * @opt: Option pour afficher les informations supplémentaires sur l'utilisation du CPU et de la mémoire.
 *
 * Cette fonction reçoit les demandes d'informations sur les processus en cours d'exécution sur la machine et les affiche.
 * Elle vérifie d'abord les processus en cours d'exécution sur la machine locale, puis les affiche avec ou sans les informations
 * supplémentaires sur l'utilisation du CPU et de la mémoire en fonction de l'option opt. Si aucun processus n'est en cours
 * d'exécution ou si la machine est inactive, elle affiche un message approprié.
 */
void rcv_gps(int opt);

/*
 * killer: - Envoyer un signal de terminaison à un processus sur une machine spécifiée.
 * @sig: Le signal à envoyer.
 * @torank: Le rang de la machine où le processus cible est en cours d'exécution.
 * @pid: L'identifiant du processus cible à terminer.
 *
 * Cette fonction envoie un signal de terminaison à un processus sur une machine spécifiée et attend une réponse de celle-ci
 * pour confirmer si le signal a été envoyé avec succès.
 *
 * @return: 0 échec ou 1 succès.
 */
int killer(int sig, int torank, int gpid);

/*
 * rcv_kill: - Recevoir une demande de terminaison de processus.
 * @sig: Le signal de terminaison à envoyer au processus.
 * @pid: L'identifiant du processus à terminer.
 *
 * Cette fonction reçoit une demande de terminaison de processus pour un processus spécifié par son PID.
 * Elle cherche d'abord le processus dans la liste des processus en cours d'exécution. Si le processus est trouvé,
 * elle envoie le signal de terminaison et marque le processus comme terminé dans la liste des processus.
 * Elle répond ensuite à la machine maître pour indiquer si la terminaison a réussi ou non.
 */
void rcv_kill(int sig, int pid);

/*
 * claim: - Réclamer une tâche à une machine en surcharge.
 *
 * Cette fonction permet à une machine en sous-charge de réclamer une tâche à une machine en surcharge.
 * Elle détermine d'abord la machine en surcharge la plus chargée à laquelle réclamer la tâche.
 * Si une telle machine est trouvée, elle envoie une demande de réclamation à cette machine et envoie
 * également un message d'information à la machine maître pour notifier la demande de réclamation.
 */
void claim();

/*
 * receive_claim: - Recevoir et traiter une demande de réclamation de tâche.
 *
 * Cette fonction reçoit une demande de réclamation de tâche provenant d'une machine en sous-charge.
 * Si la machine courante n'est pas inactive ou si elle dispose de tâches disponibles,
 * elle transfère une tâche à la machine demandant la réclamation. Sinon, elle ne répond pas à la demande
 */
void receive_claim();

/*
 * balancer: - Equilibrer la charge entre les machines.
 *
 * Cette fonction vérifie si le délai pour la publication de la charge est écoulé.
 * Si oui, elle publie la charge actuelle, évalue l'état de charge de la machine,
 * et prend des mesures en fonction de cet état :
 * - En cas de surcharge, elle envoie des tâches à d'autres machines.
 * - En cas de sous-charge, elle demande des tâches à d'autres machines.
 * - Sinon, elle ne fait rien.
 */
void balancer();


// FONCTIONS CI-DESSOUS PERMETTENT DE TESTER UN COMPORTEMENT SPECIFIQUE


/*
 * balancer_test_surcharge: - Simuler le traitement en cas de surcharge.
 *
 * Cette fonction simule le traitement en cas de surcharge.
 * Elle appelle la fonction `work_transfer` pour transférer des tâches à d'autres machines.
 */
void balancer_test_surcharge();


/*
 * balancer_test_souscharge: - Simuler le traitement en cas de sous-charge.
 *
 * Cette fonction simule le traitement en cas de sous-charge.
 * Elle ajuste l'état de sous-charge en fonction de la charge actuelle,
 * puis décide des actions à entreprendre en fonction de cet état.
 * Dans ce cas, elle simule une sous-charge en appelant la fonction `claim`
 * pour demander des tâches aux autres machines et augmente artificiellement
 * la charge pour limiter le nombre de demandes de tâches dans ce cas de test.
 */
void balancer_test_souscharge();

#endif