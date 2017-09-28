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

#include "backend-mps-mosek.h"
#include "backend-mps.h"
#include <stddef.h>
#include <stdio.h>

static CBFresponsee
  write(const char *file, const CBFdata data);

static CBFresponsee
  writeCSECTION(FILE *pFile, const CBFdata data);


// -------------------------------------
// Global variable
// -------------------------------------

CBFbackend const backend_mps_mosek = { "mps-mosek", "mps", write };


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
    res = MPS_writeROWS(pFile, data);

  if (res == CBF_RES_OK)
    res = MPS_writeCOLUMNS(pFile, data);

  if (res == CBF_RES_OK)
    res = MPS_writeRHS(pFile, data);

  if (res == CBF_RES_OK)
    res = MPS_writeBOUNDS(pFile, data);

  if (res == CBF_RES_OK)
    res = writeCSECTION(pFile, data);

  if (res == CBF_RES_OK)
    res = MPS_writeENDATA(pFile, data);

  fclose(pFile);
  return res;
}

static CBFresponsee writeCSECTION(FILE *pFile, const CBFdata data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i, j, curvar = 0, curmap = 0;
  const char *domain;

  for (i=0; i<data.varstacknum && res==CBF_RES_OK; ++i) {
    switch (data.varstackdomain[i]) {
    case CBF_CONE_QUAD:       domain = "QUAD";  break;
    case CBF_CONE_RQUAD:      domain = "RQUAD"; break;
    default:
      curvar += data.varstackdim[i];
      continue;
    }

    if (res == CBF_RES_OK)
      if (fprintf(pFile, "%-13s xK%-7lli %-14.16lg %s\n", "CSECTION", i, 0.0, domain) <= 0)
        res = CBF_RES_ERR;

    for (j=0; j<data.varstackdim[i] && res==CBF_RES_OK; ++j) {
      if (fprintf(pFile, "    x%lli\n", curvar) <= 0)
        res = CBF_RES_ERR;
      ++curvar;
    }
  }

  for (i=0; i<data.mapstacknum && res==CBF_RES_OK; ++i) {
    switch(data.mapstackdomain[i]) {
    case CBF_CONE_QUAD:       domain = "QUAD";  break;
    case CBF_CONE_RQUAD:      domain = "RQUAD"; break;
    default:
      curmap += data.mapstackdim[i];
      continue;
    }

    if (res == CBF_RES_OK)
      if (fprintf(pFile, "%-13s xgK%-6lli %-14.16lg %s\n", "CSECTION", i, 0.0, domain) <= 0)
        res = CBF_RES_ERR;

    for (j=0; j<data.mapstackdim[i] && res==CBF_RES_OK; ++j) {
      if (fprintf(pFile, "    xg%lli\n", curmap) <= 0)
        res = CBF_RES_ERR;
      ++curmap;
    }
  }

  return res;
}
