// Copyright (c) 2012 by Zuse-Institute Berlin and the Technical University of Denmark.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     1. Redistributions of source code must retain the above copyright
//        notice, this list of conditions and the following disclaimer.
//     2. Redistributions in binary form must reproduce the above copyright
//        notice, this list of conditions and the following disclaimer in the
//        documentation and/or other materials provided with the distribution.
//     3. Neither the name of the copyright holders nor contributors may not
//        be used to endorse or promote products derived from this software
//        without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS NOR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "frontend-cbf.h"
#include "cbf-format.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef CBF_FRONTEND_CBF_GZ_H
typedef FILE CBFFILE;

#else
#include <zlib.h>
typedef gzFile_s CBFFILE;
#define fopen(x,y) gzopen(x,y)
#define fclose(x) gzclose(x)
#define fgets(x,y,z) gzgets(z,x,y)
#endif

static CBFresponsee
  read(const char *file, CBFdata *data, CBFfrontendmemory *mem);

static void
  clean(CBFdata *data, CBFfrontendmemory *mem);

static CBFresponsee
  CBF_fgets(CBFFILE *pFile, long long int *linecount);

static CBFresponsee
  readVER(CBFFILE *pFile, long long int *linecount, CBFdata *data);

static CBFresponsee
  readOBJSENSE(CBFFILE *pFile, long long int *linecount, CBFdata *data);

static CBFresponsee
  readCON(CBFFILE *pFile, long long int *linecount, CBFdata *data);

static CBFresponsee
  readVAR(CBFFILE *pFile, long long int *linecount, CBFdata *data);

static CBFresponsee
  readINT(CBFFILE *pFile, long long int *linecount, CBFdata *data);

static CBFresponsee
  readPSDCON(CBFFILE *pFile, long long int *linecount, CBFdata *data);

static CBFresponsee
  readPSDVAR(CBFFILE *pFile, long long int *linecount, CBFdata *data);

static CBFresponsee
  readOBJFCOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data);

static CBFresponsee
  readOBJACOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data);

static CBFresponsee
  readOBJBCOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data);

static CBFresponsee
  readFCOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data);

static CBFresponsee
  readACOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data);

static CBFresponsee
  readBCOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data);

static CBFresponsee
  readHCOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data);

static CBFresponsee
  readDCOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data);

// -------------------------------------
// Global variable
// -------------------------------------

#ifdef CBF_FRONTEND_CBF_GZ_H
CBFfrontend const frontend_cbf_gz = { "cbf.gz", read, clean };
#else
CBFfrontend const frontend_cbf = { "cbf", read, clean };
#endif

// Automatically upgrade from frontend_cbf to frontend_cbf_gz when file ends with .gz
#ifndef CBF_FRONTEND_CBF_GZ_H
#ifdef ZLIB_SUPPORT
extern CBFfrontend const frontend_cbf_gz;
#endif
#endif

// -------------------------------------
// Function definitions
// -------------------------------------

static CBFresponsee read(const char *file, CBFdata *data, CBFfrontendmemory *mem) {
  CBFresponsee res = CBF_RES_OK;
  long long int linecount = 0;
  CBFFILE *pFile = NULL;

// Automatically upgrade from frontend_cbf to frontend_cbf_gz when file ends with .gz
#ifndef CBF_FRONTEND_CBF_GZ_H
#ifdef ZLIB_SUPPORT
  size_t flen = strlen(file);
  if ((flen >= 3) && (strcmp(file+flen-3, ".gz") == 0)) {
    return frontend_cbf_gz.read(file, data, mem);
  }
#endif
#endif

  pFile = fopen(file, "rt");
  if (!pFile) {
    return CBF_RES_ERR;
  }

  // Keyword OBJ should exist!
  data->objsense = CBF_OBJ_END;

  while( res==CBF_RES_OK && CBF_fgets(pFile, &linecount)==CBF_RES_OK )
  {
    // Parse keyword on non-empty lines
    if ( sscanf(CBF_LINE_BUFFER, CBF_NAME_FORMAT, CBF_NAME_BUFFER)==1 )
    {

      if (data->ver == 0) {

        if (strcmp(CBF_NAME_BUFFER, "VER") == 0)
          res = readVER(pFile, &linecount, data);

        else {
          printf("First keyword should be VER.\n");
          res = CBF_RES_ERR;
        }

      } else {

        if (strcmp(CBF_NAME_BUFFER, "OBJSENSE") == 0)
          res = readOBJSENSE(pFile, &linecount, data);

        else if (strcmp(CBF_NAME_BUFFER, "CON") == 0)
          res = readCON(pFile, &linecount, data);

        else if (strcmp(CBF_NAME_BUFFER, "VAR") == 0)
          res = readVAR(pFile, &linecount, data);

        else if (strcmp(CBF_NAME_BUFFER, "INT") == 0)
          res = readINT(pFile, &linecount, data);

        else if (strcmp(CBF_NAME_BUFFER, "PSDCON") == 0)
          res = readPSDCON(pFile, &linecount, data);

        else if (strcmp(CBF_NAME_BUFFER, "PSDVAR") == 0)
          res = readPSDVAR(pFile, &linecount, data);

        else if (strcmp(CBF_NAME_BUFFER, "OBJFCOORD") == 0)
          res = readOBJFCOORD(pFile, &linecount, data);

        else if (strcmp(CBF_NAME_BUFFER, "OBJACOORD") == 0)
          res = readOBJACOORD(pFile, &linecount, data);

        else if (strcmp(CBF_NAME_BUFFER, "OBJBCOORD") == 0)
          res = readOBJBCOORD(pFile, &linecount, data);

        else if (strcmp(CBF_NAME_BUFFER, "FCOORD") == 0)
          res = readFCOORD(pFile, &linecount, data);

        else if (strcmp(CBF_NAME_BUFFER, "ACOORD") == 0)
          res = readACOORD(pFile, &linecount, data);

        else if (strcmp(CBF_NAME_BUFFER, "BCOORD") == 0)
          res = readBCOORD(pFile, &linecount, data);

        else if (strcmp(CBF_NAME_BUFFER, "HCOORD") == 0)
          res = readHCOORD(pFile, &linecount, data);

        else if (strcmp(CBF_NAME_BUFFER, "DCOORD") == 0)
          res = readDCOORD(pFile, &linecount, data);

        else {
          printf("Keyword %s not recognized!\n", CBF_NAME_BUFFER);
          res = CBF_RES_ERR;
        }
      }

//      // Information blocks are terminated by an empty line
//      if ( res==CBF_RES_OK ) {
//        if ( CBF_fgets(pFile, &linecount)==CBF_RES_OK ) {
//          if ( sscanf(CBF_LINE_BUFFER, CBF_NAME_FORMAT, CBF_NAME_BUFFER)!=EOF ) {
//            printf("An empty line was expected, found: %s\n", CBF_NAME_BUFFER);
//            res = CBF_RES_ERR;
//          }
//        }
//      }
    }
  }

  if (res == CBF_RES_OK) {
    if (data->objsense == CBF_OBJ_END) {
      printf("Keyword OBJSENSE is missing.\n");
      res = CBF_RES_ERR;
    }
  }

  if (res != CBF_RES_OK) {
    printf("Failed to parse line: %lli\n", linecount);
    clean(data, mem);
  }

  fclose(pFile);
  return res;
}

static void clean(CBFdata *data, CBFfrontendmemory *mem) {
  if (data->mapstacknum >= 1) {
    free(data->mapstackdim);
    free(data->mapstackdomain);
  }

  if (data->varstacknum >= 1) {
    free(data->varstackdim);
    free(data->varstackdomain);
  }

  if (data->intvarnum >= 1) {
    free(data->intvar);
  }

  if (data->psdmapnum >= 1) {
    free(data->psdmapdim);
  }

  if (data->psdvarnum >= 1) {
    free(data->psdvardim);
  }

  if (data->objfnnz >= 1) {
    free(data->objfsubj);
    free(data->objfsubk);
    free(data->objfsubl);
    free(data->objfval);
  }

  if (data->objannz >= 1) {
    free(data->objasubj);
    free(data->objaval);
  }

  if (data->fnnz >= 1) {
    free(data->fsubi);
    free(data->fsubj);
    free(data->fsubk);
    free(data->fsubl);
    free(data->fval);
  }

  if (data->annz >= 1) {
    free(data->asubi);
    free(data->asubj);
    free(data->aval);
  }

  if (data->bnnz >= 1) {
    free(data->bsubi);
    free(data->bval);
  }

  if (data->hnnz >= 1) {
    free(data->hsubi);
    free(data->hsubj);
    free(data->hsubk);
    free(data->hsubl);
    free(data->hval);
  }

  if (data->dnnz >= 1) {
    free(data->dsubi);
    free(data->dsubk);
    free(data->dsubl);
    free(data->dval);
  }
}

static CBFresponsee CBF_fgets(CBFFILE *pFile, long long int *linecount)
{
  // Find first non-commentary line
  while( fgets(CBF_LINE_BUFFER, sizeof(CBF_LINE_BUFFER), pFile) != NULL ) {
    ++(*linecount);

    if (CBF_LINE_BUFFER[0] != '#')
      return CBF_RES_OK;
  }

  return CBF_RES_ERR;
}

static CBFresponsee readVER(CBFFILE *pFile, long long int *linecount, CBFdata *data)
{
  CBFresponsee res = CBF_RES_OK;

  res = CBF_fgets(pFile, linecount);

  if (res == CBF_RES_OK)
    if (sscanf(CBF_LINE_BUFFER, "%i", &data->ver) != 1)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK) {
    if (data->ver > CBF_VERSION) {
      printf("The version of the file format is not support.\n");
      res = CBF_RES_ERR;
    }
  }

  return res;
}

static CBFresponsee readOBJSENSE(CBFFILE *pFile, long long int *linecount, CBFdata *data)
{
  CBFresponsee res = CBF_RES_OK;

  res = CBF_fgets(pFile, linecount);

  if (res == CBF_RES_OK)
    if (sscanf(CBF_LINE_BUFFER, CBF_NAME_FORMAT, CBF_NAME_BUFFER) != 1)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK)
    res = CBF_strtoobjsense(CBF_NAME_BUFFER, &data->objsense);

  return res;
}

static CBFresponsee readCON(CBFFILE *pFile, long long int *linecount, CBFdata *data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i, mapnum = 0;

  res = CBF_fgets(pFile, linecount);

  if (res == CBF_RES_OK)
    if (sscanf(CBF_LINE_BUFFER, "%lli %lli", &data->mapnum, &data->mapstacknum) != 2)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK)
    if (data->mapstacknum < 0)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK) {
    data->mapstackdomain = (CBFscalarconee*) calloc(data->mapstacknum, sizeof(data->mapstackdomain[0]));
    data->mapstackdim = (long long int*) calloc(data->mapstacknum, sizeof(data->mapstackdim[0]));
  }

  for (i=0; i<(data->mapstacknum) && res==CBF_RES_OK; ++i) {
    res = CBF_fgets(pFile, linecount);

    if (res == CBF_RES_OK)
      if (sscanf(CBF_LINE_BUFFER, CBF_NAME_FORMAT" %lli", CBF_NAME_BUFFER, &data->mapstackdim[i]) != 2)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK) {
      mapnum += data->mapstackdim[i];
      res = CBF_strtocone(CBF_NAME_BUFFER, &data->mapstackdomain[i]);
    }

    if (res == CBF_RES_OK)
      if (data->mapstackdim[i] < 0)
        res = CBF_RES_ERR;
  }

  if (res == CBF_RES_OK)
    if (mapnum != data->mapnum)
      res = CBF_RES_ERR;

  return res;
}

static CBFresponsee readVAR(CBFFILE *pFile, long long int *linecount, CBFdata *data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i, varnum = 0;

  res = CBF_fgets(pFile, linecount);

  if (res == CBF_RES_OK)
    if (sscanf(CBF_LINE_BUFFER, "%lli %lli", &data->varnum, &data->varstacknum) != 2)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK)
    if (data->varstacknum < 0)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK) {
    data->varstackdomain = (CBFscalarconee*) calloc(data->varstacknum, sizeof(data->varstackdomain[0]));
    data->varstackdim = (long long int*) calloc(data->varstacknum, sizeof(data->varstackdim[0]));
  }

  for (i=0; i<(data->varstacknum) && res==CBF_RES_OK; ++i) {
    res = CBF_fgets(pFile, linecount);

    if (res == CBF_RES_OK)
      if (sscanf(CBF_LINE_BUFFER, CBF_NAME_FORMAT" %lli", CBF_NAME_BUFFER, &data->varstackdim[i]) != 2)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK) {
      varnum += data->varstackdim[i];
      res = CBF_strtocone(CBF_NAME_BUFFER, &data->varstackdomain[i]);
    }

    if (res == CBF_RES_OK)
      if (data->varstackdim[i] < 0)
        res = CBF_RES_ERR;
  }

  if (res == CBF_RES_OK)
    if (varnum != data->varnum)
      res = CBF_RES_ERR;

  return res;
}

static CBFresponsee readINT(CBFFILE *pFile, long long int *linecount, CBFdata *data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i;

  res = CBF_fgets(pFile, linecount);

  if (res == CBF_RES_OK)
    if (sscanf(CBF_LINE_BUFFER, "%lli", &data->intvarnum) != 1)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK)
    if (data->intvarnum < 0)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK) {
    data->intvar = (long long int*) calloc(data->intvarnum, sizeof(data->intvar[0]));

    if (!data->intvar)
      res = CBF_RES_ERR;
  }

  for (i=0; i<(data->intvarnum) && res==CBF_RES_OK; ++i) {
    res = CBF_fgets(pFile, linecount);

    if (res == CBF_RES_OK)
      if (sscanf(CBF_LINE_BUFFER, "%lli", &data->intvar[i]) != 1)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->intvar[i]) < 0 || (data->varnum-1) < (data->intvar[i]) )
        res = CBF_RES_ERR;
  }

  return res;
}

static CBFresponsee readPSDCON(CBFFILE *pFile, long long int *linecount, CBFdata *data)
{
  CBFresponsee res = CBF_RES_OK;
  int i;

  res = CBF_fgets(pFile, linecount);

  if (res == CBF_RES_OK)
    if (sscanf(CBF_LINE_BUFFER, "%i", &data->psdmapnum) != 1)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK)
    if (data->psdmapnum < 0)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK) {
    data->psdmapdim = (int*) calloc(data->psdmapnum, sizeof(data->psdmapdim[0]));
  }

  for (i=0; i<(data->psdmapnum) && res==CBF_RES_OK; ++i) {
    res = CBF_fgets(pFile, linecount);

    if (res == CBF_RES_OK)
      if (sscanf(CBF_LINE_BUFFER, "%i", &data->psdmapdim[i]) != 1)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if (data->psdmapdim[i] < 0)
        res = CBF_RES_ERR;
  }

  return res;
}

static CBFresponsee readPSDVAR(CBFFILE *pFile, long long int *linecount, CBFdata *data)
{
  CBFresponsee res = CBF_RES_OK;
  int i;

  res = CBF_fgets(pFile, linecount);

  if (res == CBF_RES_OK)
    if (sscanf(CBF_LINE_BUFFER, "%i", &data->psdvarnum) != 1)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK)
    if (data->psdvarnum < 0)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK) {
    data->psdvardim = (int*) calloc(data->psdvarnum, sizeof(data->psdvardim[0]));
  }

  for (i=0; i<(data->psdvarnum) && res==CBF_RES_OK; ++i) {
    res = CBF_fgets(pFile, linecount);

    if (res == CBF_RES_OK)
      if (sscanf(CBF_LINE_BUFFER, "%i", &data->psdvardim[i]) != 1)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if (data->psdvardim[i] < 0)
        res = CBF_RES_ERR;
  }

  return res;
}

static CBFresponsee readOBJFCOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i;

  res = CBF_fgets(pFile, linecount);

  if (res == CBF_RES_OK)
    if (sscanf(CBF_LINE_BUFFER, "%lli", &data->objfnnz) != 1)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK)
    if (data->objfnnz < 0)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK) {
    data->objfsubj = (int*) calloc(data->objfnnz, sizeof(data->objfsubj[0]));
    data->objfsubk = (int*) calloc(data->objfnnz, sizeof(data->objfsubk[0]));
    data->objfsubl = (int*) calloc(data->objfnnz, sizeof(data->objfsubl[0]));
    data->objfval  = (double*) calloc(data->objfnnz, sizeof(data->objfval[0]));
  }

  for (i=0; i<(data->objfnnz) && res==CBF_RES_OK; ++i) {
    res = CBF_fgets(pFile, linecount);

    if (res == CBF_RES_OK)
      if (sscanf(CBF_LINE_BUFFER, "%i %i %i %lg", &data->objfsubj[i], &data->objfsubk[i], &data->objfsubl[i], &data->objfval[i]) != 4)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->objfsubj[i]) < 0 || (data->psdvarnum-1) < (data->objfsubj[i]) )
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->objfsubk[i]) < 0 || (data->psdvardim[data->objfsubj[i]]-1) < (data->objfsubk[i]) )
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->objfsubl[i]) < 0 || (data->psdvardim[data->objfsubj[i]]-1) < (data->objfsubl[i]) )
        res = CBF_RES_ERR;
  }

  return res;
}

static CBFresponsee readOBJACOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i;

  res = CBF_fgets(pFile, linecount);

  if (res == CBF_RES_OK)
    if (sscanf(CBF_LINE_BUFFER, "%lli", &data->objannz) != 1)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK)
    if (data->objannz < 0)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK) {
    data->objasubj = (long long int*) calloc(data->objannz, sizeof(data->objasubj[0]));
    data->objaval  = (double*) calloc(data->objannz, sizeof(data->objaval[0]));
  }

  for (i=0; i<(data->objannz) && res==CBF_RES_OK; ++i) {
    res = CBF_fgets(pFile, linecount);

    if (res == CBF_RES_OK)
      if (sscanf(CBF_LINE_BUFFER, "%lli %lg", &data->objasubj[i], &data->objaval[i]) != 2)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->objasubj[i]) < 0 || (data->varnum-1) < (data->objasubj[i]) )
        res = CBF_RES_ERR;
  }

  return res;
}

static CBFresponsee readOBJBCOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data)
{
  CBFresponsee res = CBF_RES_OK;

  res = CBF_fgets(pFile, linecount);

  if (res == CBF_RES_OK)
    if (sscanf(CBF_LINE_BUFFER, "%lg", &data->objbval) != 1)
      res = CBF_RES_ERR;

  return res;
}

static CBFresponsee readFCOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i;

  res = CBF_fgets(pFile, linecount);

  if (res == CBF_RES_OK)
    if (sscanf(CBF_LINE_BUFFER, "%lli", &data->fnnz) != 1)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK)
    if (data->fnnz < 0)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK) {
    data->fsubi = (long long int*) calloc(data->fnnz, sizeof(data->fsubi[0]));
    data->fsubj = (int*) calloc(data->fnnz, sizeof(data->fsubj[0]));
    data->fsubk = (int*) calloc(data->fnnz, sizeof(data->fsubk[0]));
    data->fsubl = (int*) calloc(data->fnnz, sizeof(data->fsubl[0]));
    data->fval  = (double*) calloc(data->fnnz, sizeof(data->fval[0]));
  }

  for (i=0; i<(data->fnnz) && res==CBF_RES_OK; ++i) {
    res = CBF_fgets(pFile, linecount);

    if (res == CBF_RES_OK)
      if (sscanf(CBF_LINE_BUFFER, "%lli %i %i %i %lg", &data->fsubi[i], &data->fsubj[i], &data->fsubk[i], &data->fsubl[i], &data->fval[i]) != 5)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->fsubi[i]) < 0 || (data->mapnum-1) < (data->fsubi[i]) )
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->fsubj[i]) < 0 || (data->psdvarnum-1) < (data->fsubj[i]) )
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->fsubk[i]) < 0 || (data->psdvardim[data->fsubj[i]]-1) < (data->fsubk[i]) )
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->fsubl[i]) < 0 || (data->psdvardim[data->fsubj[i]]-1) < (data->fsubl[i]) )
        res = CBF_RES_ERR;
  }

  return res;
}

static CBFresponsee readACOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i;

  res = CBF_fgets(pFile, linecount);

  if (res == CBF_RES_OK)
    if (sscanf(CBF_LINE_BUFFER, "%lli", &data->annz) != 1)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK)
    if (data->annz < 0)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK) {
    data->asubi = (long long int*) calloc(data->annz, sizeof(data->asubi[0]));
    data->asubj = (long long int*) calloc(data->annz, sizeof(data->asubj[0]));
    data->aval  = (double*) calloc(data->annz, sizeof(data->aval[0]));
  }

  for (i=0; i<(data->annz) && res==CBF_RES_OK; ++i) {
    res = CBF_fgets(pFile, linecount);

    if (res == CBF_RES_OK)
      if (sscanf(CBF_LINE_BUFFER, "%lli %lli %lg", &data->asubi[i], &data->asubj[i], &data->aval[i]) != 3)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->asubi[i]) < 0 || (data->mapnum-1) < (data->asubi[i]) )
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->asubj[i]) < 0 || (data->varnum-1) < (data->asubj[i]) )
        res = CBF_RES_ERR;
  }

  return res;
}

static CBFresponsee readBCOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i;

  res = CBF_fgets(pFile, linecount);

  if (res == CBF_RES_OK)
    if (sscanf(CBF_LINE_BUFFER, "%lli", &data->bnnz) != 1)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK)
    if (data->bnnz < 0)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK) {
    data->bsubi = (long long int*) calloc(data->bnnz, sizeof(data->bsubi[0]));
    data->bval  = (double*) calloc(data->bnnz, sizeof(data->bval[0]));
  }

  for (i=0; i<(data->bnnz) && res==CBF_RES_OK; ++i) {
    res = CBF_fgets(pFile, linecount);

    if (res == CBF_RES_OK)
      if (sscanf(CBF_LINE_BUFFER, "%lli %lg", &data->bsubi[i], &data->bval[i]) != 2)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->bsubi[i]) < 0 || (data->mapnum-1) < (data->bsubi[i]) )
        res = CBF_RES_ERR;
  }

  return res;
}

static CBFresponsee readHCOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i;

  res = CBF_fgets(pFile, linecount);

  if (res == CBF_RES_OK)
    if (sscanf(CBF_LINE_BUFFER, "%lli", &data->hnnz) != 1)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK)
    if (data->hnnz < 0)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK) {
    data->hsubi = (int*) calloc(data->hnnz, sizeof(data->hsubi[0]));
    data->hsubj = (long long int*) calloc(data->hnnz, sizeof(data->hsubj[0]));
    data->hsubk = (int*) calloc(data->hnnz, sizeof(data->hsubk[0]));
    data->hsubl = (int*) calloc(data->hnnz, sizeof(data->hsubl[0]));
    data->hval  = (double*) calloc(data->hnnz, sizeof(data->hval[0]));
  }

  for (i=0; i<(data->hnnz) && res==CBF_RES_OK; ++i) {
    res = CBF_fgets(pFile, linecount);

    if (res == CBF_RES_OK)
      if (sscanf(CBF_LINE_BUFFER, "%i %lli %i %i %lg", &data->hsubi[i], &data->hsubj[i], &data->hsubk[i], &data->hsubl[i], &data->hval[i]) != 5)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->hsubi[i]) < 0 || (data->psdmapnum-1) < (data->hsubi[i]) )
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->hsubj[i]) < 0 || (data->varnum-1) < (data->hsubj[i]) )
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->hsubk[i]) < 0 || (data->psdmapdim[data->hsubi[i]]-1) < (data->hsubk[i]) )
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->hsubl[i]) < 0 || (data->psdmapdim[data->hsubi[i]]-1) < (data->hsubl[i]) )
        res = CBF_RES_ERR;
  }

  return res;
}

static CBFresponsee readDCOORD(CBFFILE *pFile, long long int *linecount, CBFdata *data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i;

  res = CBF_fgets(pFile, linecount);

  if (res == CBF_RES_OK)
    if (sscanf(CBF_LINE_BUFFER, "%lli", &data->dnnz) != 1)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK)
    if (data->dnnz < 0)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK) {
    data->dsubi = (int*) calloc(data->dnnz, sizeof(data->dsubi[0]));
    data->dsubk = (int*) calloc(data->dnnz, sizeof(data->dsubk[0]));
    data->dsubl = (int*) calloc(data->dnnz, sizeof(data->dsubl[0]));
    data->dval  = (double*) calloc(data->dnnz, sizeof(data->dval[0]));
  }

  for (i=0; i<(data->dnnz) && res==CBF_RES_OK; ++i) {
    res = CBF_fgets(pFile, linecount);

    if (res == CBF_RES_OK)
      if (sscanf(CBF_LINE_BUFFER, "%i %i %i %lg", &data->dsubi[i], &data->dsubk[i], &data->dsubl[i], &data->dval[i]) != 4)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->dsubi[i]) < 0 || (data->psdmapnum-1) < (data->dsubi[i]) )
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->dsubk[i]) < 0 || (data->psdmapdim[data->dsubi[i]]-1) < (data->dsubk[i]) )
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if ( (data->dsubl[i]) < 0 || (data->psdmapdim[data->dsubi[i]]-1) < (data->dsubl[i]) )
        res = CBF_RES_ERR;
  }

  return res;
}

