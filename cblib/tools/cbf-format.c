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

#include "cbf-format.h"
#include <string.h>


// -------------------------------------
// Global variable
// -------------------------------------

// File format buffers
char CBF_LINE_BUFFER[CBF_MAX_LINE];
char CBF_NAME_BUFFER[CBF_MAX_NAME];

// Names of the scalar cones
const char * CBF_CONENAM_FREE = "F";
const char * CBF_CONENAM_ZERO = "L=";
const char * CBF_CONENAM_POS = "L+";
const char * CBF_CONENAM_NEG = "L-";
const char * CBF_CONENAM_QUAD = "Q";
const char * CBF_CONENAM_RQUAD = "QR";

// Names of the objective senses
const char * CBF_OBJSENSENAM_MIN = "MIN";
const char * CBF_OBJSENSENAM_MAX = "MAX";

// -------------------------------------
// Function definitions
// -------------------------------------

CBFresponsee CBF_conetostr(CBFscalarconee cone, const char **str)
{
  switch (cone) {
  case CBF_CONE_FREE:
    *str = CBF_CONENAM_FREE;
    break;
  case CBF_CONE_POS:
    *str = CBF_CONENAM_POS;
    break;
  case CBF_CONE_NEG:
    *str = CBF_CONENAM_NEG;
    break;
  case CBF_CONE_ZERO:
    *str = CBF_CONENAM_ZERO;
    break;
  case CBF_CONE_QUAD:
    *str = CBF_CONENAM_QUAD;
    break;
  case CBF_CONE_RQUAD:
    *str = CBF_CONENAM_RQUAD;
    break;
  default:
    return CBF_RES_ERR;
  }
  return CBF_RES_OK;
}

CBFresponsee CBF_strtocone(const char *str, CBFscalarconee *cone)
{
  if (strcmp(str, CBF_CONENAM_FREE) == 0)
    *cone = CBF_CONE_FREE;
  else if (strcmp(str, CBF_CONENAM_POS) == 0)
    *cone = CBF_CONE_POS;
  else if (strcmp(str, CBF_CONENAM_NEG) == 0)
    *cone = CBF_CONE_NEG;
  else if (strcmp(str, CBF_CONENAM_ZERO) == 0)
    *cone = CBF_CONE_ZERO;
  else if (strcmp(str, CBF_CONENAM_QUAD) == 0)
    *cone = CBF_CONE_QUAD;
  else if (strcmp(str, CBF_CONENAM_RQUAD) == 0)
    *cone = CBF_CONE_RQUAD;
  else
    return CBF_RES_ERR;

  return CBF_RES_OK;
}

CBFresponsee CBF_objsensetostr(CBFobjsensee objsense, const char **str)
{
  switch (objsense) {
  case CBF_OBJ_MINIMIZE:
    *str = CBF_OBJSENSENAM_MIN;
    break;
  case CBF_OBJ_MAXIMIZE:
    *str = CBF_OBJSENSENAM_MAX;
    break;
  default:
    return CBF_RES_ERR;
  }
  return CBF_RES_OK;
}

CBFresponsee CBF_strtoobjsense(const char *str, CBFobjsensee *objsense)
{
  if (strcmp(str, CBF_OBJSENSENAM_MIN) == 0)
    *objsense = CBF_OBJ_MINIMIZE;
  else if (strcmp(str, CBF_OBJSENSENAM_MAX) == 0)
    *objsense = CBF_OBJ_MAXIMIZE;
  else
    return CBF_RES_ERR;

  return CBF_RES_OK;
}
