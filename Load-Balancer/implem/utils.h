#ifndef UTILS_H
#define UTILS_H

/*
 * readLine: - Lit une ligne depuis l'entrée standard et la stocke dans un tableau de chaînes de caractères.
 *
 * Cette fonction lit une ligne depuis l'entrée standard et la divise en mots en utilisant des délimiteurs
 * tels que l'espace, la tabulation et le saut de ligne. Les mots sont ensuite stockés dans un tableau de chaînes de
 * caractères.
 *
 * @return: Un pointeur vers un tableau de chaînes de caractères contenant les mots de la ligne lue, suivi d'un pointeur NULL.
 *          En cas d'erreur de lecture, la fonction renvoie NULL.
 */
char** readLine();

/*
 * printLine: - Affiche le contenu d'un tableau de chaînes de caractères.
 * @line: Pointeur vers un tableau de chaînes de cractères.
 *
 * Cette fonction affiche le contenu d'un tableau de chaînes de caractères sur la sortie standard.
 * Chaque chaîne de caractères du tableau est affichée séparément, suivie d'un espace. La fonction se termine
 * lorsque le pointeur NULL marquant la fin du tableau est rencontré.
 */
void printLine(char** line);

/*
 * clearLine: - Libère la mémoire allouée pour un tableau de chaînes de caractères.
 * @line: Pointeur vers un tableau de chaînes de cractères.
 *
 * Cette fonction libère la mémoire allouée pour chaque chaîne de caractères dans le tableau
 * ainsi que pour le tableau lui-même. Elle prend en charge la libération de la mémoire allouée
 * dynamiquement pour le tableau de chaînes de caractères et chacune de ses chaînes de caractères.
 */
void clearLine(char ** line);

#endif