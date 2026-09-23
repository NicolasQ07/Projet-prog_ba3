#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//input cell.bin

//size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
//first size is size of each element to be read in bytes, second is numher of elements

int main (int argc, const char *argv[]){
  if (argc != 5){
    printf("Paramètres excessifs ou insufisants");
    return -1;
  }
  //else unnecessary because of the return in if
  //argv[0] = nom du programme, [1] = chemin (Cell.bin), [2] = chaine (seuil pour vide, '90'), [3] = seuil 0, [4] = seuil 1

}


