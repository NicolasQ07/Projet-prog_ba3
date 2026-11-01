/**
    OCR.c

    Binairo OCR : it reads the contents of the cell (Cell.bin) and saves the correct
     digit in CellValue.txt

    Part 1 of the PPI project "OCR Binairo solver". The program is
    launched by Labview (BinairoSolver.vi) once for every cell of the grid:

        ./OCR '<path>/Cell.bin' <seuilVide> <seuil0> <seuil1>

    The 3 thresholds are integers in [0..100] (percent of similar pixels)

    Steps (project slide 17)
      1) read the header of Cell.bin (width, height) and validate it
      2) read the width x height pixels in one go into a malloc'ed array
      3) recognise '0', '1' or empty (in progress),
         main() currently saves the placeholder  d:' ', 0.0000%
      4) save the digit and its similarity in CellValue.txt, created in the
         same folder as Cell.bin

    File formats
      Cell.bin       uint32 width, uint32 height (little endian, as on the
                     host), then width x height unsigned char, one per pixel,
                     row after row: 0 = white, 1 = black
      CellValue.txt  d:'<digit>', <percent>%   followed by a new line

    Error handling:
      - every function returns OCR_OK (0) or a negative ERR code and
        forces its outputs to a predefined value when it fails
      - every value returned by the C library (fopen, fseek, ftell, fread,
        malloc, fprintf, fclose, strtol) is tested
      - inputs are validated before use: NULL pointers, threshold range,
        dimensions before computing width x height, exact file size, pixel
        values
      - a failing file/memory call is reported with perror() on stderr;
        main() then prints "Erreur <code>: <message>" on stderr and returns
        the code, which Labview reads as the exit status of OCR
      - for the different operators, on macOS/Linux the exit status is 
        truncated to 8 bits, so Labview sees 256 + code (ex. -110 -> 146);
        on Windows it stays -110.
        All the codes below remain distinct and non-zero after truncation

    Dependency
      C standard library only (C11).
      Build:  cc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -o OCR OCR.c
      Add -DDEBUG_HEADER to print the header of Cell.bin while reading it

    Authors
      Nicolas Quintane, Tetyana Kulakova

    29.10.2026, v3

    Revision
      23.09.2026, TK, reading of Cell.bin (OCR_Teutch.c)
      24.09.2026, NQ, v1: header and dimension validation
      11.10.2026, NQ, v2: thresholds, error messages, CellValue.txt
      29.10.2026, NQ, v3: everything merged in this single file,
                          documentation, error handling review
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>



 //Constants (were lecture_entrees.h and sauvegarde_resutat.h)


#define HEADER_SIZE      8L     // width + height, 2 x uint32_t in bytes  
#define DIM_MIN         10u     // accepted width/height, in pixels       
#define DIM_MAX        100u
#define DIGIT_SIZE      32u     // the digit bitmaps are 32 x 32 pixels   
#define SEUIL_MIN        0L     // accepted thresholds, in percent        
#define SEUIL_MAX      100L

#define OUT_FILE   "CellValue.txt"

//Error codes, returned by the functions and by main (exit status).
//  The values are part of the interface with Labview, so dont chnage them

#define OCR_OK            0     //no error                               
#define ERR_ARGS       -100     // wrong number of command line arguments
#define ERR_SEUIL      -105     // threshold not an integer in 0..100
#define ERR_OPEN_IN    -110     // Cell.bin cannot be opened              
#define ERR_HEADER     -115     // Cell.bin shorter than its header       
#define ERR_DIM_RANGE  -120     // width or height outside 10..100        
#define ERR_DIM_SMALL  -125     // cell smaller than the 32x32 digits     
#define ERR_MALLOC     -130     // memory allocation failed       
#define ERR_READ       -135     // I/O error while reading Cell.bin  
#define ERR_FEW_PIX    -140     // fewer than width x heght pixels 
#define ERR_MANY_PIX   -145     // more than width x height pixels
#define ERR_PIX_VAL    -150     // a pixel is neither 0 nor 1        
#define ERR_OPEN_OUT   -160     // CellValue.txt cannot be created      
#define ERR_WRITE      -165     // error while writing CellValue.txt   
#define ERR_PARAM      -170     // invalid argument given to a function


// Prototypes

// reading and validation of the inputs (was lecture_entres.c)
static int ReadSeuil(const char *str, int *seuil);
static int ValidateDim(uint32_t W, uint32_t H);
static int ValidatePixels(const unsigned char *cell, size_t n);
static int  ReadCell(const char *path, unsigned char **cell, uint32_t *W, uint32_t *H);
static const char *ErrorMsg(int err);

// saving of the result (was sauvegarde_resultat.c)
static int WriteCellValue(const char *cellPath, char digit, double percent);

// diagnostic of the header, compiled only with -DDEBUG_HEADER 
#ifdef DEBUG_HEADER
static void PrintHeaderDebug(uint32_t W, uint32_t H);
static uint32_t Swap32(uint32_t v);
#endif



// Reading and validation of the inputs (was lecture_entrees.c)


/**
   FileSize

   returns the size in bytes of an open file, witout moving its position

   Input = fp: open file
   Return = size of the file, -1 if fp is NULL or if fseek/ftell fail
     (errno is then set by the failing call)
*/
static long FileSize(FILE *fp)
{
    long pos, size;

    if (fp == NULL)                      return -1;

    pos = ftell(fp);
    if (pos < 0)                         return -1;
    if (fseek(fp, 0, SEEK_END) != 0)     return -1;
    size = ftell(fp);
    if (size < 0)                        return -1;
    if (fseek(fp, pos, SEEK_SET) != 0)   return -1;  //back where we were
    return size;
}

/**
   ReadU32

   reads one uint32_t (width or height) from the header of Cell.bin

   the value is copied as is, so the file must have the same endianness
   as the host (little endian on x86 and Apple silicon i think)
   a big edian file is still detected, its dimensions fall outside 10 to 100 though

   Input = fp: Cell.bin, opened in binary mode
   Output = v:  the value read, 0 on error
   Return = OCR_OK, ERR_HEADER (end of file), ERR_READ (I/O error), ERR_PARAM
*/
static int ReadU32(FILE *fp, uint32_t *v)
{
    if (fp == NULL || v == NULL) return ERR_PARAM;

    if (fread(v, sizeof(uint32_t), 1, fp) != 1) {
        *v = 0;
        return ferror(fp) ? ERR_READ : ERR_HEADER;
    }
    return OCR_OK;
}

/**
   ReadSeuil

   converts one threshold of the command line into an integer

   the whole string must be a base 10 integer in [SEUIL_MIN..SEUIL_MAX],
   ex. 90, "", abc, 9x, 50.5, -1, 101... or a number too large
   for a long are rejected. atoi is not used because it cannot report
   these errors (atoi("abc") just returns 0)

   Input = str:   the threshold as text, like argv[2]
   Output = seuil: the threshold, -1 (invalid threshold) on error
   Return = OCR_OK, ERR_SEUIL, ERR_PARAM
*/
static int ReadSeuil(const char *str, int *seuil)
{
    char *fin;
    long  v;

    if (seuil == NULL) return ERR_PARAM;
    *seuil = -1;
    if (str == NULL)   return ERR_PARAM;

    errno = 0;
    v = strtol(str, &fin, 10);
    // fin == str: no digit at all; *fin != '\0': trailing characters
    if (fin == str || *fin != '\0' || errno == ERANGE) return ERR_SEUIL;
    if (v < SEUIL_MIN || v > SEUIL_MAX)                return ERR_SEUIL;

    *seuil = (int)v;
    return OCR_OK;
}

/**
   ValidateDim

   checks the width and height read in the header of Cell.bin

   a negative size written as unsigned (ex. -100 -> 4294967196) or a file
   written in big endian (75 -> 1258291200) is out of range 
   the cell must also be at least as large as the 32x32 digit bitmaps, otherwise
   the digit cannot be moved over it

   Input = W, H : width and height in pixels
   Return = OCR_OK, ERR_DIM_RANGE (outside 10..100), ERR_DIM_SMALL (below 32)
*/
static int ValidateDim(uint32_t W, uint32_t H)
{
    if (W < DIM_MIN     || W > DIM_MAX)     return ERR_DIM_RANGE;
    if (H < DIM_MIN     || H > DIM_MAX)     return ERR_DIM_RANGE;
    if (W < DIGIT_SIZE  || H < DIGIT_SIZE)  return ERR_DIM_SMALL;
    return OCR_OK;
}

/**
   ValidatePixels

   checks that every pixel of the cell is 0 or 1

   Input = cell: the pixels and n: number of pixels in cell
   Return = OCR_OK, ERR_PIX_VAL, ERR_PARAM
*/
static int ValidatePixels(const unsigned char *cell, size_t n)
{
    size_t i;

    if (cell == NULL) return ERR_PARAM;

    for (i = 0; i < n; i++) {
        if (cell[i] > 1) return ERR_PIX_VAL;
    }
    return OCR_OK;
}

/**
   ReadCell

   reads Cell.bin: validates its header, then loads all its pixels into
   a dynamic array. The most important of all our functions probably

   Order of the checks:
     header present then we check dimensions in range then if file holds exactly
     8 + W x H bytes then again malloc then reads pixels in one fread and finally check
      every pixel is 0 or 1.
   The dimensions are validated before W x H is computed, so a corrupted
   header (65536 x 65536) cant overflow the product

   Errors are handled with a single "cleanup" label (goto):
   fp is closed on every path and the array is freed if anyting failed

   (failing file/memory calls are also reported with perror)

   Input = path: path of Cell.bin
   Outputs = cell: the W x H pixels, row after row, allocated with malloc();
           the caller must free() it; NULL on error;
     W, H: width and height in pixels, 0 on error
   Return =
     OCR_OK, ERR_OPEN_IN, ERR_HEADER, ERR_READ, ERR_DIM_RANGE,
     ERR_DIM_SMALL, ERR_FEW_PIX, ERR_MANY_PI, ERR_MALLOC, ERR_PIX_VAL,
     ERR_PARAM
*/
static int ReadCell(const char *path, unsigned char **cell,
                    uint32_t *W, uint32_t *H)
{
    FILE          *fp  = NULL;
    unsigned char *buf = NULL;
    long           taille, attendu;
    size_t         n;
    int            err = OCR_OK;

    if (path == NULL || cell == NULL || W == NULL || H == NULL)
        return ERR_PARAM;

    *cell = NULL;
    *W    = 0;
    *H    = 0;

    fp = fopen(path, "rb");
    if (fp == NULL) {
        perror(path);
        err = ERR_OPEN_IN;
        goto cleanup;
    }

    taille = FileSize(fp);
    if (taille < 0) {
        perror(path);
        err = ERR_READ;
        goto cleanup;
    }
    if (taille < HEADER_SIZE) { err = ERR_HEADER; goto cleanup; }

    err = ReadU32(fp, W);
    if (err == OCR_OK) err = ReadU32(fp, H);
    if (err != OCR_OK) {
        if (err == ERR_READ) perror(path);
        goto cleanup;
    }

#ifdef DEBUG_HEADER
    PrintHeaderDebug(*W, *H);
#endif

    err = ValidateDim(*W, *H);
    if (err != OCR_OK) goto cleanup;

    // again, W, H <= 100 here so the product cannot overflow a long
    attendu = HEADER_SIZE + (long)(*W) * (long)(*H);
    if (taille < attendu) { err = ERR_FEW_PIX;  goto cleanup; }
    if (taille > attendu) { err = ERR_MANY_PIX; goto cleanup; }

    n   = (size_t)(*W) * (size_t)(*H);
    buf = malloc(n);
    if (buf == NULL) {
        perror("malloc");
        err = ERR_MALLOC;
        goto cleanup;
    }

    if (fread(buf, sizeof(unsigned char), n, fp) != n) {
        if (ferror(fp)) {
            perror(path);
            err = ERR_READ;
        } else {
            err = ERR_FEW_PIX;      // if the file got shorter after FileSize
        }
        goto cleanup;
    }

    err = ValidatePixels(buf, n);

cleanup:
    if (fp != NULL && fclose(fp) != 0 && err == OCR_OK) {
        perror(path);
        err = ERR_READ;
    }
    if (err != OCR_OK) {
        free(buf);                  //free(NULL) does nothing
        *W = 0;
        *H = 0;
        return err;
    }
    *cell = buf;
    return OCR_OK;
}

/**
   ErrorMsg

   Returns the message (in french) of an error code

   Input = err: OCR_OK or one of the ERR codes
   Return = a constant string, "erreur inconnue" for an unknown code
*/
static const char *ErrorMsg(int err)
{
    switch (err) {
        case OCR_OK:        return "ok";
        case ERR_ARGS:      return "nombre de parametres incorrect (chemin + 3 seuils)";
        case ERR_SEUIL:     return "seuil invalide (entier 0..100 attendu)";
        case ERR_OPEN_IN:   return "Cell.bin introuvable ou illisible";
        case ERR_HEADER:    return "entete absente ou tronquee";
        case ERR_DIM_RANGE: return "dimensions hors bornes [10..100]";
        case ERR_DIM_SMALL: return "case trop petite pour un bitmap 32x32";
        case ERR_MALLOC:    return "echec d'allocation memoire";
        case ERR_READ:      return "erreur de lecture dans Cell.bin";
        case ERR_FEW_PIX:   return "pas assez de pixels dans le fichier";
        case ERR_MANY_PIX:  return "trop de pixels dans le fichier";
        case ERR_PIX_VAL:   return "pixel different de 0 ou 1";
        case ERR_OPEN_OUT:  return "CellValue.txt ne peut pas etre cree";
        case ERR_WRITE:     return "erreur d'ecriture dans CellValue.txt";
        case ERR_PARAM:     return "parametre de fonction invalide (erreur interne)";
        default:            return "erreur inconnue";
    }
}



 // Saving of the result (was sauvegarde_resultat.c)


/**
   BuildOutPath

   builds the path of CellValue.txt, in the same folder as Cell.bin

   i checked both '/' (macOS, Linux) and '\' (Windows) are accepted as separators
   Ex: "/Prj_Binairo/Cell.bin" -> "/Prj_Binairo/CellValue.txt"
       "Cell.bin" -> "CellValue.txt" (current folder)

   Input = cellPath: path of Cell.bin
   Return =
     the new path, allocated with malloc: the caller must free() it.
     NULL if cellPath is NULL or if malloc failed.
*/
static char *BuildOutPath(const char *cellPath)
{
    const char *sep, *bsep;
    size_t      dir;
    char       *out;

    if (cellPath == NULL) return NULL;

    sep  = strrchr(cellPath, '/');
    bsep = strrchr(cellPath, '\\');
    if (bsep != NULL && (sep == NULL || bsep > sep)) sep = bsep;
    dir  = (sep == NULL) ? 0 : (size_t)(sep - cellPath) + 1;  // keeps sep

    // sizeof of the string literal counts its terminating '\0'
    out = malloc(dir + sizeof(OUT_FILE));
    if (out == NULL) return NULL;

    memcpy(out, cellPath, dir);
    memcpy(out + dir, OUT_FILE, sizeof(OUT_FILE));
    return out;
}

/**
   WriteCellValue

   writes the recognised digit and its similarity in CellValue.txt

   The file is created (or overwritten) next to Cell.bin and contains one
   line, ex:  d:'0', 67%
   The arguments are checked first, so a wrong result of the
   recognition is reported as an errror instead of just being saved

   Inputs = cellPath: path of Cell.bin, gives the folder of CellValue.txt
     digit: '0', '1' or ' ' (empty cell)
     percent: similarity in [0..100]
   Return =
     OCR_OK, ERR_MALLOC, ERR_OPEN_OUT, ERR_WRITE, ERR_PARAM
*/
static int WriteCellValue(const char *cellPath, char digit, double percent)
{
    FILE *fp;
    char *outPath;
    int   err = OCR_OK;

    if (cellPath == NULL)                              return ERR_PARAM;
    if (digit != '0' && digit != '1' && digit != ' ') return ERR_PARAM;
    // written this way it also rejects nan (any comparison with nan is false)
    if (!(percent >= 0.0 && percent <= 100.0))         return ERR_PARAM;

    outPath = BuildOutPath(cellPath);
    if (outPath == NULL) {
        perror("malloc");
        return ERR_MALLOC;
    }

    fp = fopen(outPath, "w");
    if (fp == NULL) {
        perror(outPath);
        free(outPath);
        return ERR_OPEN_OUT;
    }

    if (fprintf(fp, "d:'%c', %.4f%%\n", digit, percent) < 0) {
        perror(outPath);
        err = ERR_WRITE;
    }
    // the data is only flushed to the disk by fclose, test it as well
    if (fclose(fp) != 0) {
        if (err == OCR_OK) perror(outPath);
        err = ERR_WRITE;
    }

    free(outPath);
    return err;
}



// Debug helpers, compiled only with -DDEBUG_HEADER


#ifdef DEBUG_HEADER
/**
   PrintHeaderDebug

   prints the width and height read in Cell.bin (decimal and hex) on
   stderr, and their byte-swapped values: if only the swapped values look
   right, Cell.bin was written in big endian

   Input =
     W, H: width and height as read in the file
*/
static void PrintHeaderDebug(uint32_t W, uint32_t H)
{
    fprintf(stderr, "[DEBUG] W = %10u (0x%08X)   H = %10u (0x%08X)\n",
            W, W, H, H);
    fprintf(stderr, "[DEBUG] byte-swapped: W = %u, H = %u\n",
            Swap32(W), Swap32(H));
}

/**
   Swap32

   Reverses the order of the 4 bytes of a uint32_t (big <-> little endian)
   So 0x4B000000 -> 0x0000004B

   Input = v: the value to swap
   Return = the swapped value
*/
static uint32_t Swap32(uint32_t v)
{
    return ((v & 0x000000FFu) << 24) | ((v & 0x0000FF00u) <<  8)
         | ((v & 0x00FF0000u) >>  8) | ((v & 0xFF000000u) >> 24);
}
#endif


// Main programme (was OCR_Teutch.c)


//input cell.bin

//size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
//first size is size of each element to be read in bytes, second is numher of elements

int main (int argc, const char *argv[]){
  if (argc != 5){
    fprintf(stderr, "Erreur %d: %s\n", ERR_ARGS, ErrorMsg(ERR_ARGS));
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
    fprintf(stderr, "Erreur %d: %s\n", err, ErrorMsg(err));
    return err;
  }

  printf("File: %s\n", path);
  printf("Thresholds: empty=%d, 0=%d, 1=%d\n", seuilV, seuil0, seuil1);

  unsigned char *Cell = NULL;
  uint32_t largeur = 0, hauteur = 0;

  err = ReadCell(path, &Cell, &largeur, &hauteur);
  if (err != OCR_OK){
    fprintf(stderr, "Erreur %d: %s\n", err, ErrorMsg(err));
    return err;
  }

  printf("Cell: %u x %u = %lu pixels lus\n",
         largeur, hauteur, (unsigned long)largeur * hauteur);

  char   chiffre     = ' ';
  double pourcentage = 0.0;

  free(Cell);

  err = WriteCellValue(path, chiffre, pourcentage);
  if (err != OCR_OK){
    fprintf(stderr, "Erreur %d: %s\n", err, ErrorMsg(err));
    return err;
  }

  printf("d:'%c', %.4f%%\n", chiffre, pourcentage);
  return OCR_OK;
}
