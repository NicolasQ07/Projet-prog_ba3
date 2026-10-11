#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "lecture_entrees.h"
#include "sauvegarde_resultat.h"

//input cell.bin

//size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
//first size is size of each element to be read in bytes, second is numher of elements

int main (int argc, const char *argv[]){
  if (argc != 5){
    printf("Erreur %d: %s\n", ERR_ARGS, ErrorMsg(ERR_ARGS));
    return ERR_ARGS;
  }
  //else unnecessary because of the return in if
  //argv[0] = nom du programme, [1] = chemin (Cell.bin), [2] = chaine (seuil pour vide, "seuilV" '90'), [3] = seuil 0, [4] = seuil 1
  const char *path = argv[1];

  int seuilV = 0, seuil0 = 0, seuil1 = 0;

  int err = ReadSeuil(argv[2], &seuilV);
  if (err == OCR_OK) err = ReadSeuil(argv[3], &seuil0);
  if (err == OCR_OK) err = ReadSeuil(argv[4], &seuil1);
  if (err != OCR_OK){
    printf("Erreur %d: %s\n", err, ErrorMsg(err));
    return err;
  }

  printf("File: %s\n", path);
  printf("Thresholds: empty=%d, 0=%d, 1=%d\n", seuilV, seuil0, seuil1);

  unsigned char *Cell = NULL;
  uint32_t largeur = 0, hauteur = 0;

  err = ReadCell(path, &Cell, &largeur, &hauteur);
  if (err != OCR_OK){
    printf("Erreur %d: %s\n", err, ErrorMsg(err));
    return err;
  }

  printf("Cell: %u x %u = %lu pixels lus\n",
         largeur, hauteur, (unsigned long)largeur * hauteur);

  char   chiffre     = ' ';
  double pourcentage = 0.0;

  free(Cell);

  err = WriteCellValue(path, chiffre, pourcentage);
  if (err != OCR_OK){
    printf("Erreur %d: %s\n", err, ErrorMsg(err));
    return err;
  }

  printf("d:'%c', %.4f%%\n", chiffre, pourcentage);
  return OCR_OK;
}