#ifndef INOUT_H_
#define INOUT_H_

/*
 * insert_worker: - Insère les charges des machines dans une machine spécifiée.
 * @to: La machine de destination où insérer les charges des machines.
 *
 * Cette fonction insère les charges des machines dans une machine spécifiée, à condition que la charge de la machine destination soit nulle. 
 * Elle envoie les charges des machines via une communication MPI à la machine destination, en utilisant le tag INSERT.
 */
void insert_worker(int to);

/*
 * receive_insert: - Reçoit et insère les charges des machines provenant d'une machine spécifiée.
 *
 * Cette fonction reçoit et insère les charges des machines provenant d'une machine spécifiée.
 * Elle marque la machine comme actif, puis elle reçoit les charges des machines
 * via une communication MPI depuis n'importe quel machine utilisant le tag INSERT. Une fois les
 * charges reçues, elle met à jour la charge de la machine local en utilisant la fonction load(),
 * puis elle envoie cette nouvelle charge à tous les autres machines pour mise à jour.
 */
void receive_insert();

/*
 * remove_worker: - Supprime une machine spécifiée.
 * @rank_to_remove: La position de la machine dans le tableau workers.
 *
 * Cette fonction envoie un signal de suppression à une machine spécifiée pour indiquer
 * qu'elle doit être retirée. Elle envoie un entier avec
 * une communication MPI à la machine destination, utilisant le tag REMOVE.
 */
void remove_worker(int rank_to_remove);

/*
 * receive_remove: - Reçoit un signal de suppression et retourne en mode veille.
 *
 * Cette fonction reçoit un signal de suppression provenant d'une machine spécifiée.
 * Elle est appelée lorsqu'une machine doit être retirée. En réponse
 * à ce signal, la fonction effectue les opérations nécessaires pour mettre la
 * machine cible en mode veille avec la fonction gobacktosleep().
 */
void receive_remove();

/*
 * nb_workers: - Calcule le nombre de machines esclaves actives sur le réseau.
 *
 * Cette fonction parcourt le tableau des machines et compte le nombre de machines actives,
 * c'est-à-dire les machines dont la charge n'est pas nulle.
 *
 * @return: Le nombre de machines actives.
 */
int nb_workers();

/*
 * lazyone: - Détermine la machine esclave active la moins chargée.
 *
 * Cette fonction parcourt le tableau des machines pour trouver la machine ayant la charge la
 * plus faible parmi celles qui sont actives, à l'exception de la machine locale. 
 *
 * @return: L'identifiant de la machine la moins chargée parmi les machines actives, ou 0 si toutes les machines sont inactives.
 */
int lazyone();

/*
 * hardworker: - Détermine la machine esclave active la plus chargée.
 *
 * Cette fonction parcourt le tableau des machines pour trouver celle ayant la charge la
 * plus élevée parmi celles qui sont actives, à l'exception de la machine locale.
 *
 * @return: L'identifiant de la machine la plus chargée parmi les machines actives, ou 0 si toutes les machines sont inactives.
 */
int hardworker();

/*
 * gobacktosleep: - Met la machine en mode veille.
 *
 * Cette fonction met la machine en mode veille en réinitialisant l'état de la machine pour indiquer
 * qu'elle est inactive. Elle envoie également un signal de charge nulle à toutes les autres
 * machines pour mettre à jour leurs informations sur la charge de la machine. Enfin, elle transfère toutes
 * les tâches de la machine locale à une autre machine, si nécessaire, afin de libérer la machine locale.
 */
void gobacktosleep();

#endif