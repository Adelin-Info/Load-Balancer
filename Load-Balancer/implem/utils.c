#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024 

char** readLine() {
  char** line = NULL; // Pointeur vers le tableau de chaînes de caractères
  int sizeLine = 0; // Taille actuelle du tableau de chaînes de caractères
  char buffer[MAX_LINE_LENGTH]; // Tampon pour stocker la ligne lue depuis l'entrée standard

  // Lire toute la ligne en utilisant fgets
  if (fgets(buffer, MAX_LINE_LENGTH, stdin) == NULL){
    return NULL; // Gérer l'erreur si la lecture échoue
  }

  // Tokeniser la ligne en utilisant strtok
  char* word = strtok(buffer, " \t\n"); // Les délimiteurs sont l'espace, la tabulation et le saut de ligne

  // Parcourir chaque mot de la ligne
  while (word != NULL) {
    // Augmenter la taille du tableau de chaînes de caractères
    line = realloc(line, sizeof(char*) * (sizeLine + 1));

    // Allouer de la mémoire pour le mot
    line[sizeLine++] = malloc(sizeof(char) * (strlen(word)+1));

    // Copier le mot dans le tableau de chaînes de caractères
    strcpy(line[sizeLine - 1], word);

    // Obtenir le mot suivant en utilisant strtok
    word = strtok(NULL, " \t\n");
  }

  // Augmenter la taille du tableau de chaînes de caractères pour ajouter un pointeur NULL marquant la fin du tableau
  line = realloc(line, sizeof(char*) * (sizeLine + 1));
  line[sizeLine++] = NULL;

  return line;
}

void printLine(char** line) {
  printf("Affichage de la commande : \n");

  int i = 0; // Variable pour l'itération à travers le tableau
  // Parcourir le tableau de chaînes de caractères jusqu'à rencontrer un pointeur NULL
  while (line[i] != NULL) {
    printf("%s ", line[i]); // Afficher chaque chaîne de caractères suivie d'un espace
    i++; // Passer à la chaîne de caractères suivante dans le tableau
  }

  printf("\n");
}

void clearLine(char** line){
  // Parcourir le tableau de chaînes de caractères jusqu'à rencontrer un pointeur NULL
  for (int i = 0; line[i] != NULL; i++) {
    free(line[i]); // Libérer la mémoire allouée pour chaque chaîne de caractères
  }
  free(line); // Libérer la mémoire allouée pour le tableau de pointeurs de chaînes de caractères
}