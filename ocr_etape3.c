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
#include "lecture_entrees.h"
#include "sauvegarde_resultat.h"
#include "FontRasterized_0_1.h"
//#include "lecture_entrees.c"
//#include "main.c"
//#include "sauvegarde_resultat.c"


/* unsigned int width, height;
 ... fread width and height, check they are in 10..100 ...
size_t n = (size_t)width * height;
unsigned char *Cell = malloc(n);
if (Cell == NULL) {  report error  }
if (fread(Cell, 1, n, fp) != n) { not enough pixels -> error }
...
free(Cell); */

// Cell == [0,1,....1,1,0][...] ([75][75])

unsigned char GetCellBit(const unsigned char *cell, int width, int line, int col){
  /**
  GetCellBit

  Takes a cell array and reads a single pixel at position determined by line and column number as well as the cell's width.

  Input
    cell as a 1D array, from ReadCell in lecture_entrees.c
  Return
    the value of the pixel at a given position.
  */

  unsigned char cbit = cell[width*line+col];
  return cbit;

}

void PrintCell (const unsigned char *cell, int width, int height){
  /**
  Print Cell
  (test)

  Takes a cell array, reads all it's pixels and prints them as a grid.

  Input
    cell as a 1D array
  Output
    printed vision of the current cell (* for 1, . for 0)
  Return
    N/A
  */

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++){
      if (GetCellBit(cell, width, i, j) == 1){
        printf("*");
      }
      else if (GetCellBit(cell, width, i, j) == 0){
        printf(".");
      }
      //no need for other cases because verified by ValidatePixels;
    }
    printf("\n");
  }
  return;
}

unsigned char GetDigitBitmapBit(int digit, int line, int col){
  /**
  GetDigitBitmapBit

  Takes a digit, accesses the corresponding digit bitmap,
  and finds the bit value at a specific line and column position.

  Input
    digit: 1 or 0
  Return
    the value of the pixel at a given position.
  */

  uint32_t row = DigitBitmap[line][col];

  //rem. columnn 0 is the leftmost pixel (most significant bit in little Endian)
}

int    CountMatches(const unsigned char *cell, int width, int digit, int offX, int offY){
  /**
  CountMatches

  Counts the number of matches between a given cell and a given digit,
  at a certain position determined by a given offset.

  Input
    cell (as an array), digit (0 or 1), offset X and Y (in pixels)
  Return
    the percentage match of the chosen part of the cell to the chosen digit at that position.
  */
}

double BestMatchPercent(const unsigned char *cell, int width, int height, int digit){
  /**
  BestMatchPercent

  Takes a cell, along with its dimensions, and determines the digit which it matches best,
  then writes the result in a seperate file.

  Input
    cell (as an array), the cell's width and height as ints, digit (0 or 1)
  Return
    the digit which best matches the cell;
  */
}
double WhitePercent(const unsigned char *cell, int width, int height){
  /**
  White Percent

  Takes a cell, determines what percentage of its pixels are white;

  Input
    cell (as an array), its width and height as ints.
  Return
    the percentage of white pixels in a cell;
  */
}
int RecognizeCell(/* cell, size, 3 thresholds, char *digit, double *percent */){
  /**
  RecognizeCell

  Takes a cell, along with its size, and the 3 percentage thresholds for a 1, a 0 or "vide":

  Input

  Return
    the digit which best matches the cell;
  */
}

