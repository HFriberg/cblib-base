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

#ifndef CBF_CBF_HELPER_H
#define CBF_CBF_HELPER_H

#include "programmingstyle.h"
#include "cbf-data.h"


/*
 * CBF_bucketsort is a stable sort (low to high) of the sequence { val[idx[i]] }_i.
 * CBF_coordinatesort uses it to sort by 'i' (primarily), followed by 'j', 'k' and 'l'.
 */
CBFresponsee
CBF_bucketsort(long long int maxval, long long int nnz, const long long int *val, long long int *idx);

CBFresponsee
CBF_bucketsort(long long int maxval, long long int nnz, const int *val, long long int *idx);

CBFresponsee
CBF_coordinatesort(long long int *i, double *v, long long int nnz, long long int maxi);

CBFresponsee
CBF_coordinatesort(long long int *i, long long int *j, double *v, long long int nnz, long long int maxi, long long int maxj);

CBFresponsee
CBF_coordinatesort(int *i, int *j, int *k, double *v, long long int nnz, long long int maxi, long long int maxj, long long int maxk);

CBFresponsee
CBF_coordinatesort(long long int *i, int *j, int *k, int *l, double *v, long long int nnz, long long int maxi, long long int maxj, long long int maxk, long long int maxl);

CBFresponsee
CBF_coordinatesort(int *i, long long int *j, int *k, int *l, double *v, long long int nnz, long long int maxi, long long int maxj, long long int maxk, long long int maxl);

CBFresponsee
CBF_coordinatesort_rowmajor_map(CBFdata *data);

CBFresponsee
CBF_coordinatesort_rowmajor_psdmap(CBFdata *data);


/*
 * These methods can find the nnz's of a particular map or psdmap.
 * WARNING: Assumes coordinates are sorted row-major
 */
CBFresponsee
CBF_findforward_map(const CBFdata *data, long long int map, long long int *fbeg, long long int *abeg, long long int *bbeg);

CBFresponsee
CBF_findforward_psdmap(const CBFdata *data, long long int psdmap, long long int *hbeg, long long int *dbeg);

CBFresponsee
CBF_findbackward_map(const CBFdata *data, long long int map, long long int *fend, long long int *aend, long long int *bend);

CBFresponsee
CBF_findbackward_psdmap(const CBFdata *data, long long int psdmap, long long int *hend, long long int *dend);


/*
 * Helps you allocate, initialize and free a 0-1 array
 * indicating whether a 'var' index belongs to 'intvar'.
 */
CBFresponsee
CBFintegerarray_init(CBFdata *data, char **integertable);

void
CBFintegerarray_free(char **integertable);


/*
 * Helps you delete maps/psdmaps and get rid of empty nnz
 */
CBFresponsee
CBF_compress_maps(CBFdata *data, const char *delmap);

CBFresponsee
CBF_compress_psdmaps(CBFdata *data, const char *delpsdmap);


/*
 * The CBFdyndata structure, makes it easy to populate
 * parts of the CBFdata structure dynamically. One example
 * would be to add bursts of triplets
 * (asubi, asubj, aval)
 *
 * Note that the cbf-helper module should ideally perform
 * all memory allocations and deallocations for the parts
 * of the CBFdata structure used dynamically!
 *
 * CBF_*_capacitysurplus will ensure room for coming elements,
 * by copying data to a new memory location if necessary.
 *
 * CBF_freedynamicallocations will deallocate only the parts
 * of the CBFdata structured that have been used dynamically.
 */
typedef struct CBFdyndata_struct {

  CBFdata *data;

  // Dynamic capacities (0 if not dynamically allocated)
  long long int mapstackdyncap;
  long long int varstackdyncap;
  long long int intvardyncap;
  int psdmapdyncap;
  int psdvardyncap;

  long long int objfdyncap;
  long long int objadyncap;
  long long int fdyncap;
  long long int adyncap;
  long long int bdyncap;
  long long int hdyncap;
  long long int ddyncap;

} CBFdyndata;

CBFresponsee
CBFdyn_assign(CBFdyndata *dyndata, CBFdata *data);

CBFresponsee
CBFdyn_append(CBFdyndata *dyndata, CBFdata *data);

CBFresponsee
CBFdyn_freedynamicallocations(CBFdyndata *dyndata);

CBFresponsee
CBFdyn_map_capacitysurplus(CBFdyndata *dyndata, long long int surplus);

CBFresponsee
CBFdyn_map_adddomain(CBFdyndata *dyndata, CBFscalarconee domain, long long int dim);

CBFresponsee
CBFdyn_var_capacitysurplus(CBFdyndata *dyndata, long long int surplus);

CBFresponsee
CBFdyn_var_adddomain(CBFdyndata *dyndata, CBFscalarconee domain, long long int dim);

CBFresponsee
CBFdyn_intvar_capacitysurplus(CBFdyndata *dyndata, long long int surplus);

CBFresponsee
CBFdyn_intvar_add(CBFdyndata *dyndata, long long int idx);

CBFresponsee
CBFdyn_psdmap_capacitysurplus(CBFdyndata *dyndata, int surplus);

CBFresponsee
CBFdyn_psdmap_add(CBFdyndata *dyndata, int dim);

CBFresponsee
CBFdyn_psdvar_capacitysurplus(CBFdyndata *dyndata, int surplus);

CBFresponsee
CBFdyn_psdvar_add(CBFdyndata *dyndata, int dim);

CBFresponsee
CBFdyn_objf_capacitysurplus(CBFdyndata *dyndata, long long int surplus);

CBFresponsee
CBFdyn_objf_add(CBFdyndata *dyndata, int objfsubj, int objfsubk, int objfsubl, double objfval);

CBFresponsee
CBFdyn_obja_capacitysurplus(CBFdyndata *dyndata, long long int surplus);

CBFresponsee
CBFdyn_obja_add(CBFdyndata *dyndata, long long int objasubj, double objaval);

CBFresponsee
CBFdyn_objb_set(CBFdyndata *dyndata, double objbval);

CBFresponsee
CBFdyn_f_capacitysurplus(CBFdyndata *dyndata, long long int surplus);

CBFresponsee
CBFdyn_f_add(CBFdyndata *dyndata, long long int fsubi, int fsubj, int fsubk, int fsubl, double fval);

CBFresponsee
CBFdyn_a_capacitysurplus(CBFdyndata *dyndata, long long int surplus);

CBFresponsee
CBFdyn_a_add(CBFdyndata *dyndata, long long int asubi, long long int asubj, double aval);

CBFresponsee
CBFdyn_b_capacitysurplus(CBFdyndata *dyndata, long long int surplus);

CBFresponsee
CBFdyn_b_add(CBFdyndata *dyndata, long long int bsubi, double bval);

CBFresponsee
CBFdyn_h_capacitysurplus(CBFdyndata *dyndata, long long int surplus);

CBFresponsee
CBFdyn_h_add(CBFdyndata *dyndata, int hsubi, long long int hsubj, int hsubk, int hsubl, double hval);

CBFresponsee
CBFdyn_d_capacitysurplus(CBFdyndata *dyndata, long long int surplus);

CBFresponsee
CBFdyn_d_add(CBFdyndata *dyndata, int dsubi, int dsubk, int dsubl, double dval);

CBFresponsee
CBFdyn_varbound_capacitysurplus(CBFdyndata *dyndata, long long int surplus);

CBFresponsee
CBFdyn_varbound_addlower(CBFdyndata *dyndata, long long int idx, double val);

CBFresponsee
CBFdyn_varbound_addupper(CBFdyndata *dyndata, long long int idx, double val);

CBFresponsee
CBFdyn_varbound_addfix(CBFdyndata *dyndata, long long int idx, double val);

#endif
