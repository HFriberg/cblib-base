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

#include "frontend-mosek.h"
#include "cbf-helper.h"
#include "mosek.h"

#include <stddef.h>

static CBFresponsee
  read(const char *file, CBFdata *data, CBFfrontendmemory *mem);

static void
  clean(CBFdata *data, CBFfrontendmemory *mem);

static void
  mosekprint(void *handle, MSKCONST char str[]);

static MSKrescodee
  readVER(MSKtask_t task, CBFdyndata *dyndata);

static MSKrescodee
  readOBJSENSE(MSKtask_t task, CBFdyndata *dyndata);

static MSKrescodee
  readPSDINFO(MSKtask_t task, CBFdyndata *dyndata);

static MSKrescodee
  readINTVARINFO(MSKtask_t task, CBFdyndata *dyndata);

static MSKrescodee
  readOBJECTIVE(MSKtask_t task, CBFdyndata *dyndata);

static MSKrescodee
  readOBJECTIVE_FCOORD(MSKtask_t task, CBFdyndata *dyndata);

static MSKrescodee
  readOBJECTIVE_ACOORD(MSKtask_t task, CBFdyndata *dyndata);

static MSKrescodee
  readOBJECTIVE_BCOORD(MSKtask_t task, CBFdyndata *dyndata);

static MSKrescodee
  readCONSTRAINTS(MSKtask_t task, CBFdyndata *dyndata);

static MSKrescodee
  readCONSTRAINTS_FCOORD(MSKtask_t task, CBFdyndata *dyndata, const long long int *subimap1, const long long int *subimap2);

static MSKrescodee
  readCONSTRAINTS_ACOORD(MSKtask_t task, CBFdyndata *dyndata, const long long int *subimap1, const long long int *subimap2);

static MSKrescodee
  readVARIABLES(MSKtask_t task, CBFdyndata *dyndata);

static MSKrescodee
  readVARIABLES_CONES(MSKtask_t task, CBFdyndata *dyndata, MSKint32t numvar, MSKint32t *dim, CBFscalarconee *domain, CBFscalarconee *lindomain);

static MSKrescodee
  readVARIABLES_CONES_ISORDERED(MSKconetypee type, MSKint32t nummembers, const MSKint32t *members, int *isordered, int *firstidx);

static MSKrescodee
  readVARIABLES_BOUNDS(MSKtask_t task, CBFdyndata *dyndata, MSKint32t numvar, MSKint32t *dim, CBFscalarconee *domain, CBFscalarconee *lindomain);


// -------------------------------------
// Global variable
// -------------------------------------

CBFfrontend const frontend_mosek = { "mosek", read, clean };


// -------------------------------------
// Function definitions
// -------------------------------------

static CBFresponsee read(const char *file, CBFdata *data, CBFfrontendmemory *mem)
{
  MSKenv_t    env  = NULL;
  MSKtask_t   task = NULL;
  CBFdyndata *dyndata = NULL;
  MSKproblemtypee ptype;
  MSKrescodee res  = MSK_RES_OK, res2;
  char response[MSK_MAX_STR_LEN] = "";

  //
  // Use CBFfrontendmemory to store the CBFdyndata wrapper of CBFdata
  //
  *mem = calloc(1, sizeof(*dyndata));
  if (!*mem) {
    return CBF_RES_ERR;
  } else {
    dyndata = (CBFdyndata*)*mem;
    dyndata->data = data;
  }

  res = MSK_makeenv(&env, NULL);

  if ( res==MSK_RES_OK )
    res = MSK_maketask(env, 0, 0, &task);

  if ( res==MSK_RES_OK )
    res = MSK_linkfunctotaskstream(task, MSK_STREAM_ERR, NULL, mosekprint);

  if ( res==MSK_RES_OK )
    res = MSK_linkfunctotaskstream(task, MSK_STREAM_WRN, NULL, mosekprint);

  if ( res==MSK_RES_OK )
    res = MSK_putintparam(task, MSK_IPAR_READ_MPS_FORMAT, MSK_MPS_FORMAT_FREE);

  if ( res==MSK_RES_OK )
  {
    res = MSK_readdata(task, file);

    if ( res!=MSK_RES_OK ) {
      // Maybe spaces are preventing read?

      res2 = MSK_putintparam(task, MSK_IPAR_READ_MPS_FORMAT, MSK_MPS_FORMAT_RELAXED);

      if ( res2==MSK_RES_OK )
        res2 = MSK_readdata(task, file);

      if ( res2==MSK_RES_OK ) {
        printf("<<<< Continued using old MPS format definition.\n");
        res = MSK_RES_OK;
      }
    }
  }

  if ( res==MSK_RES_OK )
    res = MSK_getprobtype(task, &ptype);

  if ( res==MSK_RES_OK )
  {
    if (ptype==MSK_PROBTYPE_QO || ptype==MSK_PROBTYPE_QCQO)
    {
      printf("*** Converting from quadratic to conic form using MSK_toconic ... ");
      res = MSK_toconic(task);
      printf("DONE.\n");
    }
  }

  if ( res==MSK_RES_OK )
    res = readVER(task, dyndata);

  if ( res==MSK_RES_OK )
    res = readOBJSENSE(task, dyndata);

  if ( res==MSK_RES_OK )
    res = readPSDINFO(task, dyndata);

  if ( res==MSK_RES_OK )
    res = readINTVARINFO(task, dyndata);

  if ( res==MSK_RES_OK )
    res = readOBJECTIVE(task, dyndata);

  if ( res==MSK_RES_OK )
    res = readCONSTRAINTS(task, dyndata);

  if ( res==MSK_RES_OK )
    res = readVARIABLES(task, dyndata);

  // Get response and close connection
  MSK_getcodedesc(res, NULL, response);
  MSK_deletetask(&task);
  MSK_deleteenv(&env);

  // Translate MSKrescodee to CBFfrontendresponsee
  switch (res) {
  case MSK_RES_OK:
    return CBF_RES_OK;
  default:
    printf("%s\n", response);
    clean(data, mem);
    return CBF_RES_ERR;
  }
}

static void clean(CBFdata *data, CBFfrontendmemory *mem)
{
  //
  // Free memory allocated by the cbf-helper module
  //
  if (*mem) {
    CBFdyn_freedynamicallocations( (CBFdyndata*)*mem );
    free(*mem);
    *mem = NULL;
  }

  //
  // Free remaining memory allocated by this module
  //
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

static void mosekprint(void *handle, MSKCONST char str[])
{
  printf("%s",str);
}

static MSKrescodee readVER(MSKtask_t task, CBFdyndata *dyndata)
{
  dyndata->data->ver = CBF_VERSION;
  return MSK_RES_OK;
}

static MSKrescodee readOBJSENSE(MSKtask_t task, CBFdyndata *dyndata)
{
  MSKrescodee res = MSK_RES_OK;
  MSKobjsensee sense;

  res = MSK_getobjsense(task, &sense);

  if ( res==MSK_RES_OK )
  {
    if (sense == MSK_OBJECTIVE_SENSE_MINIMIZE)
      dyndata->data->objsense = CBF_OBJ_MINIMIZE;

    else if (sense == MSK_OBJECTIVE_SENSE_MAXIMIZE)
      dyndata->data->objsense = CBF_OBJ_MAXIMIZE;

    else
      res = MSK_RES_ERR_INTERNAL;
  }

  return res;
}

static MSKrescodee readINTVARINFO(MSKtask_t task, CBFdyndata *dyndata)
{
  MSKrescodee res = MSK_RES_OK;
  MSKint32t numintvar, numvar;
  MSKvariabletypee vartype;
  MSKint32t i;

  res = MSK_getnumintvar(task, &numintvar);

  if ( res==MSK_RES_OK && numintvar >= 1 )
  {
    if ( CBFdyn_intvar_capacitysurplus(dyndata, numintvar) == CBF_RES_ERR )
      res = MSK_RES_ERR_SPACE;

    if ( res==MSK_RES_OK )
      res = MSK_getnumvar(task, &numvar);

    for (i=0; i<numvar && res==MSK_RES_OK; ++i) {
      res = MSK_getvartype(task, i, &vartype);

      if ( res==MSK_RES_OK && vartype==MSK_VAR_TYPE_INT )
        CBFdyn_intvar_add(dyndata, i);
    }
  }

  return res;
}

static MSKrescodee readPSDINFO(MSKtask_t task, CBFdyndata *dyndata)
{
  MSKrescodee res = MSK_RES_OK;
  MSKint32t numpsdvar, psdvardim;
  MSKint32t i;

  //
  // PSD variables
  //
  res = MSK_getnumbarvar(task, &numpsdvar);

  if ( res==MSK_RES_OK && numpsdvar >= 1 )
  {
    if ( CBFdyn_psdvar_capacitysurplus(dyndata, numpsdvar) == CBF_RES_ERR )
      res = MSK_RES_ERR_SPACE;

    for (i=0; i<numpsdvar && res==MSK_RES_OK; ++i)
    {
      res = MSK_getdimbarvarj(task, i, &psdvardim);

      if ( res==MSK_RES_OK )
        CBFdyn_psdvar_add(dyndata, psdvardim);
    }
  }

  //
  // PSD constraints
  //
  dyndata->data->psdmapdim = 0;
  dyndata->data->hnnz = 0;
  dyndata->data->dnnz = 0;

  return res;
}

static MSKrescodee readOBJECTIVE(MSKtask_t task, CBFdyndata *dyndata)
{
  MSKrescodee res = MSK_RES_OK;

  //
  // Fill in objective coefficients
  //
  if ( res==MSK_RES_OK )
    res = readOBJECTIVE_FCOORD(task, dyndata);

  if ( res==MSK_RES_OK )
    res = readOBJECTIVE_ACOORD(task, dyndata);

  if ( res==MSK_RES_OK )
    res = readOBJECTIVE_BCOORD(task, dyndata);

  return res;
}

static MSKrescodee readOBJECTIVE_FCOORD(MSKtask_t task, CBFdyndata *dyndata)
{
  MSKrescodee res = MSK_RES_OK;
  MSKint64t maxbarcnnz, barcnnz, i;
  MSKint32t *barcsubj = NULL, *barcsubk = NULL, *barcsubl = NULL;
  MSKrealt  *barcval = NULL;

  res = MSK_getnumbarcblocktriplets(task, &maxbarcnnz);

  if ( res==MSK_RES_OK && maxbarcnnz >= 1 )
  {
    barcsubj = (MSKint32t*) calloc(maxbarcnnz, sizeof(barcsubj[0]));
    barcsubk = (MSKint32t*) calloc(maxbarcnnz, sizeof(barcsubk[0]));
    barcsubl = (MSKint32t*) calloc(maxbarcnnz, sizeof(barcsubl[0]));
    barcval  = (MSKrealt*)  calloc(maxbarcnnz, sizeof(barcval[0]));

    if (!barcsubj || !barcsubk || !barcsubl || !barcval) {
      if (barcsubj)     free(barcsubj);
      if (barcsubk)     free(barcsubk);
      if (barcsubl)     free(barcsubl);
      if (barcval)      free(barcval);
      return MSK_RES_ERR_SPACE;
    }

    if ( res==MSK_RES_OK )
      res = MSK_getbarcblocktriplet(task, maxbarcnnz, &barcnnz, barcsubj, barcsubk, barcsubl, barcval);

    if ( res==MSK_RES_OK )
      if ( CBFdyn_objf_capacitysurplus(dyndata, barcnnz) == CBF_RES_ERR)
        res = MSK_RES_ERR_SPACE;

    for (i=0; i<barcnnz && res==MSK_RES_OK; ++i)
      if ( CBFdyn_objf_add(dyndata, barcsubj[i], barcsubk[i], barcsubl[i], barcval[i]) == CBF_RES_ERR )
        res = MSK_RES_ERR_INTERNAL;

    free(barcsubj);
    free(barcsubk);
    free(barcsubl);
    free(barcval);
  }

  return res;
}

static MSKrescodee readOBJECTIVE_ACOORD(MSKtask_t task, CBFdyndata *dyndata)
{
  MSKrescodee res = MSK_RES_OK;
  MSKint32t numvar, i;
  MSKrealt *c = NULL;

  res = MSK_getnumvar(task, &numvar);

  if ( res==MSK_RES_OK && numvar >= 1 )
  {
    c = (MSKrealt*) calloc(numvar, sizeof(c[0]));
    if (!c) {
      return MSK_RES_ERR_SPACE;
    }

    res = MSK_getc(task, c);

    if ( res==MSK_RES_OK )
      if ( CBFdyn_obja_capacitysurplus(dyndata, numvar) == CBF_RES_ERR )
        res = MSK_RES_ERR_SPACE;

    for (i=0; i<numvar && res==MSK_RES_OK; ++i)
      if (c[i] != 0.0)
        if ( CBFdyn_obja_add(dyndata, i, c[i]) == CBF_RES_ERR )
          res = MSK_RES_ERR_INTERNAL;

    free(c);
  }

  return res;
}

static MSKrescodee readOBJECTIVE_BCOORD(MSKtask_t task, CBFdyndata *dyndata)
{
  MSKrescodee res = MSK_RES_OK;
  MSKrealt cfix;

  res = MSK_getcfix(task, &cfix);

  if ( res==MSK_RES_OK )
    CBFdyn_objb_set(dyndata, cfix);

  return res;
}

static MSKrescodee readCONSTRAINTS(MSKtask_t task, CBFdyndata *dyndata)
{
  MSKrescodee res = MSK_RES_OK;
  MSKint32t numcon;
  MSKboundkeye *bkc = NULL;
  MSKrealt *blc = NULL, *buc = NULL;
  long long int *subimap1 = NULL, *subimap2 = NULL;
  MSKint32t i;

  res = MSK_getnumcon(task, &numcon);

  if ( res==MSK_RES_OK && numcon >= 1 )
  {
    //
    // Get MOSEK constraint bound keys
    //
    bkc = (MSKboundkeye*) calloc(numcon, sizeof(bkc[0]));
    blc = (MSKrealt*) calloc(numcon, sizeof(blc[0]));
    buc = (MSKrealt*) calloc(numcon, sizeof(buc[0]));
    subimap1 = (long long int *) calloc(numcon, sizeof(subimap1[0]));
    subimap2 = (long long int *) calloc(numcon, sizeof(subimap2[0]));

    if (!bkc || !blc || !buc || !subimap1 || !subimap2) {
      if (bkc)          free(bkc);
      if (blc)          free(blc);
      if (buc)          free(buc);
      if (subimap1)     free(subimap1);
      if (subimap2)     free(subimap2);
      return MSK_RES_ERR_SPACE;
    }

    res = MSK_getconboundslice(task, 0, numcon, bkc, blc, buc);

    //
    // Translate MOSEK constraints into CBF domains
    //
    if ( res==MSK_RES_OK )
      if ( CBFdyn_map_capacitysurplus(dyndata, 2*numcon) == CBF_RES_ERR )
        res = MSK_RES_ERR_SPACE;

    if ( res==MSK_RES_OK )
      if ( CBFdyn_b_capacitysurplus(dyndata, 2*numcon) == CBF_RES_ERR )
        res = MSK_RES_ERR_SPACE;

    for (i=0; i<numcon && res==MSK_RES_OK; ++i)
    {
      switch (bkc[i]) {

      case MSK_BK_FR:
        subimap1[i] = -1;
        subimap2[i] = -1;
        break;

      case MSK_BK_LO:
        if ( CBFdyn_map_adddomain(dyndata, CBF_CONE_POS, 1) == CBF_RES_ERR )
          res = MSK_RES_ERR_INTERNAL;

        subimap1[i] = dyndata->data->mapnum-1;
        subimap2[i] = -1;

        if ( res==MSK_RES_OK )
          if ( blc[i] != 0.0 )
            if ( CBFdyn_b_add(dyndata, subimap1[i], -blc[i]) == CBF_RES_ERR )
              res = MSK_RES_ERR_INTERNAL;
        break;

      case MSK_BK_UP:
        if ( CBFdyn_map_adddomain(dyndata, CBF_CONE_NEG, 1) == CBF_RES_ERR )
          res = MSK_RES_ERR_INTERNAL;

        subimap1[i] = dyndata->data->mapnum-1;
        subimap2[i] = -1;

        if ( res==MSK_RES_OK )
          if ( buc[i] != 0.0 )
            if ( CBFdyn_b_add(dyndata, subimap1[i], -buc[i]) == CBF_RES_ERR )
              res = MSK_RES_ERR_INTERNAL;
        break;

      case MSK_BK_FX:
        if ( CBFdyn_map_adddomain(dyndata, CBF_CONE_ZERO, 1) == CBF_RES_ERR )
          res = MSK_RES_ERR_INTERNAL;

        subimap1[i] = dyndata->data->mapnum-1;
        subimap2[i] = -1;

        if ( res==MSK_RES_OK )
          if ( blc[i] != 0.0 )
            if ( CBFdyn_b_add(dyndata, subimap1[i], -blc[i]) == CBF_RES_ERR )
              res = MSK_RES_ERR_INTERNAL;
        break;

      case MSK_BK_RA:
        if ( CBFdyn_map_adddomain(dyndata, CBF_CONE_POS, 1) == CBF_RES_ERR )
          res = MSK_RES_ERR_INTERNAL;
        if ( res==MSK_RES_OK )
          if ( CBFdyn_map_adddomain(dyndata, CBF_CONE_NEG, 1) == CBF_RES_ERR )
            res = MSK_RES_ERR_INTERNAL;

        subimap1[i] = dyndata->data->mapnum-2;
        subimap2[i] = dyndata->data->mapnum-1;

        if ( res==MSK_RES_OK )
          if ( blc[i] != 0.0 )
            if ( CBFdyn_b_add(dyndata, subimap1[i], -blc[i]) == CBF_RES_ERR )
              res = MSK_RES_ERR_INTERNAL;
        if ( res==MSK_RES_OK )
          if ( buc[i] != 0.0 )
            if ( CBFdyn_b_add(dyndata, subimap2[i], -buc[i]) == CBF_RES_ERR )
              res = MSK_RES_ERR_INTERNAL;
        break;
      default:
        res = MSK_RES_ERR_INTERNAL;
        break;
      }
    }

    //
    // Fill in constraint coefficients
    //
    if ( res==MSK_RES_OK )
      res = readCONSTRAINTS_FCOORD(task, dyndata, subimap1, subimap2);

    if ( res==MSK_RES_OK )
      res = readCONSTRAINTS_ACOORD(task, dyndata, subimap1, subimap2);

    free(bkc);
    free(blc);
    free(buc);
    free(subimap1);
    free(subimap2);
  }

  return res;
}

static MSKrescodee readCONSTRAINTS_FCOORD(MSKtask_t task, CBFdyndata *dyndata, const long long int *subimap1, const long long int *subimap2)
{
  MSKrescodee res = MSK_RES_OK;
  MSKint64t maxbarannz, barannz, i;
  MSKint32t *barasubi = NULL, *barasubj = NULL, *barasubk = NULL, *barasubl = NULL;
  MSKrealt  *baraval = NULL;

  res = MSK_getnumbarablocktriplets(task, &maxbarannz);

  if ( res==MSK_RES_OK && maxbarannz >= 1 )
  {
    barasubi = (MSKint32t*) calloc(maxbarannz, sizeof(barasubi[0]));
    barasubj = (MSKint32t*) calloc(maxbarannz, sizeof(barasubj[0]));
    barasubk = (MSKint32t*) calloc(maxbarannz, sizeof(barasubk[0]));
    barasubl = (MSKint32t*) calloc(maxbarannz, sizeof(barasubl[0]));
    baraval  = (MSKrealt*)  calloc(maxbarannz, sizeof(baraval[0]));

    if (!barasubi || !barasubj || !barasubk || !barasubl || !baraval) {
      if (barasubi)     free(barasubi);
      if (barasubj)     free(barasubj);
      if (barasubk)     free(barasubk);
      if (barasubl)     free(barasubl);
      if (baraval)      free(baraval);
      return MSK_RES_ERR_SPACE;
    }

    if ( res==MSK_RES_OK )
      res = MSK_getbarablocktriplet(task, maxbarannz, &barannz, barasubi, barasubj, barasubk, barasubl, baraval);

    if ( res==MSK_RES_OK )
      if ( CBFdyn_f_capacitysurplus(dyndata, 2*barannz) == CBF_RES_ERR)
        res = MSK_RES_ERR_SPACE;

    for (i=0; i<barannz && res==MSK_RES_OK; ++i) {
      if (subimap1[barasubi[i]] != -1)
        if ( CBFdyn_f_add(dyndata, subimap1[barasubi[i]], barasubj[i], barasubk[i], barasubl[i], baraval[i]) == CBF_RES_ERR )
          res = MSK_RES_ERR_INTERNAL;

      if ( res==MSK_RES_OK && subimap2[barasubi[i]] != -1)
        if ( CBFdyn_f_add(dyndata, subimap2[barasubi[i]], barasubj[i], barasubk[i], barasubl[i], baraval[i]) == CBF_RES_ERR )
          res = MSK_RES_ERR_INTERNAL;
    }

    free(barasubi);
    free(barasubj);
    free(barasubk);
    free(barasubl);
    free(baraval);
  }

  return res;
}

static MSKrescodee readCONSTRAINTS_ACOORD(MSKtask_t task, CBFdyndata *dyndata, const long long int *subimap1, const long long int *subimap2)
{
  MSKrescodee res = MSK_RES_OK;
  MSKint64t surp, annz, i;
  MSKint32t numvar;
  MSKint32t *asubi=NULL, *asubj=NULL;
  MSKrealt  *aval=NULL;

  res = MSK_getnumanz64(task, &annz);

  if ( res==MSK_RES_OK && annz >= 1 )
  {
    asubi = (MSKint32t*) calloc(annz, sizeof(asubi[0]));
    asubj = (MSKint32t*) calloc(annz, sizeof(asubj[0]));
    aval  =  (MSKrealt*) calloc(annz, sizeof(aval[0]));

    if (!asubi || !asubj || !aval) {
      if (asubi)    free(asubi);
      if (asubj)    free(asubj);
      if (aval)     free(aval);
      return MSK_RES_ERR_SPACE;
    }

    res = MSK_getnumvar(task, &numvar);

    if ( res==MSK_RES_OK ) {
      surp = annz;
      res = MSK_getacolslicetrip(task, 0, numvar, annz, &surp, asubi, asubj, aval);
    }

    if ( res==MSK_RES_OK )
      if ( CBFdyn_a_capacitysurplus(dyndata, 2*annz) == CBF_RES_ERR)
        res = MSK_RES_ERR_SPACE;

    for (i=0; i<annz && res==MSK_RES_OK; ++i) {
      if (subimap1[asubi[i]] != -1)
        if ( CBFdyn_a_add(dyndata, subimap1[asubi[i]], asubj[i], aval[i]) == CBF_RES_ERR ) {
          res = MSK_RES_ERR_INTERNAL;
        }

      if ( res==MSK_RES_OK && subimap2[asubi[i]] != -1 )
        if ( CBFdyn_a_add(dyndata, subimap2[asubi[i]], asubj[i], aval[i]) == CBF_RES_ERR ) {
          res = MSK_RES_ERR_INTERNAL;
        }
    }

    free(asubi);
    free(asubj);
    free(aval);
  }

  return res;
}

static MSKrescodee readVARIABLES(MSKtask_t task, CBFdyndata *dyndata)
{
  MSKrescodee res = MSK_RES_OK;
  MSKint32t numvar, i;
  MSKint32t *dim = NULL;
  CBFscalarconee *domain = NULL, *lindomain = NULL;     // variable 'lindomain' is the linear domain
                                                        // (free, L+, L-, L=) implied by variable 'domain',
                                                        // used when evaluating bounds.

  res = MSK_getnumvar(task, &numvar);

  if ( res==MSK_RES_OK && numvar >= 1 )
  {
    dim       = (MSKint32t*)      malloc(numvar * sizeof(dim[0]));
    domain    = (CBFscalarconee*) malloc(numvar * sizeof(domain[0]));
    lindomain = (CBFscalarconee*) malloc(numvar * sizeof(lindomain[0]));

    if (!dim || !domain || !lindomain) {
      if (dim)          free(dim);
      if (domain)       free(domain);
      if (lindomain)    free(lindomain);
      return MSK_RES_ERR_SPACE;
    }

    for (i=0; i<numvar; ++i) {
      dim[i] = 1;
      domain[i] = CBF_CONE_END;
      lindomain[i] = CBF_CONE_FREE;
    }

    //
    // Fill in variable domains
    // (only 'readVARIABLES_BOUNDS' is able to handle the situation when a variable
    // already has a specified domain - such that bound enforcement has to go through
    // CBF constraints - and thus last in line)
    //
    if ( res==MSK_RES_OK )
      res = readVARIABLES_CONES(task, dyndata, numvar, dim, domain, lindomain);

    if ( res==MSK_RES_OK )
      res = readVARIABLES_BOUNDS(task, dyndata, numvar, dim, domain, lindomain);

    //
    // Add variables domains to CBFdyndata structure
    //
    if ( res==MSK_RES_OK )
      if ( CBFdyn_var_capacitysurplus(dyndata, numvar) == CBF_RES_ERR )
        res = MSK_RES_ERR_SPACE;

    i=0;
    while (i < numvar && res==MSK_RES_OK) {
      if ( CBFdyn_var_adddomain(dyndata, domain[i], dim[i]) == CBF_RES_ERR ) {
        res = MSK_RES_ERR_INTERNAL;
      }

      i += dim[i];
    }

    free(dim);
    free(domain);
    free(lindomain);
  }

  return res;
}

static MSKrescodee readVARIABLES_CONES(MSKtask_t task, CBFdyndata *dyndata, MSKint32t numvar, MSKint32t *dim, CBFscalarconee *domain, CBFscalarconee *lindomain)
{
  MSKrescodee res = MSK_RES_OK;
  MSKint32t numcones, nummembers, firstidx, i, j;
  MSKint32t *members = NULL;
  MSKconetypee type;
  CBFscalarconee cbftype;
  int isordered;

  if ( res==MSK_RES_OK )
    res = MSK_getnumcone(task, &numcones);

  for (i=0; i<numcones && res==MSK_RES_OK; ++i)
  {
    // Identify cone type
    res = MSK_getconeinfo(task, i, &type, NULL, &nummembers);

    switch (type) {
    case MSK_CT_QUAD:   cbftype = CBF_CONE_QUAD;  break;
    case MSK_CT_RQUAD:  cbftype = CBF_CONE_RQUAD; break;
    default:
      res = MSK_RES_ERR_INTERNAL;
      break;
    }

    if ( res==MSK_RES_OK && nummembers >= 1 )
    {
      // Extract cone members
      members = (MSKint32t*) malloc(nummembers * sizeof(members[0]));
      if (!members) {
        res = MSK_RES_ERR_SPACE;
        break;
      }

      if ( res==MSK_RES_OK )
        res = MSK_getcone(task, i, NULL, NULL, NULL, members);

      if ( res==MSK_RES_OK )
        res = readVARIABLES_CONES_ISORDERED(type, nummembers, members, &isordered, &firstidx);

      if ( res==MSK_RES_OK ) {
        if (isordered) {
          //
          // Ordered cones are added as variable domains
          //
          dim[firstidx]    = nummembers;
          domain[firstidx] = cbftype;

          switch (type) {
          case MSK_CT_QUAD:
            lindomain[firstidx] = CBF_CONE_POS;
            break;
          case MSK_CT_RQUAD:
            lindomain[firstidx] = CBF_CONE_POS;
            lindomain[firstidx+1] = CBF_CONE_POS;
            break;
          default:
            res = MSK_RES_ERR_INTERNAL;
            break;
          }
        }
        else
        {
          //
          // Unordered cones are added as constraints
          //
          if ( CBFdyn_a_capacitysurplus(dyndata, nummembers) == CBF_RES_ERR )
            res = MSK_RES_ERR_SPACE;

          if ( res==MSK_RES_OK )
            if ( CBFdyn_map_capacitysurplus(dyndata, nummembers) == CBF_RES_ERR )
              res = MSK_RES_ERR_SPACE;

          if ( res==MSK_RES_OK )
            if ( CBFdyn_map_adddomain(dyndata, cbftype, nummembers) == CBF_RES_ERR )
              res = MSK_RES_ERR_INTERNAL;

          for (j=0; j<nummembers && res==MSK_RES_OK; ++j)
            if ( CBFdyn_a_add(dyndata, dyndata->data->mapnum-nummembers+j, members[j], 1.0) == CBF_RES_ERR )
              res = MSK_RES_ERR_INTERNAL;
        }
      }

      free(members);
    }
  }

  return res;
}

static MSKrescodee readVARIABLES_CONES_ISORDERED(MSKconetypee type, MSKint32t nummembers, const MSKint32t *members, int *isordered, MSKint32t *firstidx)
{
  MSKint32t min, max, i;

  min = max = members[0];
  for (i=1; i<nummembers; ++i) {
    if (members[i] < min)
      min = members[i];
    if (members[i] > max)
      max = members[i];
  }

  // Assume cone is ordered
  *isordered = 1;
  *firstidx  = min;

  // Are they consecutive?
  if (max - min + 1 != nummembers)
    *isordered = 0;

  switch (type) {
  case MSK_CT_QUAD:
    // Is cone root first?
    if (members[0] != min)
      *isordered = 0;
    break;
  case MSK_CT_RQUAD:
    // Are the two cone roots first?
    if (members[0] != min && members[0] != min+1)
      *isordered = 0;
    if (members[1] != min && members[1] != min+1)
      *isordered = 0;
    break;
  default:
    return MSK_RES_ERR_INTERNAL;
    break;
  }

  return MSK_RES_OK;
}

static MSKrescodee readVARIABLES_BOUNDS(MSKtask_t task, CBFdyndata *dyndata, MSKint32t numvar, MSKint32t *dim, CBFscalarconee *domain, CBFscalarconee *lindomain)
{
  MSKrescodee res = MSK_RES_OK;
  MSKboundkeye *bkx = NULL;
  MSKrealt *blx = NULL, *bux = NULL;
  MSKint32t i;

  bkx = (MSKboundkeye*) malloc(numvar * sizeof(bkx[0]));
  blx = (MSKrealt*)     malloc(numvar * sizeof(blx[0]));
  bux = (MSKrealt*)     malloc(numvar * sizeof(bux[0]));

  if (!bkx || !blx || !bux) {
    if (bkx)    free(bkx);
    if (blx)    free(blx);
    if (bux)    free(bux);
    return MSK_RES_ERR_SPACE;
  }

  if ( res==MSK_RES_OK )
    res = MSK_getvarboundslice(task, 0, numvar, bkx, blx, bux);

  i = 0;
  while (i < numvar && res==MSK_RES_OK)
  {
    if (domain[i] == CBF_CONE_END)
    {
      //
      // Populate 'domain' with cone best matching bound key.
      //
      dim[i] = 1;

      switch (bkx[i]) {
      case MSK_BK_FR:
        domain[i] = lindomain[i] = CBF_CONE_FREE;
        break;
      case MSK_BK_LO:
        if (blx[i] >= 0.0)
          domain[i] = lindomain[i] = CBF_CONE_POS;
        else
          domain[i] = lindomain[i] = CBF_CONE_FREE;
        break;
      case MSK_BK_UP:
        if (bux[i] <= 0.0)
          domain[i] = lindomain[i] = CBF_CONE_NEG;
        else
          domain[i] = lindomain[i] = CBF_CONE_FREE;
        break;
      case MSK_BK_FX:
        if (blx[i] == 0.0)
          domain[i] = lindomain[i] = CBF_CONE_ZERO;
        else
          domain[i] = lindomain[i] = CBF_CONE_FREE;
        break;
      case MSK_BK_RA:
        if (blx[i] == 0.0 && bux[i] == 0.0)
          domain[i] = lindomain[i] = CBF_CONE_ZERO;
        else if (blx[i] >= 0.0)
          domain[i] = lindomain[i] = CBF_CONE_POS;
        else if (bux[i] <= 0.0)
          domain[i] = lindomain[i] = CBF_CONE_NEG;
        else
          domain[i] = lindomain[i] = CBF_CONE_FREE;
        break;
      default:
        res = MSK_RES_ERR_INTERNAL;
        break;
      }
    }

    i += dim[i];
  }

  for (i=0; i < numvar && res==MSK_RES_OK; ++i)
  {
    //
    // Add bounds missing from 'lindomain' as constraints
    // (no bound is stricter than fixing)
    //
    if ( res==MSK_RES_OK && lindomain[i] != CBF_CONE_ZERO )
    {
      if ( CBFdyn_varbound_capacitysurplus(dyndata, 2) == CBF_RES_ERR)
        res = MSK_RES_ERR_SPACE;

      if ( res==MSK_RES_OK ) {
        switch (bkx[i]) {
        case MSK_BK_FR:
          break;
        case MSK_BK_LO:
          if (!(blx[i] == 0.0 && lindomain[i] == CBF_CONE_POS))
            if ( CBFdyn_varbound_addlower(dyndata, i, blx[i]) == CBF_RES_ERR )
              res = MSK_RES_ERR_INTERNAL;
          break;
        case MSK_BK_UP:
          if (!(bux[i] == 0.0 && lindomain[i] == CBF_CONE_NEG))
            if ( CBFdyn_varbound_addupper(dyndata, i, bux[i]) == CBF_RES_ERR )
              res = MSK_RES_ERR_INTERNAL;
          break;
        case MSK_BK_FX:
          if (!(blx[i] == 0.0 && lindomain[i] == CBF_CONE_ZERO))
            if ( CBFdyn_varbound_addfix(dyndata, i, blx[i]) == CBF_RES_ERR )
              res = MSK_RES_ERR_INTERNAL;
          break;
        case MSK_BK_RA:
          if (!(blx[i] == 0.0 && lindomain[i] == CBF_CONE_POS))
            if ( CBFdyn_varbound_addlower(dyndata, i, blx[i]) == CBF_RES_ERR )
              res = MSK_RES_ERR_INTERNAL;

          if (!(bux[i] == 0.0 && lindomain[i] == CBF_CONE_NEG))
            if ( CBFdyn_varbound_addupper(dyndata, i, bux[i]) == CBF_RES_ERR )
              res = MSK_RES_ERR_INTERNAL;
          break;
        default:
          res = MSK_RES_ERR_INTERNAL;
          break;
        }
      }
    }
  }

  free(bkx);
  free(blx);
  free(bux);

  return res;
}
