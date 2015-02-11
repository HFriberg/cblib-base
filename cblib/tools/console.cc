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

#include "console.h"

#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// -------------------------------------
// Function definitions
// -------------------------------------

void printoptions(const CBFfrontend **plugs_frontend, const CBFbackend **plugs_backend, const CBFtransform **plugs_transform,
    const CBFfrontend *default_frontend, const CBFbackend *default_backend, const CBFtransform *default_transform) {
  int i;
  printf("OPTIONS:\n");

  if (plugs_frontend[0] != NULL) {
    printf("  -i format   : File manager for input files:\n");
    printf("                ");
    for (i = 0; plugs_frontend[i] != NULL; ++i) {
      if (plugs_frontend[i] == default_frontend)
        printf("(%s), ", plugs_frontend[i]->name);
      else
        printf("%s, ", plugs_frontend[i]->name);
    }
    printf("\n");
  }

  if (plugs_backend[0] != NULL) {
    printf("  -o format   : File manager for output files:\n");
    printf("                ");
    for (i = 0; plugs_backend[i] != NULL; ++i) {
      if (plugs_backend[i] == default_backend)
        printf("(%s), ", plugs_backend[i]->name);
      else
        printf("%s, ", plugs_backend[i]->name);
    }
    printf("\n");
  }

  if (plugs_transform[0] != NULL) {
    printf("  -t method   : Problem transformation manager:\n");
    printf("                ");
    for (i = 0; plugs_transform[i] != NULL; ++i) {
      if (plugs_transform[i] == default_transform)
        printf("(%s), ", plugs_transform[i]->name);
      else
        printf("%s, ", plugs_transform[i]->name);
    }
    printf("\n");
  }

  printf("  -opath path : Output destination.\n");
  printf("  -pfix name  : Postfix for output files.\n");

  printf("\n\n");
}

CBFresponsee getoptions(int argc, char *argv[], const CBFfrontend **plugs_frontend, const CBFbackend **plugs_backend, const CBFtransform **plugs_transform,
    const CBFfrontend **frontend, const CBFbackend **backend, const CBFtransform **transform, const char **opath, const char **pfix) {
  CBFresponsee res = CBF_RES_OK;
  char const *frontend_name = "";
  char const *backend_name = "";
  char const *transform_name = "";
  int i;

  for (i = 1; i < argc && res == CBF_RES_OK; ++i) {
    if (argv[i]) {

      if (strcmp(argv[i], "-i") == 0) {
        if (i + 1 < argc) {
          frontend_name = argv[i + 1];
          *frontend = NULL;
          argv[i] = NULL;
          argv[i + 1] = NULL;
        } else {
          res = CBF_RES_ERR;
        }
      }

      else if (strcmp(argv[i], "-o") == 0) {
        if (i + 1 < argc) {
          backend_name = argv[i + 1];
          *backend = NULL;
          argv[i] = NULL;
          argv[i + 1] = NULL;
        } else {
          res = CBF_RES_ERR;
        }
      }

      else if (strcmp(argv[i], "-t") == 0) {
        if (i + 1 < argc) {
          transform_name = argv[i + 1];
          *transform = NULL;
          argv[i] = NULL;
          argv[i + 1] = NULL;
        } else {
          res = CBF_RES_ERR;
        }
      }

      else if (strcmp(argv[i], "-opath") == 0) {
        if (i + 1 < argc) {
          *opath = argv[i + 1];
          argv[i] = NULL;
          argv[i + 1] = NULL;
        } else {
          res = CBF_RES_ERR;
        }
      }

      else if (strcmp(argv[i], "-pfix") == 0) {
        if (i + 1 < argc) {
          *pfix = argv[i + 1];
          argv[i] = NULL;
          argv[i + 1] = NULL;
        } else {
          res = CBF_RES_ERR;
        }
      }
    }
  }

  if (res == CBF_RES_OK) {
    // Identify frontend by name
    for (i = 0; plugs_frontend[i] != NULL; ++i) {
      if (strcmp(frontend_name, plugs_frontend[i]->name) == 0) {
        *frontend = plugs_frontend[i];
        break;
      }
    }

    // Identify backend by name
    for (i = 0; plugs_backend[i] != NULL; ++i) {
      if (strcmp(backend_name, plugs_backend[i]->name) == 0) {
        *backend = plugs_backend[i];
        break;
      }
    }

    // Identify transform by name
    for (i = 0; plugs_transform[i] != NULL; ++i) {
      if (strcmp(transform_name, plugs_transform[i]->name) == 0) {
        *transform = plugs_transform[i];
        break;
      }
    }

    if (*frontend == NULL || *backend == NULL || *transform == NULL)
      res = CBF_RES_ERR;
  }

  return res;
}

const std::string swapfiledirandext(const char *ifile, const char *newpath, const char *newpostfix, const char *newformat) {
  std::string ifilestr = ifile;
  std::string ofilestr;
  int from, len;

  // path
  if (newpath && newpath[0] != '\0') {
    ofilestr = newpath;
    if (*ofilestr.rbegin() != '/' && *ofilestr.rbegin() != '\\') {
      ofilestr += "/";
    }
  }

  // name
  if (ifilestr.length() >= 3 && ifilestr.substr(ifilestr.length() - 3, 3) == ".gz")
    ifilestr[ifilestr.length() - 3] = '\0';

  from = ifilestr.find_last_of("/\\") + 1;
  len = ifilestr.find_last_of(".") - from;
  if (len <= 0) {
    len = ifilestr.length() - from;
  }
  ofilestr += ifilestr.substr(from, len);

  // postfix
  if (newpostfix && newpostfix[0] != '\0') {
    ofilestr += newpostfix;
  }

  // format
  ofilestr += ".";
  ofilestr += newformat;

  return ofilestr;
}

CBFresponsee processfile(const CBFfrontend *frontend, const CBFbackend *backend, const CBFtransform *transform, const char *ifile, const char *ofile) {
  CBFresponsee res = CBF_RES_OK;
  CBFfrontendmemory mem = { 0, };
  CBFtransform_param param;
  CBFdata data = { 0, };

  // Read file
  res = frontend->read(ifile, &data, &mem);

  if (res != CBF_RES_OK) {
    printf("Failed to read file: %s\n", ifile);

  } else {
    // Initialize parameters
    param.init(&data);

    // Transform file
    res = transform->transform(&data, param);

    if (res != CBF_RES_OK) {
      printf("Failed to transform file: %s\n", ifile);

    } else {
      // Write file
      res = backend->write(ofile, data);

      if (res != CBF_RES_OK)
        printf("Failed to write file: %s\n", ofile);
    }

    // Clean data structure
    frontend->clean(&data, &mem);
  }

  return res;
}
