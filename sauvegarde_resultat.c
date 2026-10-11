#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lecture_entrees.h"
#include "sauvegarde_resultat.h"


static char *BuildOutPath(const char *cellPath)
{
    const char *sep, *bsep;
    size_t      dir;
    char       *out;

    sep  = strrchr(cellPath, '/');
    bsep = strrchr(cellPath, '\\');
    if (bsep != NULL && (sep == NULL || bsep > sep)) sep = bsep;
    dir  = (sep == NULL) ? 0 : (size_t)(sep - cellPath) + 1;

    out = malloc(dir + strlen(OUT_FILE) + 1);
    if (out == NULL) return NULL;

    memcpy(out, cellPath, dir);
    strcpy(out + dir, OUT_FILE);
    return out;
}

int WriteCellValue(const char *cellPath, char digit, double percent)
{
    FILE *fp;
    char *outPath;
    int   err;

    outPath = BuildOutPath(cellPath);
    if (outPath == NULL) return ERR_MALLOC;

    fp = fopen(outPath, "w");
    free(outPath);
    if (fp == NULL) return ERR_OPEN_OUT;

    err = OCR_OK;
    if (fprintf(fp, "d:'%c', %.4f%%\n", digit, percent) < 0) err = ERR_WRITE;
    if (fclose(fp) != 0)                                     err = ERR_WRITE;
    return err;
}