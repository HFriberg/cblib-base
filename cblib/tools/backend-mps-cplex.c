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

#include "backend-mps-cplex.h"
#include "backend-mps.h"
#include <stddef.h>
#include <stdio.h>

static CBFresponsee
  write(const char *file, const CBFdata data);

static CBFresponsee
  writeROWS(FILE *pFile, const CBFdata data);

static CBFresponsee
  writeQCMATRIX(FILE *pFile, const CBFdata data);


// -------------------------------------
// Global variable
// -------------------------------------

CBFbackend const backend_mps_cplex = { "mps-cplex", "mps", write };


// -------------------------------------
// Function definitions
// -------------------------------------

static CBFresponsee write(const char *file, const CBFdata data) {
  CBFresponsee res = CBF_RES_OK;
  FILE *pFile = NULL;

  if (data.psdmapnum >= 1 || data.psdvarnum >= 1) {
    printf("Positive semidefinite domains are not supported in the selected output file format.\n");
    return CBF_RES_ERR;
  }

  pFile = fopen(file, "wt");
  if (!pFile) {
    return CBF_RES_ERR;
  }

  if (res == CBF_RES_OK)
    res = MPS_writeNAME(pFile, data);

  if (res == CBF_RES_OK)
    res = MPS_writeOBJSENSE(pFile, data);

  if (res == CBF_RES_OK)
    res = writeROWS(pFile, data);

  if (res == CBF_RES_OK)
    res = MPS_writeCOLUMNS(pFile, data);

  if (res == CBF_RES_OK)
    res = MPS_writeRHS(pFile, data);

  if (res == CBF_RES_OK)
    res = MPS_writeBOUNDS(pFile, data);

  if (res == CBF_RES_OK)
    res = writeQCMATRIX(pFile, data);

  if (res == CBF_RES_OK)
    res = MPS_writeENDATA(pFile, data);

  fclose(pFile);
  return res;
}

static CBFresponsee writeROWS(FILE *pFile, const CBFdata data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i;

  // Start with standard MPS
  res = MPS_writeROWS(pFile, data);

  // Append rows for the QCMATRIX definitions
  for (i=0; i<data.varstacknum && res==CBF_RES_OK; ++i) {
    switch (data.varstackdomain[i]) {
    case CBF_CONE_QUAD:
    case CBF_CONE_RQUAD:
      if (res == CBF_RES_OK)
        if (fprintf(pFile, " %s  xK%lli\n", "L", i) <= 0)
          res = CBF_RES_ERR;
      break;

    default:
      break;
    }
  }

  for (i=0; i<data.mapstacknum && res==CBF_RES_OK; ++i) {
    switch (data.mapstackdomain[i]) {
    case CBF_CONE_QUAD:
    case CBF_CONE_RQUAD:
      if (res == CBF_RES_OK)
        if (fprintf(pFile, " %s  xgK%lli\n", "L", i) <= 0)
          res = CBF_RES_ERR;
      break;

    default:
      break;
    }
  }

  return res;
}

static CBFresponsee writeQCMATRIX(FILE *pFile, const CBFdata data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i, j, curvar = 0, curmap = 0;

  for (i=0; i<data.varstacknum && res==CBF_RES_OK; ++i) {
    switch (data.varstackdomain[i]) {
    case CBF_CONE_QUAD:         break;
    case CBF_CONE_RQUAD:        break;
    default:
      curvar += data.varstackdim[i];
      continue;
    }

    if (res == CBF_RES_OK)
      if (fprintf(pFile, "%-10s xK%lli\n", "QCMATRIX", i) <= 0)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK) {
      if (data.varstackdomain[i] == CBF_CONE_QUAD) {
        if (fprintf(pFile, "    x%-8lli x%-8lli %.16lg\n    x%-8lli x%-8lli %.16lg\n", curvar, curvar, -1.0, curvar+1, curvar+1, 1.0) <= 0)
          res = CBF_RES_ERR;

      } else if (data.varstackdomain[i] == CBF_CONE_RQUAD) {
        if (fprintf(pFile, "    x%-8lli x%-8lli %.16lg\n    x%-8lli x%-8lli %.16lg\n", curvar, curvar+1, -1.0, curvar+1, curvar, -1.0) <= 0)
          res = CBF_RES_ERR;
      }
    }
    curvar += 2;

    for (j=2; j<data.varstackdim[i] && res==CBF_RES_OK; ++j) {
      if (fprintf(pFile, "    x%-8lli x%-8lli %.16lg\n", curvar, curvar, 1.0) <= 0)
        res = CBF_RES_ERR;
      ++curvar;
    }
  }

  for (i=0; i<data.mapstacknum && res==CBF_RES_OK; ++i) {
    switch (data.mapstackdomain[i]) {
    case CBF_CONE_QUAD:         break;
    case CBF_CONE_RQUAD:        break;
    default:
      curmap += data.mapstackdim[i];
      continue;
    }

    if (res == CBF_RES_OK)
      if (fprintf(pFile, "%-10s xgK%lli\n", "QCMATRIX", i) <= 0)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK) {
      if (data.mapstackdomain[i] == CBF_CONE_QUAD) {
        if (fprintf(pFile, "    xg%-7lli xg%-7lli %.16lg\n    xg%-7lli xg%-7lli %.16lg\n", curmap, curmap, -1.0, curmap+1, curmap+1, 1.0) <= 0)
          res = CBF_RES_ERR;

      } else if (data.mapstackdomain[i] == CBF_CONE_RQUAD) {
        if (fprintf(pFile, "    xg%-7lli xg%-7lli %.16lg\n    xg%-7lli xg%-7lli %.16lg\n", curmap, curmap+1, -1.0, curmap+1, curmap, -1.0) <= 0)
          res = CBF_RES_ERR;
      }
    }
    curmap += 2;

    for (j=2; j<data.mapstackdim[i] && res==CBF_RES_OK; ++j) {
      if (fprintf(pFile, "    xg%-7lli xg%-7lli %.16lg\n", curmap, curmap, 1.0) <= 0)
        res = CBF_RES_ERR;
      ++curmap;
    }
  }

  return res;
}
