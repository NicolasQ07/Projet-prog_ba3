
#ifndef DIM_H
#define DIM_H

#include <stdint.h>

#define HEADER_SIZE      8L
#define DIM_MIN         10u
#define DIM_MAX        100u
#define DIGIT_SIZE      32u


#define OCR_OK            0
#define ERR_OPEN_IN    -110
#define ERR_HEADER     -115
#define ERR_DIM_RANGE  -120
#define ERR_DIM_SMALL  -125
#define ERR_MALLOC     -130
#define ERR_FEW_PIX    -140
#define ERR_MANY_PIX   -145


int ValidateDim(uint32_t W, uint32_t H);


int ReadCell(const char *path, unsigned char **cell,
             uint32_t *W, uint32_t *H);

const char *DimErrorMsg(int err);

void PrintHeaderDebug(uint32_t W, uint32_t H);

uint32_t Swap32(uint32_t v);

#endif