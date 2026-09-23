#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//input cell.bin

//size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
//first size is size of each element to be read in bytes, second is numher of elements

typedef struct celldim {
  unsigned int largeur;
  unsigned int hauteur;
} celldim_t;

FILE* openfile(const char * path){
  return fopen(path, "rb");
}

int main (int argc, const char *argv[]){
  if (argc != 5){
    printf("Paramètres excessifs ou insuffisants\n");
    return -1;
  }
  //else unnecessary because of the return in if
  //argv[0] = nom du programme, [1] = chemin (Cell.bin), [2] = chaine (seuil pour vide, "seuilV" '90'), [3] = seuil 0, [4] = seuil 1
  const char *path = argv[1];

  int seuilV = atoi(argv[2]);
  int seuil0 = atoi(argv[3]);
  int seuil1   = atoi(argv[4]);

  printf("File: %s\n", path);
  printf("Thresholds: empty=%d, 0=%d, 1=%d\n", seuilV, seuil0, seuil1);

  FILE* cellbin = openfile(path);
  if (cellbin == NULL){
    printf("Erreur: Cell.bin ne peut pas être ouvert.\n");
    return -1;
  }

  celldim_t celldim;
  size_t read_largeur = fread (&celldim.largeur, sizeof(unsigned int), 1, cellbin);
  if (read_largeur != 1){
    printf("Erreur lors de la lecture du fichier (largeur).\n");
    fclose (cellbin);
    return -1;
  }
  size_t read_hauteur = fread (&celldim.hauteur, sizeof(unsigned int), 1, cellbin);
  if (read_hauteur != 1){
    printf("Erreur lors de la lecture du fichier (hauteur).\n");
    fclose (cellbin);
    return -1;
  }

  if (celldim.largeur < 10 || celldim.largeur > 100){
    printf("Erreur: largeur invalide.\n");
    fclose (cellbin);
    return -1;
  }
  if (celldim.hauteur < 10 || celldim.hauteur > 100){
    printf("Erreur: hauteur invalide.\n");
    fclose (cellbin);
    return -1;
  }

  fclose (cellbin);
  return 0;
}


