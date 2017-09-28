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

#include "backend-mps.h"
#include "cbf-helper.h"
#include <stddef.h>
#include <stdlib.h>

static CBFresponsee
  MPS_writeCOLUMNS_controlINTEGERMARK(FILE *pFile, const CBFdata data, long long int xID, const long long int *intidx, long long int *curint, long long int *curintmark, int *isintegermark);


CBFresponsee MPS_writeNAME(FILE *pFile, const CBFdata data)
{
  CBFresponsee res = CBF_RES_OK;

  if (fprintf(pFile, "%-14s%s\n", "NAME", "UNKNOWN") <= 0)
    res = CBF_RES_ERR;

  return res;
}

CBFresponsee MPS_writeOBJSENSE(FILE *pFile, const CBFdata data)
{
  CBFresponsee res = CBF_RES_OK;
  const char *sense;

  if (data.objsense == CBF_OBJ_MINIMIZE)
    sense = "MIN";
  else if (data.objsense == CBF_OBJ_MAXIMIZE)
    sense = "MAX";
  else
    res = CBF_RES_ERR;

  if (res == CBF_RES_OK)
    if (fprintf(pFile, "OBJSENSE\n    %s\n", sense) <= 0)
      res = CBF_RES_ERR;

  return res;
}

CBFresponsee MPS_writeROWS(FILE *pFile, const CBFdata data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i, j, curmap = 0;
  const char *domain;

  if (res == CBF_RES_OK)
    if (fprintf(pFile, "ROWS\n") <= 0)
      res = CBF_RES_ERR;

  if (res == CBF_RES_OK) {
    if (fprintf(pFile, " %s  %s\n", "N", "obj") <= 0)
      res = CBF_RES_ERR;
  }

  for (i=0; i<data.mapstacknum && res==CBF_RES_OK; ++i) {
    switch(data.mapstackdomain[i])
    {
    case CBF_CONE_FREE:         domain = "N";   break;
    case CBF_CONE_POS:          domain = "G";   break;
    case CBF_CONE_NEG:          domain = "L";   break;
    case CBF_CONE_ZERO:         domain = "E";   break;
    case CBF_CONE_QUAD:         domain = "E";   break;
    case CBF_CONE_RQUAD:        domain = "E";   break;
    default:
      res = CBF_RES_ERR;
      break;
    }

    for (j=0; j<data.mapstackdim[i] && res==CBF_RES_OK; ++j) {
      if (fprintf(pFile, " %s  g%lli\n", domain, curmap) <= 0)
        res = CBF_RES_ERR;
      ++curmap;
    }
  }
  return res;
}

CBFresponsee MPS_writeCOLUMNS(FILE *pFile, const CBFdata data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i, j, curmap = 0;
  long long int *aidx = NULL, *objaidx = NULL, *intidx = NULL;
  int isintegermark = 0;
  long long int lastj = -1, curobja = 0, curint = 0, curintmark = 0;

  aidx = (long long int *) malloc(data.annz * sizeof(aidx[0]));
  objaidx = (long long int *) malloc(data.objannz * sizeof(objaidx[0]));
  intidx  = (long long int *) malloc(data.intvarnum * sizeof(intidx[0]));

  if (!aidx || !objaidx || !intidx) {
    if (aidx)           free(aidx);
    if (objaidx)        free(objaidx);
    if (intidx)         free(intidx);
    return CBF_RES_ERR;
  }

  //
  // Sort a-coefficients, obja-coefficients, and integer variable indexes
  //
  if (data.annz >= 1)
  {
    if (res == CBF_RES_OK)
      for (i=0; i<data.annz; ++i)
        aidx[i] = i;

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(data.mapnum-1, data.annz, data.asubi, aidx);   // secondarily by asubi

    if ( res==CBF_RES_OK )
      res = CBF_bucketsort(data.varnum-1, data.annz, data.asubj, aidx);   // primarily by asubj
  }

  if (data.objannz >= 1)
  {
    if (res == CBF_RES_OK)
      for (i=0; i<data.objannz; ++i)
        objaidx[i] = i;

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(data.varnum-1, data.objannz, data.objasubj, objaidx);
  }

  if (data.intvarnum >= 1)
  {
    if (res == CBF_RES_OK)
      for (i=0; i<data.intvarnum; ++i)
        intidx[i] = i;

    if ( res==CBF_RES_OK )
      res = CBF_bucketsort(data.varnum-1, data.intvarnum, data.intvar, intidx);
  }

  //
  // Write data
  //
  if (res == CBF_RES_OK)
    if (fprintf(pFile, "COLUMNS\n") <= 0)
      res = CBF_RES_ERR;

  for (i=0; i<data.annz && res==CBF_RES_OK; ++i) {

    // Insert objective coefficient of variables
    // that do not appear in any column.
    for (j=lastj+1; j<data.asubj[aidx[i]] && res==CBF_RES_OK; ++j) {
      res = MPS_writeCOLUMNS_controlINTEGERMARK(pFile, data, j, intidx, &curint, &curintmark, &isintegermark);

      if ( res==CBF_RES_OK ) {
        if ( curobja < data.objannz && data.objasubj[objaidx[curobja]] == j ) {
          if (fprintf(pFile, "    x%-8lli %-9s %.16lg\n", j, "obj", data.objaval[objaidx[curobja]]) <= 0) {
            res = CBF_RES_ERR;
          }
          ++curobja;
        
        } else {
          if (fprintf(pFile, "    x%-8lli %-9s %.16lg\n", j, "obj", 0.0) <= 0)
            res = CBF_RES_ERR;
        }
      }
    }
    
    // Insert coefficients of current variable
    if ( res==CBF_RES_OK )
      res = MPS_writeCOLUMNS_controlINTEGERMARK(pFile, data, data.asubj[aidx[i]], intidx, &curint, &curintmark, &isintegermark);
    
    if ( res==CBF_RES_OK ) {
      if ( lastj != data.asubj[aidx[i]] ) {
        if ( curobja < data.objannz && data.objasubj[objaidx[curobja]] == j ) {
          if (fprintf(pFile, "    x%-8lli %-9s %.16lg\n", j, "obj", data.objaval[objaidx[curobja]]) <= 0) {
            res = CBF_RES_ERR;
          }
          ++curobja;
        }
      }
    }
    
    if ( res==CBF_RES_OK )
      if (fprintf(pFile, "    x%-8lli g%-8lli %.16lg\n", data.asubj[aidx[i]], data.asubi[aidx[i]], data.aval[aidx[i]]) <= 0)
        res = CBF_RES_ERR;
    
    lastj = data.asubj[aidx[i]];
  }

  // Insert objective coefficient of variables
  // that do not appear in any column.
  for (j=lastj+1; j<data.varnum && res==CBF_RES_OK; ++j) {
    res = MPS_writeCOLUMNS_controlINTEGERMARK(pFile, data, j, intidx, &curint, &curintmark, &isintegermark);

    if ( res==CBF_RES_OK )
    {
      if ( curobja < data.objannz && data.objasubj[objaidx[curobja]] == j )
      {
        if (fprintf(pFile, "    x%-8lli %-9s %.16lg\n", j, "obj", data.objaval[objaidx[curobja]]) <= 0)
          res = CBF_RES_ERR;
        ++curobja;
      }
      else
      {
        if (fprintf(pFile, "    x%-8lli %-9s %.16lg\n", j, "obj", 0.0) <= 0)
          res = CBF_RES_ERR;
      }
    }
  }

  // Close integer mark if still open
  if ( res==CBF_RES_OK ) {
    if (isintegermark) {
      if (fprintf(pFile, "    MARK%04lli  %-24s %s\n", curintmark, "'MARKER'", "'INTEND'") <= 0)
        res = CBF_RES_ERR;

      isintegermark = 0;
      curintmark = (curintmark+1) % 10000;
    }
  }

  // Print slack variables used to put affine maps in cones
  for (i=0; i<data.mapstacknum && res==CBF_RES_OK; ++i) {
    switch(data.mapstackdomain[i])
    {
    case CBF_CONE_QUAD:
    case CBF_CONE_RQUAD:
      for (j=0; j<data.mapstackdim[i] && res==CBF_RES_OK; ++j) {
        if (fprintf(pFile, "    xg%-7lli g%-8lli %.16lg\n", curmap, curmap, -1.0) <= 0)
          res = CBF_RES_ERR;
        ++curmap;
      }
      break;
    default:
      curmap += data.mapstackdim[i];
      break;
    }
  }

  free(aidx);
  free(intidx);

  return res;
}

static CBFresponsee MPS_writeCOLUMNS_controlINTEGERMARK(FILE *pFile, const CBFdata data, long long int xID,
                                                        const long long int *intidx,
                                                        long long int       *curint,
                                                        long long int       *curintmark,
                                                        int                 *isintegermark)
{
  CBFresponsee res = CBF_RES_OK;

  // Handle integer mark
  if (data.intvarnum >= 1)
  {
    while (*curint < data.intvarnum-1 && data.intvar[intidx[*curint]] < xID)
      ++(*curint);

    if ( res==CBF_RES_OK ) {
      if (!*isintegermark && data.intvar[intidx[*curint]] == xID) {
        if (fprintf(pFile, "    MARK%04lli  %-24s %s\n", *curintmark, "'MARKER'", "'INTORG'") <= 0)
          res = CBF_RES_ERR;

        *isintegermark = 1;
        *curintmark = (*curintmark+1) % 10000;
      }
    }

    if ( res==CBF_RES_OK ) {
      if (*isintegermark && data.intvar[intidx[*curint]] != xID) {
        if (fprintf(pFile, "    MARK%04lli  %-24s %s\n", *curintmark, "'MARKER'", "'INTEND'") <= 0)
          res = CBF_RES_ERR;

        *isintegermark = 0;
        *curintmark = (*curintmark+1) % 10000;
      }
    }
  }

  return res;
}

CBFresponsee MPS_writeRHS(FILE *pFile, const CBFdata data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i;

  if ( data.objbval != 0.0 || data.bnnz >= 1 )
  {
    if (res == CBF_RES_OK)
      if (fprintf(pFile, "RHS\n") <= 0)
        res = CBF_RES_ERR;

    if (res == CBF_RES_OK)
      if (data.objbval != 0.0)
        if (fprintf(pFile, "    %-9s %-9s %.16lg\n", "BVEC", "obj", -data.objbval) <= 0)
          res = CBF_RES_ERR;

    for (i=0; i<data.bnnz && res==CBF_RES_OK; ++i)
      if (fprintf(pFile, "    %-9s g%-8lli %.16lg\n", "BVEC", data.bsubi[i], -data.bval[i]) <= 0)
        res = CBF_RES_ERR;
  }

  return res;
}

CBFresponsee MPS_writeBOUNDS(FILE *pFile, const CBFdata data)
{
  CBFresponsee res = CBF_RES_OK;
  long long int i, j, curvar = 0, curmap = 0, stackidx = 0;
  const char *domain1, *domain2;

  if (res == CBF_RES_OK)
    if (fprintf(pFile, "BOUNDS\n") <= 0)
      res = CBF_RES_ERR;

  // Bounds of variables
  for (i=0; i<data.varstacknum && res==CBF_RES_OK; ++i) {
    switch (data.varstackdomain[i]) {
    case CBF_CONE_FREE:
      stackidx = 0;     domain1 = "FR"; domain2 = NULL;  break;
    case CBF_CONE_POS:
      stackidx = 0;     domain1 = "PL"; domain2 = NULL;  break;         // Lower bound of 0 is assumed default
    case CBF_CONE_NEG:
      stackidx = 0;     domain1 = "MI"; domain2 = "UP";  break;
    case CBF_CONE_ZERO:
      stackidx = 0;     domain1 = NULL; domain2 = "FX";  break;
    case CBF_CONE_QUAD:
      if (fprintf(pFile, " %s %-9s x%-8lli\n", "PL", "DOMAIN", curvar) <= 0)
        res = CBF_RES_ERR;
      stackidx = 1;     domain1 = "FR"; domain2 = NULL;  break;
    case CBF_CONE_RQUAD:
      if (fprintf(pFile, " %s %-9s x%-8lli\n"
                         " %s %-9s x%-8lli\n", "PL", "DOMAIN", curvar, "PL", "DOMAIN", curvar+1) <= 0)
        res = CBF_RES_ERR;
      stackidx = 2;     domain1 = "FR"; domain2 = NULL;  break;
    default:
      res = CBF_RES_ERR;
      break;
    }

    curvar += stackidx;
    for (j=stackidx; j<data.varstackdim[i] && res==CBF_RES_OK; ++j) {
      if (domain1 != NULL)
        if (fprintf(pFile, " %s %-9s x%-8lli\n", domain1, "DOMAIN", curvar) <= 0)
          res = CBF_RES_ERR;
      if (domain2 != NULL)
        if (fprintf(pFile, " %s %-9s x%-8lli %.16lg\n", domain2, "DOMAIN", curvar, 0.0) <= 0)
          res = CBF_RES_ERR;
      ++curvar;
    }
  }

  // Bounds of variables added as slack to model the conic domain of a map
  for (i=0; i<data.mapstacknum && res==CBF_RES_OK; ++i) {
    switch(data.mapstackdomain[i])
    {
    case CBF_CONE_QUAD:
      if (fprintf(pFile, " %s %-9s xg%-7lli\n", "PL", "DOMAIN", curmap) <= 0)
        res = CBF_RES_ERR;
      stackidx = 1;     domain1 = "FR";  break;
      break;
    case CBF_CONE_RQUAD:
      if (fprintf(pFile, " %s %-9s xg%-7lli\n"
                         " %s %-9s xg%-7lli\n", "PL", "DOMAIN", curmap, "PL", "DOMAIN", curmap+1) <= 0)
        res = CBF_RES_ERR;
      stackidx = 2;     domain1 = "FR";  break;
      break;
    default:
      curmap += data.mapstackdim[i];
      continue;
    }

    curmap += stackidx;
    for (j=stackidx; j<data.mapstackdim[i] && res==CBF_RES_OK; ++j) {
      if (fprintf(pFile, " %s %-9s xg%-7lli\n", domain1, "DOMAIN", curmap) <= 0)
        res = CBF_RES_ERR;
      ++curmap;
    }
  }

  return res;
}

CBFresponsee MPS_writeENDATA(FILE *pFile, const CBFdata data)
{
  CBFresponsee res = CBF_RES_OK;

  if (fprintf(pFile, "ENDATA\n") <= 0)
    res = CBF_RES_ERR;

  return res;
}
