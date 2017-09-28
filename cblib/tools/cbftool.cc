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

#include "frontend-cbf.h"
#include "backend-cbf.h"
#include "backend-mps-mosek.h"
#include "backend-mps-cplex.h"
#include "backend-sdpa.h"
#include "transform-none.h"
#include "transform-dual.h"

#include "console.h"

#include <string>
#include <stdio.h>


// -------------------------------------
// Function definitions
// -------------------------------------

int main (int argc, char *argv[])
{
  CBFresponsee res = CBF_RES_OK;
  const CBFfrontend  *default_frontend,  *frontend;
  const CBFbackend   *default_backend,   *backend;
  const CBFtransform *default_transform, *transform;
  std::string ofile;
  const char *ifile;
  const char *opath;
  const char *pfix;
  bool verbose;
  int i;

  // For debugging crashes
  setbuf(stdout, NULL);

  // List of plugins
  const CBFfrontend *plugs_frontend[] = {&frontend_cbf,
                                         NULL};

  const CBFbackend  *plugs_backend[]  = {&backend_cbf,
                                         &backend_mps_cplex,
                                         &backend_mps_mosek,
                                         &backend_sdpa,
                                         NULL};

  const CBFtransform *plugs_transform[] = {&transform_none,
                                           &transform_dual,
                                           NULL};

  // Default options
  frontend  = default_frontend  = &frontend_cbf;
  backend   = default_backend   = &backend_cbf;
  transform = default_transform = &transform_none;
  opath = NULL;
  pfix  = NULL;
  verbose = true;

  // User defined options
  res = getoptions(argc, argv, plugs_frontend, plugs_backend, plugs_transform,
                   &frontend,
                   &backend,
                   &transform,
                   &opath,
                   &pfix,
                   &verbose);

  if (argc <= 1 || res != CBF_RES_OK)
  {
    printf("\nBad command, syntax is:\n");
    printf(">> cbftool [OPTIONS] infile1 infile2 infile3 ...\n\n");
    printoptions(plugs_frontend, plugs_backend, plugs_transform,
        default_frontend, default_backend, default_transform);
  }
  else
  {
    // All non-nullified arguments are filenames
    for (i=1; i<argc && res==CBF_RES_OK; ++i) {
      if (argv[i]) {
        ifile = argv[i];
        ofile = swapfiledirandext(ifile, opath, pfix, backend->format);

        res = processfile(frontend, backend, transform, ifile, ofile.c_str(), verbose);
      }
    }
  }

  return res;
}
