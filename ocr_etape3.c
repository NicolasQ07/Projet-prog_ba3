/**
  ocr_etapes3.c

  Etape 3 du Procedure OCR

  Dependancy
  © Tetyana Kulakova, 2026, GNU licence
  28.10.2026, v.1827365.2

  Revision
  28.10.2026, initial release

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lecture_entrees.c"
#include "lecture_entrees.h"
#include "main.c"
#include "sauvegarde_resultat.c"
#include "sauvegarde_resultat.h"

/* unsigned int width, height;
 ... fread width and height, check they are in 10..100 ...
size_t n = (size_t)width * height;
unsigned char *Cell = malloc(n);
if (Cell == NULL) {  report error  }
if (fread(Cell, 1, n, fp) != n) { not enough pixels -> error }
...
free(Cell); */

// Cell == [0,1,....1,1,0][...] ([75][75])

bin_to_array (){
  /**
  bin_to_array

Converts a binary bitmap to a array.

  Input
    DigitBitmap.bin
  Return
    return: DigitBitmap[][]

*/

}

num_checker (unsigned char* Cell, int32_t* DigitBitmap  ){
  /**
  num_checker

  Passes a bitmap corresponding to a given digit all over the current cell,
  determines the position and value of the highest percentage cell-bitmap match.

  Input
    unsigned char* Cell: pointer to an array (typically 75x75) containing a bitmap of the current cell
    unsigned char* DigitBitmap: pointer to an array (32x32) containing a bitmap of the number being verified
  Outputs
    d = digit (0, 1 or -1 for vide)
    p = percent match to d
    best_pos = position of closest match for d
    (digit, similarity percentage, position)
  Return
    return: ???

*/

}
// !!! What to do if two different numbers hit the required percent?

num_confirmer (){
  /**
  num_confirmer

  Compares the percentage matches of a given cell to different bitmaps,
  returns an error and terminates the program if no matches or more than one match.

  Input
    d = digit (0, 1 or -1 for vide)
    p = percent match to d
    best_pos = position of closest match for d
  Outputs
    CellValue.txt : File containing the value of the number
    syntax: d:'digit', percent% retour à la ligne
    (digit contained in the current cell, as well as its similarity percentage)
  Return
    return: error code ( not sure yet, to be specified later)

*/
}
