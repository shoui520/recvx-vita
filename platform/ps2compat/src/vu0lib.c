/*
 * libvu0 helpers (sceVu0ITOF12Vector lives in the game's ps2_NaDraw2D.c). Conventions follow Sony's libvu0.c: matrices are four row
 * vectors with the translation in row 3, MulMatrix(m0, m1, m2) computes
 * m0[i] = sum_k m1[k] * m2[i][k], and the products leave w = 0.
 */
#include <math.h>
#include <string.h>
#include "libvu0.h"
#include "ee_asm.h"

void sceVpu0Reset(void)
{
    memset(&recvx_vu0, 0, sizeof(recvx_vu0));
    VF(0)[3] = 1.0f;
}

void sceVu0UnitMatrix(sceVu0FMATRIX m)
{
    memset(m, 0, sizeof(sceVu0FMATRIX));
    m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
}

void sceVu0CopyMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1) { memmove(m0, m1, sizeof(sceVu0FMATRIX)); }

void sceVu0MulMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FMATRIX m2)
{
    sceVu0FMATRIX r;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            r[i][j] = m1[0][j] * m2[i][0] + m1[1][j] * m2[i][1] + m1[2][j] * m2[i][2] + m1[3][j] * m2[i][3];
    memcpy(m0, r, sizeof(r));
}

void sceVu0ApplyMatrix(sceVu0FVECTOR v0, sceVu0FMATRIX m, sceVu0FVECTOR v1)
{
    sceVu0FVECTOR r;
    for (int j = 0; j < 4; j++)
        r[j] = m[0][j] * v1[0] + m[1][j] * v1[1] + m[2][j] * v1[2] + m[3][j] * v1[3];
    memcpy(v0, r, sizeof(r));
}

void sceVu0TransposeMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1)
{
    sceVu0FMATRIX r;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) r[i][j] = m1[j][i];
    memcpy(m0, r, sizeof(r));
}

/* inverse of a rotation + translation matrix */
void sceVu0InversMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1)
{
    sceVu0FMATRIX r;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) r[i][j] = m1[j][i];
        r[i][3] = 0.0f;
    }
    for (int j = 0; j < 3; j++)
        r[3][j] = -(m1[3][0] * m1[j][0] + m1[3][1] * m1[j][1] + m1[3][2] * m1[j][2]);
    r[3][3] = 1.0f;
    memcpy(m0, r, sizeof(r));
}

static void rot(sceVu0FMATRIX m0, sceVu0FMATRIX m1, int a, int b, float c, float s)
{
    sceVu0FMATRIX t;
    sceVu0UnitMatrix(t);
    t[a][a] = c; t[a][b] = s;
    t[b][a] = -s; t[b][b] = c;
    sceVu0MulMatrix(m0, t, m1);
}

void sceVu0RotMatrixX(sceVu0FMATRIX m0, sceVu0FMATRIX m1, float rx) { rot(m0, m1, 1, 2, cosf(rx), sinf(rx)); }
void sceVu0RotMatrixY(sceVu0FMATRIX m0, sceVu0FMATRIX m1, float ry) { rot(m0, m1, 2, 0, cosf(ry), sinf(ry)); }
void sceVu0RotMatrixZ(sceVu0FMATRIX m0, sceVu0FMATRIX m1, float rz) { rot(m0, m1, 0, 1, cosf(rz), sinf(rz)); }

void sceVu0TransMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FVECTOR tv)
{
    memmove(m0, m1, sizeof(sceVu0FMATRIX));
    m0[3][0] += tv[0];
    m0[3][1] += tv[1];
    m0[3][2] += tv[2];
}

void sceVu0CopyVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1) { memmove(v0, v1, sizeof(sceVu0FVECTOR)); }

void sceVu0AddVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2)
{
    for (int i = 0; i < 4; i++) v0[i] = v1[i] + v2[i];
}

void sceVu0SubVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2)
{
    for (int i = 0; i < 4; i++) v0[i] = v1[i] - v2[i];
}

void sceVu0MulVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2)
{
    for (int i = 0; i < 4; i++) v0[i] = v1[i] * v2[i];
}

void sceVu0ScaleVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float s)
{
    for (int i = 0; i < 4; i++) v0[i] = v1[i] * s;
}

void sceVu0ScaleVectorXYZ(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float s)
{
    v0[0] = v1[0] * s; v0[1] = v1[1] * s; v0[2] = v1[2] * s; v0[3] = v1[3];
}

float sceVu0InnerProduct(sceVu0FVECTOR v0, sceVu0FVECTOR v1)
{
    return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
}

void sceVu0OuterProduct(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2)
{
    float x = v1[1] * v2[2] - v1[2] * v2[1];
    float y = v1[2] * v2[0] - v1[0] * v2[2];
    float z = v1[0] * v2[1] - v1[1] * v2[0];
    v0[0] = x; v0[1] = y; v0[2] = z; v0[3] = 0.0f;
}

void sceVu0Normalize(sceVu0FVECTOR v0, sceVu0FVECTOR v1)
{
    float l = sqrtf(v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]);
    float q = l != 0.0f ? 1.0f / l : 0.0f;   /* VU DIV by zero saturates; callers never hit it */
    v0[0] = v1[0] * q; v0[1] = v1[1] * q; v0[2] = v1[2] * q; v0[3] = 0.0f;
}

void sceVu0DivVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float q)
{
    float r = 1.0f / q;
    for (int i = 0; i < 4; i++) v0[i] = v1[i] * r;
}

static void itof(sceVu0FVECTOR v0, sceVu0IVECTOR v1, float scale)
{
    for (int i = 0; i < 4; i++) v0[i] = (float)v1[i] * scale;
}

/* VU FTOI truncates and saturates */
static int ftoi(float f)
{
    if (f >= 2147483647.0f) return 0x7fffffff;
    if (f <= -2147483648.0f) return (int)0x80000000;
    if (f != f) return 0;
    return (int)f;
}

void sceVu0ITOF0Vector(sceVu0FVECTOR v0, sceVu0IVECTOR v1) { itof(v0, v1, 1.0f); }
void sceVu0ITOF4Vector(sceVu0FVECTOR v0, sceVu0IVECTOR v1) { itof(v0, v1, 1.0f / 16.0f); }

void sceVu0FTOI0Vector(sceVu0IVECTOR v0, sceVu0FVECTOR v1)
{
    for (int i = 0; i < 4; i++) v0[i] = ftoi(v1[i]);
}

void sceVu0FTOI4Vector(sceVu0IVECTOR v0, sceVu0FVECTOR v1)
{
    for (int i = 0; i < 4; i++) v0[i] = ftoi(v1[i] * 16.0f);
}
