// Copyright (c) 2012 by Zuse-Institute Berlin and the Technical University of Denmark.
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "backend-sdpa.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static CBFresponsee
  write(const char *file, const CBFdata data);

static CBFresponsee
  writeVAR(FILE *pFile, const CBFdata data);

static CBFresponsee
  writeBLOCKS(FILE *pFile, const CBFdata data);

static CBFresponsee
  writeMAPZERO(FILE *pFile, const CBFdata data);

static CBFresponsee
  writePSDCON(FILE *pFile, const CBFdata data);

static CBFresponsee
  writeINTVAR(FILE *pFile, const CBFdata data);


// -------------------------------------
// Global variable
// -------------------------------------

CBFbackend const backend_sdpa = { "sdpa", "dat-s", write };


// -------------------------------------
// Function definitions
// -------------------------------------

static CBFresponsee write(const char *file, const CBFdata data) {
  CBFresponsee res = CBF_RES_OK;
  long long int i;
  FILE *pFile = NULL;

  if (data.mapnum >= 1) {
    printf("Scalar map constraints are not supported in the selected output file format.\n");
    return CBF_RES_ERR;
  }

  for (i=0; i<data.varstacknum; ++i) {
    if (data.varstackdomain[i] != CBF_CONE_FREE) {
      printf("Non-free scalar variables are not supported in the selected output file format.\n");
      return CBF_RES_ERR;
    }
  }

  if (data.objsense == CBF_OBJ_MAXIMIZE) {
    printf("Maximization problems are not supported in the selected output file format.\n");
    return CBF_RES_ERR;
  }

  if (data.objbval != 0.0) {
    printf("The non-zero constant in the objective function is not supported in the selected output file format.\n");
    return CBF_RES_ERR;
  }

  if (data.psdvarnum >= 1) {
    printf("Positive semidefinite variables are not supported in the selected output file format.\n");
    return CBF_RES_ERR;
  }

  pFile = fopen(file, "wt");
  if (!pFile) {
    return CBF_RES_ERR;
  }

  if (res == CBF_RES_OK)
    res = writeVAR(pFile, data);

  if (res == CBF_RES_OK)
    res = writeBLOCKS(pFile, data);

  if (res == CBF_RES_OK)
    res = writeMAPZERO(pFile, data);

  if (res == CBF_RES_OK)
    res = writePSDCON(pFile, data);

  if (res == CBF_RES_OK)
    res = writeINTVAR(pFile, data);

  fclose(pFile);
  return res;
}

static CBFresponsee writeVAR(FILE *pFile, const CBFdata data)
{
  CBFresponsee res = CBF_RES_OK;

  if (fprintf(pFile, "%lli\n", data.varnum) <= 0)
    res = CBF_RES_ERR;

  return res;
}

static CBFresponsee writeBLOCKS(FILE *pFile, const CBFdata data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i;

  if (fprintf(pFile, "%i\n", data.psdmapnum) <= 0)
    res = CBF_RES_ERR;

  for (i=0; i<data.psdmapnum && res==CBF_RES_OK; ++i)
    if (fprintf(pFile, "%i ", data.psdmapdim[i]) <= 0)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK)
    if (fprintf(pFile, "\n") <= 0)
      res = CBF_RES_ERR;

  return res;
}

static CBFresponsee writeMAPZERO(FILE *pFile, const CBFdata data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i;
  double *c = NULL;
  double sign = 1.0;

  if (data.varnum >= 1)
  {
    c = (double*) calloc(data.varnum, sizeof(c[0]));
    if (!c) {
      return CBF_RES_ERR;
    }

    for (i=0; i<data.objannz && res==CBF_RES_OK; ++i)
      c[data.objasubj[i]] = data.objaval[i];

    for (i=0; i<data.varnum && res==CBF_RES_OK; ++i)
      if (fprintf(pFile, "%.16lg ", sign*c[i]) <= 0)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if (fprintf(pFile, "\n") <= 0)
        res = CBF_RES_ERR;

    free(c);
  }

  return res;
}

static CBFresponsee writePSDCON(FILE *pFile, const CBFdata data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i;

  for (i=0; i<data.dnnz && res==CBF_RES_OK; ++i)
    if (fprintf(pFile, "%lli %i %i %i %.16lg\n", 0LL, data.dsubi[i]+1, data.dsubk[i]+1, data.dsubl[i]+1, -data.dval[i]) <= 0)
      res = CBF_RES_ERR;

  for (i=0; i<data.hnnz && res==CBF_RES_OK; ++i)
    if (fprintf(pFile, "%lli %i %i %i %.16lg\n", data.hsubj[i]+1, data.hsubi[i]+1, data.hsubk[i]+1, data.hsubl[i]+1, data.hval[i]) <= 0)
      res = CBF_RES_ERR;

  return res;
}

static CBFresponsee writeINTVAR(FILE *pFile, const CBFdata data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i;

  if (data.intvarnum >= 1)
  {
    if (res == CBF_RES_OK)
      if (fprintf(pFile, "*INTEGER*\n") <= 0)
        res = CBF_RES_ERR;

    for (i=0; i<data.intvarnum && res==CBF_RES_OK; ++i)
      if (fprintf(pFile, "*%lli\n", data.intvar[i]) <= 0)
        res = CBF_RES_ERR;
  }

  return res;
}

