
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "validation_dimensions.h"


static long FileSize(FILE *fp)
{
    long pos, size;

    pos = ftell(fp);
    if (pos < 0)                         return -1;
    if (fseek(fp, 0, SEEK_END) != 0)     return -1;
    size = ftell(fp);
    if (size < 0)                        return -1;
    if (fseek(fp, pos, SEEK_SET) != 0)   return -1;
    return size;
}

static int ReadU32(FILE *fp, uint32_t *v)
{
    if (fread(v, sizeof(uint32_t), 1, fp) != 1) return ERR_HEADER;
    return OCR_OK;
}


int ValidateDim(uint32_t W, uint32_t H)
{
    if (W < DIM_MIN     || W > DIM_MAX)     return ERR_DIM_RANGE;
    if (H < DIM_MIN     || H > DIM_MAX)     return ERR_DIM_RANGE;
    if (W < DIGIT_SIZE  || H < DIGIT_SIZE)  return ERR_DIM_SMALL;
    return OCR_OK;
}

int ReadCell(const char *path, unsigned char **cell, uint32_t *W, uint32_t *H)
{
    FILE          *fp;
    long           taille, attendu;
    size_t         n;
    unsigned char *buf;
    int            err;

    *cell = NULL;
    *W    = 0;
    *H    = 0;

    fp = fopen(path, "rb");
    if (fp == NULL) return ERR_OPEN_IN;

    taille = FileSize(fp);
    if (taille < HEADER_SIZE) { fclose(fp); return ERR_HEADER; }

    err = ReadU32(fp, W);
    if (err == OCR_OK) err = ReadU32(fp, H);
    if (err != OCR_OK) { fclose(fp); return err; }

#ifdef DEBUG_HEADER
    PrintHeaderDebug(*W, *H);
#endif

    err = ValidateDim(*W, *H);
    if (err != OCR_OK) { fclose(fp); return err; }

    attendu = HEADER_SIZE + (long)(*W) * (long)(*H);
    if (taille < attendu) { fclose(fp); return ERR_FEW_PIX;  }
    if (taille > attendu) { fclose(fp); return ERR_MANY_PIX; }

    n   = (size_t)(*W) * (size_t)(*H);
    buf = malloc(n);
    if (buf == NULL) { fclose(fp); return ERR_MALLOC; }

    if (fread(buf, sizeof(unsigned char), n, fp) != n) {
        free(buf);
        fclose(fp);
        return ERR_FEW_PIX;
    }

    fclose(fp);
    *cell = buf;
    return OCR_OK;
}

const char *DimErrorMsg(int err)
{
    switch (err) {
        case OCR_OK:        return "ok";
        case ERR_OPEN_IN:   return "Cell.bin introuvable ou illisible";
        case ERR_HEADER:    return "entete absente ou tronquee";
        case ERR_DIM_RANGE: return "dimensions hors bornes [10..100]";
        case ERR_DIM_SMALL: return "case trop petite pour un bitmap 32x32";
        case ERR_MALLOC:    return "echec d'allocation memoire";
        case ERR_FEW_PIX:   return "pas assez de pixels dans le fichier";
        case ERR_MANY_PIX:  return "trop de pixels dans le fichier";
        default:            return "erreur inconnue";
    }
}

void PrintHeaderDebug(uint32_t W, uint32_t H)
{
    printf("[DEBUG] W = %10u (0x%08X)   H = %10u (0x%08X)\n",
           W, W, H, H);
}

uint32_t Swap32(uint32_t v)
{
    return ((v & 0x000000FFu) << 24) | ((v & 0x0000FF00u) <<  8)
         | ((v & 0x00FF0000u) >>  8) | ((v & 0xFF000000u) >> 24);
}