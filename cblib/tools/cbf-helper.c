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

#include "cbf-helper.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
/*

 * ------------------------------------------------
 * Bucket and coordinate sort
 * ------------------------------------------------
 */

CBFresponsee CBF_bucketsort(long long int maxval, long long int nnz, const long long int *val, long long int *idx) {
  CBFresponsee res = CBF_RES_OK;
  long long int i, j, idxi, vali, k = 0;
  long long int *bucket = NULL;
  long long int *bucketpath = NULL;

  if (nnz == 0) {
    return CBF_RES_OK;
  } else {
    bucket = (long long int *) malloc((maxval + 1) * sizeof(bucket[0]));
    bucketpath = (long long int *) malloc(nnz * sizeof(bucketpath[0]));
  }

  if (!bucket || !bucketpath) {
    if (bucket)
      free(bucket);
    if (bucketpath)
      free(bucketpath);
    return CBF_RES_ERR;
  }

  if (res == CBF_RES_OK) {
    for (i = 0; i <= maxval; ++i)
      bucket[i] = -1;

    for (i = 0; i < nnz; ++i)
      bucketpath[i] = -1;
  }

  // Link elements with a common value, first pointed to by bucket[value]
  // (backwards to reverse the effect of LIFO structure and get a stable sort)
  for (i = nnz - 1; i >= 0 && res == CBF_RES_OK; --i) {
    idxi = idx[i];
    vali = val[idxi];

    bucketpath[idxi] = bucket[vali];
    bucket[vali] = idxi;
  }

  // Extract idx from buckets
  if (res == CBF_RES_OK) {
    for (i = 0; i <= maxval; ++i) {
      j = bucket[i];
      while (j != -1) {
        idx[k++] = j;
        j = bucketpath[j];
      }
    }
  }

  free(bucket);
  free(bucketpath);

  return res;
}

CBFresponsee CBF_bucketsort(long long int maxval, long long int nnz, const int *val, long long int *idx) {
  CBFresponsee res = CBF_RES_OK;
  long long int i, j, idxi, vali, k = 0;
  long long int *bucket = NULL;
  long long int *bucketpath = NULL;

  if (nnz == 0) {
    return CBF_RES_OK;
  } else {
    bucket = (long long int *) malloc((maxval + 1) * sizeof(bucket[0]));
    bucketpath = (long long int *) malloc(nnz * sizeof(bucketpath[0]));
  }

  if (!bucket || !bucketpath) {
    if (bucket)
      free(bucket);
    if (bucketpath)
      free(bucketpath);
    return CBF_RES_ERR;
  }

  if (res == CBF_RES_OK) {
    for (i = 0; i <= maxval; ++i)
      bucket[i] = -1;

    for (i = 0; i < nnz; ++i)
      bucketpath[i] = -1;
  }

  // Link elements with a common value, first pointed to by bucket[value]
  // (backwards to reverse the effect of LIFO structure and get a stable sort)
  for (i = nnz - 1; i >= 0 && res == CBF_RES_OK; --i) {
    idxi = idx[i];
    vali = val[idxi];

    bucketpath[idxi] = bucket[vali];
    bucket[vali] = idxi;
  }

  // Extract idx from buckets
  if (res == CBF_RES_OK) {
    for (i = 0; i <= maxval; ++i) {
      j = bucket[i];
      while (j != -1) {
        idx[k++] = j;
        j = bucketpath[j];
      }
    }
  }

  free(bucket);
  free(bucketpath);

  return res;
}

CBFresponsee CBF_coordinatesort(long long int *i, double *v, long long int nnz, long long int maxi) {
  CBFresponsee res = CBF_RES_OK;
  long long int idx, *sortidx = NULL;
  long long int *itmp = NULL;
  double *vtmp = NULL;

  if (nnz == 0) {
    return CBF_RES_OK;
  } else {
    sortidx = (long long int *) malloc(nnz * sizeof(sortidx[0]));
    itmp = (long long int *) malloc(nnz * sizeof(i[0]));
    vtmp = (double *) malloc(nnz * sizeof(v[0]));
  }

  if (sortidx && itmp && vtmp) {
    for (idx = 0; idx < nnz; ++idx) {
      sortidx[idx] = idx;
      itmp[idx] = i[idx];
      vtmp[idx] = v[idx];
    }

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(maxi, nnz, i, sortidx); // stable sort by i

    if (res == CBF_RES_OK) {
      for (idx = 0; idx < nnz; ++idx) {
        i[idx] = itmp[sortidx[idx]];
        v[idx] = vtmp[sortidx[idx]];
      }
    }

  } else {
    res = CBF_RES_ERR;
  }

  if (sortidx)
    free(sortidx);
  if (itmp)
    free(itmp);
  if (vtmp)
    free(vtmp);

  return res;
}

CBFresponsee CBF_coordinatesort(long long int *i, long long int *j, double *v, long long int nnz, long long int maxi, long long int maxj) {
  CBFresponsee res = CBF_RES_OK;
  long long int idx, *sortidx;
  long long int *itmp = NULL;
  long long int *jtmp = NULL;
  double *vtmp = NULL;

  if (nnz == 0) {
    return CBF_RES_OK;
  } else {
    sortidx = (long long int *) malloc(nnz * sizeof(sortidx[0]));
    itmp = (long long int *) malloc(nnz * sizeof(i[0]));
    jtmp = (long long int *) malloc(nnz * sizeof(j[0]));
    vtmp = (double *) malloc(nnz * sizeof(v[0]));
  }

  if (sortidx && itmp && jtmp && vtmp) {
    for (idx = 0; idx < nnz; ++idx) {
      sortidx[idx] = idx;
      itmp[idx] = i[idx];
      jtmp[idx] = j[idx];
      vtmp[idx] = v[idx];
    }

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(maxj, nnz, j, sortidx); // stable sort by j

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(maxi, nnz, i, sortidx); // stable sort by i

    if (res == CBF_RES_OK) {
      for (idx = 0; idx < nnz; ++idx) {
        i[idx] = itmp[sortidx[idx]];
        j[idx] = jtmp[sortidx[idx]];
        v[idx] = vtmp[sortidx[idx]];
      }
    }

  } else {
    res = CBF_RES_ERR;
  }

  if (sortidx)
    free(sortidx);
  if (itmp)
    free(itmp);
  if (jtmp)
    free(jtmp);
  if (vtmp)
    free(vtmp);

  return res;
}

CBFresponsee CBF_coordinatesort(int *i, int *j, int *k, double *v, long long int nnz, long long int maxi, long long int maxj, long long int maxk) {
  CBFresponsee res = CBF_RES_OK;
  long long int idx, *sortidx;
  int *itmp = NULL;
  int *jtmp = NULL;
  int *ktmp = NULL;
  double *vtmp = NULL;

  if (nnz == 0) {
    return CBF_RES_OK;
  } else {
    sortidx = (long long int *) malloc(nnz * sizeof(sortidx[0]));
    itmp = (int *) malloc(nnz * sizeof(i[0]));
    jtmp = (int *) malloc(nnz * sizeof(j[0]));
    ktmp = (int *) malloc(nnz * sizeof(k[0]));
    vtmp = (double *) malloc(nnz * sizeof(v[0]));
  }

  if (sortidx && itmp && jtmp && ktmp && vtmp) {
    for (idx = 0; idx < nnz; ++idx) {
      sortidx[idx] = idx;
      itmp[idx] = i[idx];
      jtmp[idx] = j[idx];
      ktmp[idx] = k[idx];
      vtmp[idx] = v[idx];
    }

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(maxk, nnz, k, sortidx); // stable sort by k

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(maxj, nnz, j, sortidx); // stable sort by j

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(maxi, nnz, i, sortidx); // stable sort by i

    if (res == CBF_RES_OK) {
      for (idx = 0; idx < nnz; ++idx) {
        i[idx] = itmp[sortidx[idx]];
        j[idx] = jtmp[sortidx[idx]];
        k[idx] = ktmp[sortidx[idx]];
        v[idx] = vtmp[sortidx[idx]];
      }
    }

  } else {
    res = CBF_RES_ERR;
  }

  if (sortidx)
    free(sortidx);
  if (itmp)
    free(itmp);
  if (jtmp)
    free(jtmp);
  if (ktmp)
    free(ktmp);
  if (vtmp)
    free(vtmp);

  return res;
}

CBFresponsee CBF_coordinatesort(long long int *i, int *j, int *k, int *l, double *v, long long int nnz, long long int maxi, long long int maxj,
    long long int maxk, long long int maxl) {
  CBFresponsee res = CBF_RES_OK;
  long long int idx, *sortidx;
  long long int *itmp = NULL;
  int *jtmp = NULL;
  int *ktmp = NULL;
  int *ltmp = NULL;
  double *vtmp = NULL;

  if (nnz == 0) {
    return CBF_RES_OK;
  } else {
    sortidx = (long long int *) malloc(nnz * sizeof(sortidx[0]));
    itmp = (long long int *) malloc(nnz * sizeof(i[0]));
    jtmp = (int *) malloc(nnz * sizeof(j[0]));
    ktmp = (int *) malloc(nnz * sizeof(k[0]));
    ltmp = (int *) malloc(nnz * sizeof(l[0]));
    vtmp = (double *) malloc(nnz * sizeof(v[0]));
  }

  if (sortidx && itmp && jtmp && ktmp && ltmp && vtmp) {
    for (idx = 0; idx < nnz; ++idx) {
      sortidx[idx] = idx;
      itmp[idx] = i[idx];
      jtmp[idx] = j[idx];
      ktmp[idx] = k[idx];
      ltmp[idx] = l[idx];
      vtmp[idx] = v[idx];
    }

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(maxl, nnz, l, sortidx); // stable sort by l

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(maxk, nnz, k, sortidx); // stable sort by k

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(maxj, nnz, j, sortidx); // stable sort by j

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(maxi, nnz, i, sortidx); // stable sort by i

    if (res == CBF_RES_OK) {
      for (idx = 0; idx < nnz; ++idx) {
        i[idx] = itmp[sortidx[idx]];
        j[idx] = jtmp[sortidx[idx]];
        k[idx] = ktmp[sortidx[idx]];
        l[idx] = ltmp[sortidx[idx]];
        v[idx] = vtmp[sortidx[idx]];
      }
    }

  } else {
    res = CBF_RES_ERR;
  }

  if (sortidx)
    free(sortidx);
  if (itmp)
    free(itmp);
  if (jtmp)
    free(jtmp);
  if (ktmp)
    free(ktmp);
  if (ltmp)
    free(ltmp);
  if (vtmp)
    free(vtmp);

  return res;
}

CBFresponsee CBF_coordinatesort(int *i, long long int *j, int *k, int *l, double *v, long long int nnz, long long int maxi, long long int maxj,
    long long int maxk, long long int maxl) {
  CBFresponsee res = CBF_RES_OK;
  long long int idx, *sortidx;
  int *itmp = NULL;
  long long int *jtmp = NULL;
  int *ktmp = NULL;
  int *ltmp = NULL;
  double *vtmp = NULL;

  if (nnz == 0) {
    return CBF_RES_OK;
  } else {
    sortidx = (long long int *) malloc(nnz * sizeof(sortidx[0]));
    itmp = (int *) malloc(nnz * sizeof(i[0]));
    jtmp = (long long int *) malloc(nnz * sizeof(j[0]));
    ktmp = (int *) malloc(nnz * sizeof(k[0]));
    ltmp = (int *) malloc(nnz * sizeof(l[0]));
    vtmp = (double *) malloc(nnz * sizeof(v[0]));
  }

  if (sortidx && itmp && jtmp && ktmp && ltmp && vtmp) {
    for (idx = 0; idx < nnz; ++idx) {
      sortidx[idx] = idx;
      itmp[idx] = i[idx];
      jtmp[idx] = j[idx];
      ktmp[idx] = k[idx];
      ltmp[idx] = l[idx];
      vtmp[idx] = v[idx];
    }

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(maxl, nnz, l, sortidx); // stable sort by l

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(maxk, nnz, k, sortidx); // stable sort by k

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(maxj, nnz, j, sortidx); // stable sort by j

    if (res == CBF_RES_OK)
      res = CBF_bucketsort(maxi, nnz, i, sortidx); // stable sort by i

    if (res == CBF_RES_OK) {
      for (idx = 0; idx < nnz; ++idx) {
        i[idx] = itmp[sortidx[idx]];
        j[idx] = jtmp[sortidx[idx]];
        k[idx] = ktmp[sortidx[idx]];
        l[idx] = ltmp[sortidx[idx]];
        v[idx] = vtmp[sortidx[idx]];
      }
    }

  } else {
    res = CBF_RES_ERR;
  }

  if (sortidx)
    free(sortidx);
  if (itmp)
    free(itmp);
  if (jtmp)
    free(jtmp);
  if (ktmp)
    free(ktmp);
  if (ltmp)
    free(ltmp);
  if (vtmp)
    free(vtmp);

  return res;
}

CBFresponsee CBF_coordinatesort_rowmajor_map(CBFdata *data) {

  CBFresponsee res = CBF_RES_OK;
  long long int i, maxpsdvardim = 0;

  for (i = 0; i < data->psdvarnum; ++i)
    if (data->psdvardim[i] > maxpsdvardim)
      maxpsdvardim = data->psdvardim[i];

  if (res == CBF_RES_OK)
    res = CBF_coordinatesort(data->fsubi, data->fsubj, data->fsubk, data->fsubl, data->fval, data->fnnz, data->mapnum, data->psdvarnum, maxpsdvardim,
        maxpsdvardim);

  if (res == CBF_RES_OK)
    res = CBF_coordinatesort(data->asubi, data->asubj, data->aval, data->annz, data->mapnum, data->varnum);

  if (res == CBF_RES_OK)
    res = CBF_coordinatesort(data->bsubi, data->bval, data->bnnz, data->mapnum);

  return res;
}

CBFresponsee CBF_coordinatesort_rowmajor_psdmap(CBFdata *data) {

  CBFresponsee res = CBF_RES_OK;
  long long int i, maxpsdmapdim = 0;

  for (i = 0; i < data->psdmapnum; ++i)
    if (data->psdmapdim[i] > maxpsdmapdim)
      maxpsdmapdim = data->psdmapdim[i];

  if (res == CBF_RES_OK)
    res = CBF_coordinatesort(data->hsubi, data->hsubj, data->hsubk, data->hsubl, data->hval, data->hnnz, data->psdmapnum, data->varnum, maxpsdmapdim,
        maxpsdmapdim);

  if (res == CBF_RES_OK)
    res = CBF_coordinatesort(data->dsubi, data->dsubk, data->dsubl, data->dval, data->dnnz, data->psdmapnum, maxpsdmapdim, maxpsdmapdim);

  return res;
}

/*
 * ------------------------------------------------
 * Find nnz's of map and psdmap
 * ------------------------------------------------
 */

CBFresponsee CBF_findforward_map(const CBFdata *data, long long int map, long long int *fbeg, long long int *abeg, long long int *bbeg) {

  // FCOORD
  if (fbeg)
    for (; (*fbeg < data->fnnz) && (data->fsubi[*fbeg] < map); ++*fbeg)
      continue;

  // ACOORD
  if (abeg)
    for (; (*abeg < data->annz) && (data->asubi[*abeg] < map); ++*abeg)
      continue;

  // BCOORD
  if (bbeg)
    for (; (*bbeg < data->bnnz) && (data->bsubi[*bbeg] < map); ++*bbeg)
      continue;

  return CBF_RES_OK;
}

CBFresponsee CBF_findforward_psdmap(const CBFdata *data, long long int psdmap, long long int *hbeg, long long int *dbeg) {

  // HCOORD
  if (hbeg)
    for (; (*hbeg < data->hnnz) && (data->hsubi[*hbeg] < psdmap); ++*hbeg)
      continue;

  // DCOORD
  if (dbeg)
    for (; (*dbeg < data->dnnz) && (data->dsubi[*dbeg] < psdmap); ++*dbeg)
      continue;

  return CBF_RES_OK;
}

CBFresponsee CBF_findbackward_map(const CBFdata *data, long long int map, long long int *fend, long long int *aend, long long int *bend) {

  // FCOORD
  if (fend)
    for (; (*fend >= 0) && (data->fsubi[*fend] > map); --*fend)
      continue;

  // ACOORD
  if (aend)
    for (; (*aend >= 0) && (data->asubi[*aend] > map); --*aend)
      continue;

  // BCOORD
  if (bend)
    for (; (*bend >= 0) && (data->bsubi[*bend] > map); --*bend)
      continue;

  return CBF_RES_OK;
}

CBFresponsee CBF_findbackward_psdmap(const CBFdata *data, long long int psdmap, long long int *hend, long long int *dend) {

  // HCOORD
  if (hend)
    for (; (*hend >= 0) && (data->hsubi[*hend] > psdmap); --*hend)
      continue;

  // DCOORD
  if (dend)
    for (; (*dend >= 0) && (data->dsubi[*dend] > psdmap); --*dend)
      continue;

  return CBF_RES_OK;
}

/*
 * ------------------------------------------------
 * Remove empty nnz and deleted maps
 * ------------------------------------------------
 */

CBFresponsee CBF_compress_maps(CBFdata *data, const char *delmap) {

  CBFresponsee res = CBF_RES_OK;
  long long int k, r, rbeg, fbeg, abeg, bbeg;
  long long int mapstacknum, mapstackdim, mapnum, fnnz, annz, bnnz;

  // Sort coordinates
  res = CBF_coordinatesort_rowmajor_map(data);

  // Rewrite coordinates
  rbeg = bbeg = abeg = fbeg = 0;
  mapstacknum = mapnum = fnnz = annz = bnnz = 0;
  for (k = 0; k < data->mapstacknum && res == CBF_RES_OK; ++k) {
    mapstackdim = 0;

    for (r = rbeg; r < rbeg + data->mapstackdim[k] && res == CBF_RES_OK; ++r) {
      if (!delmap || delmap[r] != 1) {

        // FCOORD
        while ((fbeg < data->fnnz) && (data->fsubi[fbeg] == r)) {
          if (data->fval[fbeg] != 0.0) {
            data->fsubi[fnnz] = mapnum;
            data->fsubj[fnnz] = data->fsubj[fbeg];
            data->fsubk[fnnz] = data->fsubk[fbeg];
            data->fsubl[fnnz] = data->fsubl[fbeg];
            data->fval[fnnz] = data->fval[fbeg];
            ++fnnz;
          }
          ++fbeg;
        }

        // ACOORD
        while ((abeg < data->annz) && (data->asubi[abeg] == r)) {
          if (data->aval[abeg] != 0.0) {
            data->asubi[annz] = mapnum;
            data->asubj[annz] = data->asubj[abeg];
            data->aval[annz] = data->aval[abeg];
            ++annz;
          }
          ++abeg;
        }

        // BCOORD
        while ((bbeg < data->bnnz) && (data->bsubi[bbeg] == r)) {
          if (data->bval[bbeg] != 0.0) {
            data->bsubi[bnnz] = mapnum;
            data->bval[bnnz] = data->bval[bbeg];
            ++bnnz;
          }
          ++bbeg;
        }

        ++mapnum;
        ++mapstackdim;

      } else {
        res = CBF_findforward_map(data, r + 1, &fbeg, &abeg, &bbeg);
      }
    }

    if (mapstackdim) {
      data->mapstackdomain[mapstacknum] = data->mapstackdomain[k];
      data->mapstackdim[mapstacknum] = mapstackdim;
      ++mapstacknum;
    }

    rbeg = r;
  }

  data->fnnz = fnnz;
  data->annz = annz;
  data->bnnz = bnnz;
  data->mapnum = mapnum;
  data->mapstacknum = mapstacknum;

  return res;
}

CBFresponsee CBF_compress_psdmaps(CBFdata *data, const char *delpsdmap) {
  CBFresponsee res = CBF_RES_OK;
  long long int r, row, hbeg, dbeg;
  long long int hnnz = 0, dnnz = 0, psdmapnum = 0;

  // Sort coordinates
  res = CBF_coordinatesort_rowmajor_psdmap(data);

  // Process coordinates
  r = row = hbeg = dbeg = 0;
  while (r < data->psdmapnum && res == CBF_RES_OK) {
    if (delpsdmap[r] != 1) {
      ++psdmapnum;

      // HCOORD
      while ((hbeg < data->hnnz) && (data->hsubi[hbeg] == r)) {
        if (data->hval[hbeg] != 0.0) {
          data->hsubi[hnnz] = r - row;
          data->hsubj[hnnz] = data->hsubj[hbeg];
          data->hsubk[hnnz] = data->hsubk[hbeg];
          data->hsubl[hnnz] = data->hsubl[hbeg];
          data->hval[hnnz] = data->hval[hbeg];
          ++hnnz;
        }
        ++hbeg;
      }

      // DCOORD
      while ((dbeg < data->dnnz) && (data->dsubi[dbeg] == r)) {
        if (data->dval[dbeg] != 0.0) {
          data->dsubi[dnnz] = r - row;
          data->dsubk[dnnz] = data->dsubk[dbeg];
          data->dsubl[dnnz] = data->dsubl[dbeg];
          data->dval[dnnz] = data->dval[dbeg];
          ++dnnz;
        }
        ++dbeg;
      }

    } else {
      ++row;
      res = CBF_findforward_psdmap(data, r + 1, &hbeg, &dbeg);
    }
    ++r;
  }

  data->hnnz = hnnz;
  data->dnnz = dnnz;
  data->psdmapnum = psdmapnum;

  return res;
}

/*
 * ------------------------------------------------
 * Integer array
 * ------------------------------------------------
 */

CBFresponsee CBFintegerarray_init(CBFdata *data, char **integerarray) {
  long long int i;

  *integerarray = (char*) calloc(data->varnum, sizeof(char));

  if (!*integerarray)
    return CBF_RES_ERR;

  for (i = 0; i < data->intvarnum; ++i)
    (*integerarray)[data->intvar[i]] = 1;

  return CBF_RES_OK;
}

void CBFintegerarray_free(char **integerarray) {
  if (*integerarray) {
    free(*integerarray);
    *integerarray = NULL;
  }
}

/*
 * ------------------------------------------------
 * Dynamic allocation of CBFdata
 * ------------------------------------------------
 */

CBFresponsee CBFdyn_assign(CBFdyndata *dyndata, CBFdata *data) {
  dyndata->data = data;

  dyndata->mapstackdyncap = data->mapstacknum;
  dyndata->varstackdyncap = data->varstacknum;
  dyndata->intvardyncap = data->intvarnum;
  dyndata->psdmapdyncap = data->psdmapnum;
  dyndata->psdvardyncap = data->psdvarnum;

  dyndata->objfdyncap = data->objfnnz;
  dyndata->objadyncap = data->objannz;
  dyndata->fdyncap = data->fnnz;
  dyndata->adyncap = data->annz;
  dyndata->bdyncap = data->bnnz;
  dyndata->hdyncap = data->hnnz;
  dyndata->ddyncap = data->dnnz;

  return CBF_RES_OK;
}

CBFresponsee CBFdyn_append(CBFdyndata *dyndata, CBFdata *data) {
  CBFresponsee res = CBF_RES_OK;
  long long int i;

  if (res == CBF_RES_OK)
    if (data->mapstacknum >= 1) {
      res = CBFdyn_map_capacitysurplus(dyndata, data->mapstacknum);
      for (i = 0; i < data->mapstacknum && res == CBF_RES_OK; ++i) {
        res = CBFdyn_map_adddomain(dyndata, data->mapstackdomain[i], data->mapstackdim[i]);
      }
    }

  if (res == CBF_RES_OK)
    if (data->varstacknum >= 1) {
      res = CBFdyn_var_capacitysurplus(dyndata, data->varstacknum);
      for (i = 0; i < data->varstacknum && res == CBF_RES_OK; ++i) {
        res = CBFdyn_var_adddomain(dyndata, data->varstackdomain[i], data->varstackdim[i]);
      }
    }

  if (res == CBF_RES_OK)
    if (data->intvarnum >= 1) {
      res = CBFdyn_intvar_capacitysurplus(dyndata, data->intvarnum);
      for (i = 0; i < data->intvarnum && res == CBF_RES_OK; ++i) {
        res = CBFdyn_intvar_add(dyndata, data->intvar[i]);
      }
    }

  if (res == CBF_RES_OK)
    if (data->psdmapnum >= 1) {
      res = CBFdyn_psdmap_capacitysurplus(dyndata, data->psdmapnum);
      for (i = 0; i < data->psdmapnum && res == CBF_RES_OK; ++i) {
        res = CBFdyn_psdmap_add(dyndata, data->psdmapdim[i]);
      }
    }

  if (res == CBF_RES_OK)
    if (data->psdvarnum >= 1) {
      res = CBFdyn_psdvar_capacitysurplus(dyndata, data->psdvarnum);
      for (i = 0; i < data->psdvarnum && res == CBF_RES_OK; ++i) {
        res = CBFdyn_psdvar_add(dyndata, data->psdvardim[i]);
      }
    }

  if (res == CBF_RES_OK)
    if (data->objfnnz >= 1) {
      res = CBFdyn_objf_capacitysurplus(dyndata, data->objfnnz);
      for (i = 0; i < data->objfnnz && res == CBF_RES_OK; ++i) {
        res = CBFdyn_objf_add(dyndata, data->objfsubj[i], data->objfsubk[i], data->objfsubl[i], data->objfval[i]);
      }
    }

  if (res == CBF_RES_OK)
    if (data->objannz >= 1) {
      res = CBFdyn_obja_capacitysurplus(dyndata, data->objannz);
      for (i = 0; i < data->objannz && res == CBF_RES_OK; ++i) {
        res = CBFdyn_obja_add(dyndata, data->objasubj[i], data->objaval[i]);
      }
    }

  if (res == CBF_RES_OK)
    if (data->fnnz >= 1) {
      res = CBFdyn_f_capacitysurplus(dyndata, data->fnnz);
      for (i = 0; i < data->fnnz && res == CBF_RES_OK; ++i) {
        res = CBFdyn_f_add(dyndata, data->fsubi[i], data->fsubj[i], data->fsubk[i], data->fsubl[i], data->fval[i]);
      }
    }

  if (res == CBF_RES_OK)
    if (data->annz >= 1) {
      res = CBFdyn_a_capacitysurplus(dyndata, data->annz);
      for (i = 0; i < data->annz && res == CBF_RES_OK; ++i) {
        res = CBFdyn_a_add(dyndata, data->asubi[i], data->asubj[i], data->aval[i]);
      }
    }

  if (res == CBF_RES_OK)
    if (data->bnnz >= 1) {
      res = CBFdyn_b_capacitysurplus(dyndata, data->bnnz);
      for (i = 0; i < data->bnnz && res == CBF_RES_OK; ++i) {
        res = CBFdyn_b_add(dyndata, data->bsubi[i], data->bval[i]);
      }
    }

  if (res == CBF_RES_OK)
    if (data->hnnz >= 1) {
      res = CBFdyn_h_capacitysurplus(dyndata, data->hnnz);
      for (i = 0; i < data->hnnz && res == CBF_RES_OK; ++i) {
        res = CBFdyn_h_add(dyndata, data->hsubi[i], data->hsubj[i], data->hsubk[i], data->hsubl[i], data->hval[i]);
      }
    }

  if (res == CBF_RES_OK)
    if (data->dnnz >= 1) {
      res = CBFdyn_d_capacitysurplus(dyndata, data->dnnz);
      for (i = 0; i < data->dnnz && res == CBF_RES_OK; ++i) {
        res = CBFdyn_d_add(dyndata, data->dsubi[i], data->dsubk[i], data->dsubl[i], data->dval[i]);
      }
    }

  return res;
}

CBFresponsee CBFdyn_freedynamicallocations(CBFdyndata *dyndata) {
  if (dyndata->mapstackdyncap >= 1) {
    free(dyndata->data->mapstackdim);
    free(dyndata->data->mapstackdomain);
    dyndata->data->mapstacknum = 0;
  }

  if (dyndata->varstackdyncap >= 1) {
    free(dyndata->data->varstackdim);
    free(dyndata->data->varstackdomain);
    dyndata->data->varstacknum = 0;
  }

  if (dyndata->intvardyncap >= 1) {
    free(dyndata->data->intvar);
    dyndata->data->intvarnum = 0;
  }

  if (dyndata->psdmapdyncap >= 1) {
    free(dyndata->data->psdmapdim);
    dyndata->data->psdmapnum = 0;
  }

  if (dyndata->psdvardyncap >= 1) {
    free(dyndata->data->psdvardim);
    dyndata->data->psdvarnum = 0;
  }

  if (dyndata->objfdyncap >= 1) {
    free(dyndata->data->objfsubj);
    free(dyndata->data->objfsubk);
    free(dyndata->data->objfsubl);
    free(dyndata->data->objfval);
    dyndata->data->objfnnz = 0;
  }

  if (dyndata->objadyncap >= 1) {
    free(dyndata->data->objasubj);
    free(dyndata->data->objaval);
    dyndata->data->objannz = 0;
  }

  dyndata->data->objbval = 0;

  if (dyndata->fdyncap >= 1) {
    free(dyndata->data->fsubi);
    free(dyndata->data->fsubj);
    free(dyndata->data->fsubk);
    free(dyndata->data->fsubl);
    free(dyndata->data->fval);
    dyndata->data->fnnz = 0;
  }

  if (dyndata->adyncap >= 1) {
    free(dyndata->data->asubi);
    free(dyndata->data->asubj);
    free(dyndata->data->aval);
    dyndata->data->annz = 0;
  }

  if (dyndata->bdyncap >= 1) {
    free(dyndata->data->bsubi);
    free(dyndata->data->bval);
    dyndata->data->bnnz = 0;
  }

  if (dyndata->hdyncap >= 1) {
    free(dyndata->data->hsubi);
    free(dyndata->data->hsubj);
    free(dyndata->data->hsubk);
    free(dyndata->data->hsubl);
    free(dyndata->data->hval);
    dyndata->data->hnnz = 0;
  }

  if (dyndata->ddyncap >= 1) {
    free(dyndata->data->dsubi);
    free(dyndata->data->dsubk);
    free(dyndata->data->dsubl);
    free(dyndata->data->dval);
    dyndata->data->dnnz = 0;
  }

  return CBF_RES_OK;
}

CBFresponsee CBFdyn_map_capacitysurplus(CBFdyndata *dyndata, long long int surplus) {
  long long int size;
  long long int *buf1;
  CBFscalarconee *buf2;

  if (dyndata->data->mapstacknum > dyndata->mapstackdyncap)
    return CBF_RES_ERR;

  size = dyndata->data->mapstacknum + surplus;

  if (size > dyndata->mapstackdyncap) {
    buf1 = (long long int*) realloc(dyndata->data->mapstackdim, size * sizeof(dyndata->data->mapstackdim[0]));
    buf2 = (CBFscalarconee*) realloc(dyndata->data->mapstackdomain, size * sizeof(dyndata->data->mapstackdomain[0]));

    if (buf1 && buf2) {
      dyndata->data->mapstackdim = buf1;
      dyndata->data->mapstackdomain = buf2;
      dyndata->mapstackdyncap = size;
    } else {
      return CBF_RES_ERR;
    }
  }
  return CBF_RES_OK;
}

CBFresponsee CBFdyn_map_adddomain(CBFdyndata *dyndata, CBFscalarconee domain, long long int dim) {
  if (domain == CBF_CONE_END)
    return CBF_RES_ERR;

  if (dyndata->data->mapstacknum + 1 > dyndata->mapstackdyncap)
    return CBF_RES_ERR;

  // Always try to skip the creation of a new domains
  if ((dyndata->data->mapstacknum == 0) || (domain != dyndata->data->mapstackdomain[dyndata->data->mapstacknum - 1])
      || (domain != CBF_CONE_FREE && domain != CBF_CONE_POS && domain != CBF_CONE_NEG && domain != CBF_CONE_ZERO)) {
    // Create new domain
    ++dyndata->data->mapstacknum;
    dyndata->data->mapstackdim[dyndata->data->mapstacknum - 1] = 0;
    dyndata->data->mapstackdomain[dyndata->data->mapstacknum - 1] = domain;

  }
  // Increase dimension of current domain
  dyndata->data->mapstackdim[dyndata->data->mapstacknum - 1] += dim;
  dyndata->data->mapnum += dim;

  return CBF_RES_OK;
}

CBFresponsee CBFdyn_var_capacitysurplus(CBFdyndata *dyndata, long long int surplus) {
  long long int size;
  long long int *buf1;
  CBFscalarconee *buf2;

  if (dyndata->data->varstacknum > dyndata->varstackdyncap)
    return CBF_RES_ERR;

  size = dyndata->data->varstacknum + surplus;

  if (size > dyndata->varstackdyncap) {
    buf1 = (long long int*) realloc(dyndata->data->varstackdim, size * sizeof(dyndata->data->varstackdim[0]));
    buf2 = (CBFscalarconee*) realloc(dyndata->data->varstackdomain, size * sizeof(dyndata->data->varstackdomain[0]));

    if (buf1 && buf2) {
      dyndata->data->varstackdim = buf1;
      dyndata->data->varstackdomain = buf2;
      dyndata->varstackdyncap = size;
    } else {
      return CBF_RES_ERR;
    }
  }
  return CBF_RES_OK;
}

CBFresponsee CBFdyn_var_adddomain(CBFdyndata *dyndata, CBFscalarconee domain, long long int dim) {
  if (domain == CBF_CONE_END)
    return CBF_RES_ERR;

  if (dyndata->data->varstacknum + 1 > dyndata->varstackdyncap)
    return CBF_RES_ERR;

  // Always try to skip the creation of a new domains
  if ((dyndata->data->varstacknum == 0) || (domain != dyndata->data->varstackdomain[dyndata->data->varstacknum - 1])
      || (domain != CBF_CONE_FREE && domain != CBF_CONE_POS && domain != CBF_CONE_NEG && domain != CBF_CONE_ZERO)) {
    // Create new domain
    ++dyndata->data->varstacknum;
    dyndata->data->varstackdim[dyndata->data->varstacknum - 1] = 0;
    dyndata->data->varstackdomain[dyndata->data->varstacknum - 1] = domain;

  }
  // Increase dimension of current domain
  dyndata->data->varstackdim[dyndata->data->varstacknum - 1] += dim;
  dyndata->data->varnum += dim;

  return CBF_RES_OK;
}

CBFresponsee CBFdyn_intvar_capacitysurplus(CBFdyndata *dyndata, long long int surplus) {
  long long int size;
  long long int *buf1;

  if (dyndata->data->intvarnum > dyndata->intvardyncap)
    return CBF_RES_ERR;

  size = dyndata->data->intvarnum + surplus;

  if (size > dyndata->intvardyncap) {
    buf1 = (long long int*) realloc(dyndata->data->intvar, size * sizeof(dyndata->data->intvar[0]));

    if (buf1) {
      dyndata->data->intvar = buf1;
      dyndata->intvardyncap = size;
    } else {
      return CBF_RES_ERR;
    }
  }
  return CBF_RES_OK;
}

CBFresponsee CBFdyn_intvar_add(CBFdyndata *dyndata, long long int idx) {
  if (dyndata->data->intvarnum + 1 > dyndata->intvardyncap)
    return CBF_RES_ERR;

  ++dyndata->data->intvarnum;
  dyndata->data->intvar[dyndata->data->intvarnum - 1] = idx;

  return CBF_RES_OK;
}

CBFresponsee CBFdyn_psdmap_capacitysurplus(CBFdyndata *dyndata, int surplus) {
  long long int size;
  int *buf1;

  if (dyndata->data->psdmapnum > dyndata->psdmapdyncap)
    return CBF_RES_ERR;

  size = dyndata->data->psdmapnum + surplus;

  if (size > dyndata->psdmapdyncap) {
    buf1 = (int*) realloc(dyndata->data->psdmapdim, size * sizeof(dyndata->data->psdmapdim[0]));

    if (buf1) {
      dyndata->data->psdmapdim = buf1;
      dyndata->psdmapdyncap = size;
    } else {
      return CBF_RES_ERR;
    }
  }
  return CBF_RES_OK;
}

CBFresponsee CBFdyn_psdmap_add(CBFdyndata *dyndata, int dim) {
  if (dyndata->data->psdmapnum + 1 > dyndata->psdmapdyncap)
    return CBF_RES_ERR;

  ++dyndata->data->psdmapnum;
  dyndata->data->psdmapdim[dyndata->data->psdmapnum - 1] = dim;

  return CBF_RES_OK;
}

CBFresponsee CBFdyn_psdvar_capacitysurplus(CBFdyndata *dyndata, int surplus) {
  long long int size;
  int *buf1;

  if (dyndata->data->psdvarnum > dyndata->psdvardyncap)
    return CBF_RES_ERR;

  size = dyndata->data->psdvarnum + surplus;

  if (size > dyndata->psdvardyncap) {
    buf1 = (int*) realloc(dyndata->data->psdvardim, size * sizeof(dyndata->data->psdvardim[0]));

    if (buf1) {
      dyndata->data->psdvardim = buf1;
      dyndata->psdvardyncap = size;
    } else {
      return CBF_RES_ERR;
    }
  }
  return CBF_RES_OK;
}

CBFresponsee CBFdyn_psdvar_add(CBFdyndata *dyndata, int dim) {
  if (dyndata->data->psdvarnum + 1 > dyndata->psdvardyncap)
    return CBF_RES_ERR;

  ++dyndata->data->psdvarnum;
  dyndata->data->psdvardim[dyndata->data->psdvarnum - 1] = dim;

  return CBF_RES_OK;
}

CBFresponsee CBFdyn_objf_capacitysurplus(CBFdyndata *dyndata, long long int surplus) {
  long long int size;
  int *buf1, *buf2, *buf3;
  double *buf4;

  if (dyndata->data->objfnnz > dyndata->objfdyncap)
    return CBF_RES_ERR;

  size = dyndata->data->objfnnz + surplus;

  if (size > dyndata->objfdyncap) {
    buf1 = (int*) realloc(dyndata->data->objfsubj, size * sizeof(dyndata->data->objfsubj[0]));
    buf2 = (int*) realloc(dyndata->data->objfsubk, size * sizeof(dyndata->data->objfsubk[0]));
    buf3 = (int*) realloc(dyndata->data->objfsubl, size * sizeof(dyndata->data->objfsubl[0]));
    buf4 = (double*) realloc(dyndata->data->objfval, size * sizeof(dyndata->data->objfval[0]));

    if (buf1 && buf2 && buf3 && buf4) {
      dyndata->data->objfsubj = buf1;
      dyndata->data->objfsubk = buf2;
      dyndata->data->objfsubl = buf3;
      dyndata->data->objfval = buf4;
      dyndata->objfdyncap = size;
    } else {
      return CBF_RES_ERR;
    }
  }
  return CBF_RES_OK;
}

CBFresponsee CBFdyn_objf_add(CBFdyndata *dyndata, int objfsubj, int objfsubk, int objfsubl, double objfval) {
  if (dyndata->data->objfnnz + 1 > dyndata->objfdyncap)
    return CBF_RES_ERR;

  ++dyndata->data->objfnnz;
  dyndata->data->objfsubj[dyndata->data->objfnnz - 1] = objfsubj;
  dyndata->data->objfsubk[dyndata->data->objfnnz - 1] = objfsubk;
  dyndata->data->objfsubl[dyndata->data->objfnnz - 1] = objfsubl;
  dyndata->data->objfval[dyndata->data->objfnnz - 1] = objfval;

  return CBF_RES_OK;
}

CBFresponsee CBFdyn_obja_capacitysurplus(CBFdyndata *dyndata, long long int surplus) {
  long long int size;
  long long int *buf1;
  double *buf2;

  if (dyndata->data->objannz > dyndata->objadyncap)
    return CBF_RES_ERR;

  size = dyndata->data->objannz + surplus;

  if (size > dyndata->objadyncap) {
    buf1 = (long long int*) realloc(dyndata->data->objasubj, size * sizeof(dyndata->data->objasubj[0]));
    buf2 = (double*) realloc(dyndata->data->objaval, size * sizeof(dyndata->data->objaval[0]));

    if (buf1 && buf2) {
      dyndata->data->objasubj = buf1;
      dyndata->data->objaval = buf2;
      dyndata->objadyncap = size;
    } else {
      return CBF_RES_ERR;
    }
  }
  return CBF_RES_OK;
}

CBFresponsee CBFdyn_obja_add(CBFdyndata *dyndata, long long int objasubj, double objaval) {
  if (dyndata->data->objannz + 1 > dyndata->objadyncap)
    return CBF_RES_ERR;

  ++dyndata->data->objannz;
  dyndata->data->objasubj[dyndata->data->objannz - 1] = objasubj;
  dyndata->data->objaval[dyndata->data->objannz - 1] = objaval;

  return CBF_RES_OK;
}

CBFresponsee CBFdyn_objb_set(CBFdyndata *dyndata, double objbval) {
  dyndata->data->objbval = objbval;
  return CBF_RES_OK;
}

CBFresponsee CBFdyn_f_capacitysurplus(CBFdyndata *dyndata, long long int surplus) {
  long long int size;
  long long int *buf1;
  int *buf2, *buf3, *buf4;
  double *buf5;

  if (dyndata->data->fnnz > dyndata->fdyncap)
    return CBF_RES_ERR;

  size = dyndata->data->fnnz + surplus;

  if (size > dyndata->fdyncap) {
    buf1 = (long long int*) realloc(dyndata->data->fsubi, size * sizeof(dyndata->data->fsubi[0]));
    buf2 = (int*) realloc(dyndata->data->fsubj, size * sizeof(dyndata->data->fsubj[0]));
    buf3 = (int*) realloc(dyndata->data->fsubk, size * sizeof(dyndata->data->fsubk[0]));
    buf4 = (int*) realloc(dyndata->data->fsubl, size * sizeof(dyndata->data->fsubl[0]));
    buf5 = (double*) realloc(dyndata->data->fval, size * sizeof(dyndata->data->fval[0]));

    if (buf1 && buf2 && buf3 && buf4 && buf5) {
      dyndata->data->fsubi = buf1;
      dyndata->data->fsubj = buf2;
      dyndata->data->fsubk = buf3;
      dyndata->data->fsubl = buf4;
      dyndata->data->fval = buf5;
      dyndata->fdyncap = size;
    } else {
      return CBF_RES_ERR;
    }
  }
  return CBF_RES_OK;
}

CBFresponsee CBFdyn_f_add(CBFdyndata *dyndata, long long int fsubi, int fsubj, int fsubk, int fsubl, double fval) {
  if (dyndata->data->fnnz + 1 > dyndata->fdyncap)
    return CBF_RES_ERR;

  ++dyndata->data->fnnz;
  dyndata->data->fsubi[dyndata->data->fnnz - 1] = fsubi;
  dyndata->data->fsubj[dyndata->data->fnnz - 1] = fsubj;
  dyndata->data->fsubk[dyndata->data->fnnz - 1] = fsubk;
  dyndata->data->fsubl[dyndata->data->fnnz - 1] = fsubl;
  dyndata->data->fval[dyndata->data->fnnz - 1] = fval;

  return CBF_RES_OK;
}

CBFresponsee CBFdyn_a_capacitysurplus(CBFdyndata *dyndata, long long int surplus) {
  long long int size;
  long long int *buf1, *buf2;
  double *buf3;

  if (dyndata->data->annz > dyndata->adyncap)
    return CBF_RES_ERR;

  size = dyndata->data->annz + surplus;

  if (size > dyndata->adyncap) {
    buf1 = (long long int *) realloc(dyndata->data->asubi, size * sizeof(dyndata->data->asubi[0]));
    buf2 = (long long int *) realloc(dyndata->data->asubj, size * sizeof(dyndata->data->asubj[0]));
    buf3 = (double *) realloc(dyndata->data->aval, size * sizeof(dyndata->data->aval[0]));

    if (buf1 && buf2 && buf3) {
      dyndata->data->asubi = buf1;
      dyndata->data->asubj = buf2;
      dyndata->data->aval = buf3;
      dyndata->adyncap = size;
    } else {
      return CBF_RES_ERR;
    }
  }
  return CBF_RES_OK;
}

CBFresponsee CBFdyn_a_add(CBFdyndata *dyndata, long long int asubi, long long int asubj, double aval) {
  if (dyndata->data->annz + 1 > dyndata->adyncap)
    return CBF_RES_ERR;

  ++dyndata->data->annz;
  dyndata->data->asubi[dyndata->data->annz - 1] = asubi;
  dyndata->data->asubj[dyndata->data->annz - 1] = asubj;
  dyndata->data->aval[dyndata->data->annz - 1] = aval;

  return CBF_RES_OK;
}

CBFresponsee CBFdyn_b_capacitysurplus(CBFdyndata *dyndata, long long int surplus) {
  long long int size;
  long long int *buf1;
  double *buf2;

  if (dyndata->data->bnnz > dyndata->bdyncap)
    return CBF_RES_ERR;

  size = dyndata->data->bnnz + surplus;

  if (size > dyndata->bdyncap) {
    buf1 = (long long int*) realloc(dyndata->data->bsubi, size * sizeof(dyndata->data->bsubi[0]));
    buf2 = (double*) realloc(dyndata->data->bval, size * sizeof(dyndata->data->bval[0]));

    if (buf1 && buf2) {
      dyndata->data->bsubi = buf1;
      dyndata->data->bval = buf2;
      dyndata->bdyncap = size;
    } else {
      return CBF_RES_ERR;
    }
  }
  return CBF_RES_OK;
}

CBFresponsee CBFdyn_b_add(CBFdyndata *dyndata, long long int bsubi, double bval) {
  if (dyndata->data->bnnz + 1 > dyndata->bdyncap)
    return CBF_RES_ERR;

  ++dyndata->data->bnnz;
  dyndata->data->bsubi[dyndata->data->bnnz - 1] = bsubi;
  dyndata->data->bval[dyndata->data->bnnz - 1] = bval;

  return CBF_RES_OK;
}

CBFresponsee CBFdyn_h_capacitysurplus(CBFdyndata *dyndata, long long int surplus) {
  long long int size;
  long long int *buf2;
  int *buf1, *buf3, *buf4;
  double *buf5;

  if (dyndata->data->hnnz > dyndata->hdyncap)
    return CBF_RES_ERR;

  size = dyndata->data->hnnz + surplus;

  if (size > dyndata->hdyncap) {
    buf1 = (int*) realloc(dyndata->data->hsubi, size * sizeof(dyndata->data->hsubi[0]));
    buf2 = (long long int*) realloc(dyndata->data->hsubj, size * sizeof(dyndata->data->hsubj[0]));
    buf3 = (int*) realloc(dyndata->data->hsubk, size * sizeof(dyndata->data->hsubk[0]));
    buf4 = (int*) realloc(dyndata->data->hsubl, size * sizeof(dyndata->data->hsubl[0]));
    buf5 = (double*) realloc(dyndata->data->hval, size * sizeof(dyndata->data->hval[0]));

    if (buf1 && buf2 && buf3 && buf4 && buf5) {
      dyndata->data->hsubi = buf1;
      dyndata->data->hsubj = buf2;
      dyndata->data->hsubk = buf3;
      dyndata->data->hsubl = buf4;
      dyndata->data->hval = buf5;
      dyndata->hdyncap = size;
    } else {
      return CBF_RES_ERR;
    }
  }
  return CBF_RES_OK;
}

CBFresponsee CBFdyn_h_add(CBFdyndata *dyndata, int hsubi, long long int hsubj, int hsubk, int hsubl, double hval) {
  if (dyndata->data->hnnz + 1 > dyndata->hdyncap)
    return CBF_RES_ERR;

  ++dyndata->data->hnnz;
  dyndata->data->hsubi[dyndata->data->hnnz - 1] = hsubi;
  dyndata->data->hsubj[dyndata->data->hnnz - 1] = hsubj;
  dyndata->data->hsubk[dyndata->data->hnnz - 1] = hsubk;
  dyndata->data->hsubl[dyndata->data->hnnz - 1] = hsubl;
  dyndata->data->hval[dyndata->data->hnnz - 1] = hval;

  return CBF_RES_OK;
}

CBFresponsee CBFdyn_d_capacitysurplus(CBFdyndata *dyndata, long long int surplus) {
  long long int size;
  int *buf1, *buf2, *buf3;
  double *buf4;

  if (dyndata->data->dnnz > dyndata->ddyncap)
    return CBF_RES_ERR;

  size = dyndata->data->dnnz + surplus;

  if (size > dyndata->ddyncap) {
    buf1 = (int*) realloc(dyndata->data->dsubi, size * sizeof(dyndata->data->dsubi[0]));
    buf2 = (int*) realloc(dyndata->data->dsubk, size * sizeof(dyndata->data->dsubk[0]));
    buf3 = (int*) realloc(dyndata->data->dsubl, size * sizeof(dyndata->data->dsubl[0]));
    buf4 = (double*) realloc(dyndata->data->dval, size * sizeof(dyndata->data->dval[0]));

    if (buf1 && buf2 && buf3 && buf4) {
      dyndata->data->dsubi = buf1;
      dyndata->data->dsubk = buf2;
      dyndata->data->dsubl = buf3;
      dyndata->data->dval = buf4;
      dyndata->ddyncap = size;
    } else {
      return CBF_RES_ERR;
    }
  }
  return CBF_RES_OK;
}

CBFresponsee CBFdyn_d_add(CBFdyndata *dyndata, int dsubi, int dsubk, int dsubl, double dval) {
  if (dyndata->data->dnnz + 1 > dyndata->ddyncap)
    return CBF_RES_ERR;

  ++dyndata->data->dnnz;
  dyndata->data->dsubi[dyndata->data->dnnz - 1] = dsubi;
  dyndata->data->dsubk[dyndata->data->dnnz - 1] = dsubk;
  dyndata->data->dsubl[dyndata->data->dnnz - 1] = dsubl;
  dyndata->data->dval[dyndata->data->dnnz - 1] = dval;

  return CBF_RES_OK;
}

CBFresponsee CBFdyn_varbound_capacitysurplus(CBFdyndata *dyndata, long long int surplus) {
  CBFresponsee res = CBF_RES_OK;
  res = CBFdyn_map_capacitysurplus(dyndata, surplus);

  if (res == CBF_RES_OK)
    CBFdyn_a_capacitysurplus(dyndata, surplus);

  if (res == CBF_RES_OK)
    CBFdyn_b_capacitysurplus(dyndata, surplus);

  return res;
}

CBFresponsee CBFdyn_varbound_addlower(CBFdyndata *dyndata, long long int idx, double lower) {
  // x[i] - lower >= 0
  CBFresponsee res = CBF_RES_OK;
  res = CBFdyn_map_adddomain(dyndata, CBF_CONE_POS, 1);

  if (res == CBF_RES_OK)
    if (lower != 0.0)
      res = CBFdyn_b_add(dyndata, dyndata->data->mapnum - 1, -lower);

  if (res == CBF_RES_OK)
    res = CBFdyn_a_add(dyndata, dyndata->data->mapnum - 1, idx, 1.0);

  return res;
}

CBFresponsee CBFdyn_varbound_addupper(CBFdyndata *dyndata, long long int idx, double upper) {
  // x[i] - upper <= 0
  CBFresponsee res = CBF_RES_OK;
  res = CBFdyn_map_adddomain(dyndata, CBF_CONE_NEG, 1);

  if (res == CBF_RES_OK)
    if (upper != 0.0)
      res = CBFdyn_b_add(dyndata, dyndata->data->mapnum - 1, -upper);

  if (res == CBF_RES_OK)
    res = CBFdyn_a_add(dyndata, dyndata->data->mapnum - 1, idx, 1.0);

  return res;
}

CBFresponsee CBFdyn_varbound_addfix(CBFdyndata *dyndata, long long int idx, double fix) {
  // x[i] - fix == 0
  CBFresponsee res = CBF_RES_OK;
  res = CBFdyn_map_adddomain(dyndata, CBF_CONE_ZERO, 1);

  if (res == CBF_RES_OK)
    if (fix != 0.0)
      res = CBFdyn_b_add(dyndata, dyndata->data->mapnum - 1, -fix);

  if (res == CBF_RES_OK)
    res = CBFdyn_a_add(dyndata, dyndata->data->mapnum - 1, idx, 1.0);

  return res;
}
