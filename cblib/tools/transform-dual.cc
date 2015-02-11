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

#include "transform-dual.h"
#include "cbf-format.h"

#include <algorithm>

struct CBFtransform_flipsign {
  bool obja;
  bool objf;
  bool a;
  bool f;
  bool h;
  bool b;
  bool d;
};

static CBFresponsee
  transform(CBFdata *data, CBFtransform_param param);

static CBFresponsee
  swap_obja_b(CBFdata *data, CBFtransform_flipsign *flipsign);

static CBFresponsee
  swap_objf_d(CBFdata *data, CBFtransform_flipsign *flipsign);

static CBFresponsee
  swap_f_h(CBFdata *data, CBFtransform_flipsign *flipsign);

static CBFresponsee
  transpose_a(CBFdata *data, CBFtransform_flipsign *flipsign);

static CBFresponsee
  remove_intvar(CBFdata *data, CBFtransform_flipsign *flipsign);

static CBFresponsee
  swap_map_var(CBFdata *data, CBFtransform_flipsign *flipsign);

static CBFresponsee
  swap_psdmap_psdvar(CBFdata *data, CBFtransform_flipsign *flipsign);

static CBFresponsee
  flip_objsense(CBFdata *data, CBFtransform_flipsign *flipsign);

static CBFresponsee
  flip_signs(CBFdata *data, CBFtransform_flipsign *flipsign);

// -------------------------------------
// Global variable
// -------------------------------------

CBFtransform const transform_dual = { "dual", transform, };


// -------------------------------------
// Function definitions
// -------------------------------------

static CBFresponsee transform(CBFdata *data, CBFtransform_param param)
{
  CBFresponsee res = CBF_RES_OK;
  CBFtransform_flipsign flipsign = {false};

  if ( res == CBF_RES_OK )
    res = flip_objsense(data, &flipsign);

  if ( res == CBF_RES_OK )
    res = swap_obja_b(data, &flipsign);

  if ( res == CBF_RES_OK )
    res = swap_objf_d(data, &flipsign);

  if ( res == CBF_RES_OK )
    res = swap_f_h(data, &flipsign);

  if ( res == CBF_RES_OK )
    res = transpose_a(data, &flipsign);

  if ( res == CBF_RES_OK )
    res = remove_intvar(data, &flipsign);

  if ( res == CBF_RES_OK )
    res = swap_map_var(data, &flipsign);

  if ( res == CBF_RES_OK )
    res = swap_psdmap_psdvar(data, &flipsign);

  if ( res == CBF_RES_OK )
    res = flip_signs(data, &flipsign);

  return res;
}


static CBFresponsee swap_obja_b(CBFdata *data, CBFtransform_flipsign *flipsign)
{
  std::swap(data->objannz,  data->bnnz);
  std::swap(data->objasubj, data->bsubi);
  std::swap(data->objaval,  data->bval);
  std::swap(flipsign->obja, flipsign->b);

  // dualization flips signs
  flipsign->obja = !flipsign->obja;
  flipsign->b    = !flipsign->b;

  return CBF_RES_OK;
}

static CBFresponsee swap_objf_d(CBFdata *data, CBFtransform_flipsign *flipsign)
{
  std::swap(data->objfnnz,  data->dnnz);
  std::swap(data->objfsubj, data->dsubi);
  std::swap(data->objfsubk, data->dsubk);
  std::swap(data->objfsubl, data->dsubl);
  std::swap(data->objfval,  data->dval);
  std::swap(flipsign->objf, flipsign->d);

  // dualization flips signs
  flipsign->objf = !flipsign->objf;
  flipsign->d    = !flipsign->d;

  return CBF_RES_OK;
}

static CBFresponsee swap_f_h(CBFdata *data, CBFtransform_flipsign *flipsign)
{
  std::swap(data->fnnz,  data->hnnz);
  std::swap(data->fsubi, data->hsubj);
  std::swap(data->fsubj, data->hsubi);
  std::swap(data->fsubk, data->hsubk);
  std::swap(data->fsubl, data->hsubl);
  std::swap(data->fval,  data->hval);
  std::swap(flipsign->f, flipsign->h);

  return CBF_RES_OK;
}

static CBFresponsee transpose_a(CBFdata *data, CBFtransform_flipsign *flipsign)
{
  std::swap(data->asubi, data->asubj);

  return CBF_RES_OK;
}

static CBFresponsee remove_intvar(CBFdata *data, CBFtransform_flipsign *flipsign)
{
  // Dual of continuous relaxation
  if (data->intvarnum >= 1) {
    free(data->intvar);
    data->intvarnum = 0;
  }

  return CBF_RES_OK;
}

static CBFresponsee swap_map_var(CBFdata *data, CBFtransform_flipsign *flipsign)
{
  long long int i;

  std::swap(data->mapnum,         data->varnum);
  std::swap(data->mapstacknum,    data->varstacknum);
  std::swap(data->mapstackdim,    data->varstackdim);
  std::swap(data->mapstackdomain, data->varstackdomain);

  // Dualize map domains
  for (i = 0; i < data->mapstacknum; ++i) {
    if (data->mapstackdomain[i] == CBF_CONE_FREE)
      data->mapstackdomain[i] = CBF_CONE_ZERO;

    else if (data->mapstackdomain[i] == CBF_CONE_ZERO)
      data->mapstackdomain[i] = CBF_CONE_FREE;
  }

  // Dualize var domains
  for (i = 0; i < data->varstacknum; ++i) {
    if (data->varstackdomain[i] == CBF_CONE_FREE)
      data->varstackdomain[i] = CBF_CONE_ZERO;

    else if (data->varstackdomain[i] == CBF_CONE_ZERO)
      data->varstackdomain[i] = CBF_CONE_FREE;
  }

  // map's can not belong to negative domains
  flipsign->a = !flipsign->a;
  flipsign->f = !flipsign->f;
  flipsign->b = !flipsign->b;

  return CBF_RES_OK;
}

static CBFresponsee swap_psdmap_psdvar(CBFdata *data, CBFtransform_flipsign *flipsign)
{
  std::swap(data->psdmapnum, data->psdvarnum);
  std::swap(data->psdmapdim, data->psdvardim);

  // psdmap's can not belong to negative domains
  flipsign->h = !flipsign->h;
  flipsign->d = !flipsign->d;

  return CBF_RES_OK;
}

static CBFresponsee flip_objsense(CBFdata *data, CBFtransform_flipsign *flipsign)
{
  if (data->objsense == CBF_OBJ_MAXIMIZE) {

    data->objsense = CBF_OBJ_MINIMIZE;

    // Flip sign of objective before dualization
    flipsign->obja = !flipsign->obja;
    flipsign->objf = !flipsign->objf;

    // Flip sign of objective after dualization
    flipsign->b = !flipsign->b;
    flipsign->d = !flipsign->d;

  } else {

    data->objsense = CBF_OBJ_MAXIMIZE;
  }

  return CBF_RES_OK;
}

static CBFresponsee flip_signs(CBFdata *data, CBFtransform_flipsign *flipsign)
{
  long long int i;

  if (flipsign->obja)
    for (i = 0; i < data->objannz; ++i)
      data->objaval[i] = -data->objaval[i];

  if (flipsign->objf)
    for (i = 0; i < data->objfnnz; ++i)
      data->objfval[i] = -data->objfval[i];

  if (flipsign->b)
    for (i = 0; i < data->bnnz; ++i)
    data->bval[i] = -data->bval[i];

  if (flipsign->d)
    for (i = 0; i < data->dnnz; ++i)
      data->dval[i] = -data->dval[i];

  if (flipsign->a)
    for (i = 0; i < data->annz; ++i)
      data->aval[i] = -data->aval[i];

  if (flipsign->f)
    for (i = 0; i < data->fnnz; ++i)
      data->fval[i] = -data->fval[i];

  if (flipsign->h)
    for (i = 0; i < data->hnnz; ++i)
      data->hval[i] = -data->hval[i];

  return CBF_RES_OK;
}
