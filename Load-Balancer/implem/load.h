#ifndef LOAD_H
#define LOAD_H

#define MIN_LOAD 0.65
#define MAX_LOAD 1.35

/*
 * load: - Calcule la charge système en pourcentage.
 *
 * Cette fonction lit la charge système à partir du fichier "/proc/loadavg" et calcule
 * la charge en pourcentage. Le fichier "/proc/loadavg" contient des informations sur
 * la charge système moyenne sur différentes périodes de temps.
 *
 * @return: La charge système en pourcentage.
 */
int load();

/*
 * publish_load: - Publie la charge système actuelle aux autres machines.
 *
 * Cette fonction publie la charge système actuelle aux autres machines dans le système répartie.
 * Elle compare également le temps écoulé depuis la dernière publication avec le délai spécifié
 * pour déterminer si une nouvelle publication est nécessaire.
 */
void publish_load();

/*
 * state_load: - Détermine l'état de charge de la machine locale par rapport au réseau.
 *
 * Cette fonction détermine l'état de charge de la machine locale par rapport au réseau en utilisant
 * la charge locale et la charge réseau globale. Elle compare la charge locale avec les seuils
 * définis par MAX_LOAD et MIN_LOAD multipliés par la charge réseau globale.
 *
 * @return: Un entier représentant l'état de charge de la machine locale par rapport au réseau
 *         1 si la machine locale est surchargée par rapport au réseau,
 *         -1 si la machine locale est sous-chargée par rapport au réseau,
 *         0 si la machine locale est dans une plage de charge acceptable par rapport au réseau.
 */
int state_load();

/*
 * update: - Met à jour les informations sur la charge des machines et la charge réseau globale.
 *
 * Cette fonction met à jour les informations sur la charge des machines et la charge réseau globale
 * en recevant des messages de charge des autres machines. Elle utilise une approche asynchrone pour
 * vérifier la réception de messages pendant un certain nombre d'itérations, déterminé par le délai multiplié
 * par un facteur de 10. Les messages reçus sont traités et les informations sur la charge des machines
 * sont mises à jour en conséquence.
 */
void update();

/*
 * available_proc: - Vérifie la disponibilité d'au moins un processus.
 *
 * Cette fonction vérifie la disponibilité des processus en parcourant le tableau de processus
 * et en recherchant au moins un processus dont l'identifiant n'est pas égal à zéro. 
 *
 * @return: 1 si au moins un processus est disponible, 0 sinon.
 */
int available_proc();

/*
 * busy_proc: - Détermine le processus le plus chargé.
 *
 * Cette fonction parcourt le tableau de processus pour déterminer le processus le plus chargé.
 * Elle examine chaque processus dont l'identifiant n'est pas égal à zéro, puis appelle la fonction
 * cpu_proc pour obtenir la charge CPU de chaque processus. Elle compare ensuite ces charges pour
 * trouver le processus ayant la charge CPU maximale.
 *
 * @return: La position du processus le plus chargé dans le tableau de processus, ou -1 si aucun processus n'est trouvé.
 */
int busy_proc();

/*
 * cpu_proc: - Calcule l'utilisation CPU d'un processus spécifié par son PID.
 * @pid: L'identifiant du processus dont on souhaite calculer l'utilisation CPU.
 *
 * Cette fonction calcule l'utilisation CPU d'un processus en fonction de son PID.
 * Elle récupère les statistiques du processus à partir du fichier /proc/pid/stat,
 * extrait les valeurs de temps utilisateur (utime) et système (stime), puis calcule
 * l'utilisation CPU totale en additionnant ces valeurs.
 *
 * @return: L'utilisation CPU du processus, en ticks horloge, spécifié par son PID, ou -1 en cas d'erreur.
 */
int cpu_proc(int pid);

/*
 * mem_proc: - Calcule la quantité de mémoire utilisée par un processus spécifié par son PID.
 * @pid: L'identifiant du processus dont on souhaite calculer la quantité de mémoire utilisée.
 *
 * Cette fonction calcule la quantité de mémoire utilisée par un processus en fonction de son PID.
 * Elle récupère les informations de mémoire du processus à partir du fichier /proc/pid/status,
 * recherche la ligne contenant l'information sur la mémoire résidente (VmRSS) et extrait la
 * quantité de mémoire utilisée à partir de cette ligne.
 *
 * @return: La quantité de mémoire, en kilobytes, utilisée par le processus spécifié par son PID, ou -1 en cas d'erreur.
 */
int mem_proc(int pid);

/*
 * init_load_master: - Initialise les charges des machines sur le maître.
 *
 * Cette fonction initialise les charges des machines sur le maître en recevant les
 * charges des machines via des communications MPI. Elle attend la réception
 * de la charge de chaque machine, en utilisant le tag LOAD, et stocke les charges dans le
 * tableau des machines, à l'exception du maître lui-même.
 */
void init_load_master();

/*
 * init_load_worker: - Initialise les charges des machines esclaves.
 *
 * Cette fonction initialise les charges des machines esclaves en calculant
 * sa propre charge à l'aide de la fonction load(), puis en envoyant cette charge à tous
 * les autres machines via des communications MPI. Ensuite, elle reçoit les charges des autres
 * machines esclaves et les stocke dans le tableau des machines.
 */
void init_load_worker();

#endif
