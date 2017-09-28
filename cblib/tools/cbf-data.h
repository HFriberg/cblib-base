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

#ifndef CBF_CBF_DATA_H
#define CBF_CBF_DATA_H

#define CBF_VERSION     1
#define CBF_MAX_LINE  512       // Last 3 chars reserved for '\r\n\0'
#define CBF_MAX_NAME  512


typedef enum CBFobjsense_enum {
  CBF_OBJ_BEGIN = 0,
  CBF_OBJ_END = 2,

  CBF_OBJ_MINIMIZE = 0,
  CBF_OBJ_MAXIMIZE = 1
} CBFobjsensee;


typedef enum CBFscalarcone_enum {
  CBF_CONE_BEGIN = 0,
  CBF_CONE_END = 8,

  CBF_CONE_FREE = 0,
  CBF_CONE_POS = 1,
  CBF_CONE_NEG = 2,
  CBF_CONE_ZERO = 3,
  CBF_CONE_QUAD = 4,
  CBF_CONE_RQUAD = 5,
  CBF_CONE_PEXP = 6,
  CBF_CONE_DEXP = 7
} CBFscalarconee;


typedef struct CBFdata_struct {

  //
  // Problem format
  //
  int ver;

  //
  // Problem structure
  //
  CBFobjsensee    objsense;

  long long int   mapnum;
  long long int   mapstacknum;
  long long int  *mapstackdim;
  CBFscalarconee *mapstackdomain;

  long long int   varnum;
  long long int   varstacknum;
  long long int  *varstackdim;
  CBFscalarconee *varstackdomain;

  long long int   intvarnum;
  long long int  *intvar;

  int             psdmapnum;
  int            *psdmapdim;

  int             psdvarnum;
  int            *psdvardim;

  //
  // Coefficients of the objective scalar map
  //
  long long int  objfnnz;
  int           *objfsubj;
  int           *objfsubk;
  int           *objfsubl;
  double        *objfval;

  long long int  objannz;
  long long int *objasubj;
  double        *objaval;

  double         objbval;

  //
  // Coefficients of the scalar maps
  //
  long long int  fnnz;
  long long int *fsubi;
  int           *fsubj;
  int           *fsubk;
  int           *fsubl;
  double        *fval;

  long long int  annz;
  long long int *asubi;
  long long int *asubj;
  double        *aval;

  long long int  bnnz;
  long long int *bsubi;
  double        *bval;

  //
  // Coefficients of positive semidefinite maps
  //
  long long int  hnnz;
  int           *hsubi;
  long long int *hsubj;
  int           *hsubk;
  int           *hsubl;
  double        *hval;

  long long int  dnnz;
  int           *dsubi;
  int           *dsubk;
  int           *dsubl;
  double        *dval;

} CBFdata;

#endif

