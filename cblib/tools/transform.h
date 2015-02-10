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

#ifndef CBF_TRANSFORM_H
#define CBF_TRANSFORM_H

#include "cbf-data.h"
#include "programmingstyle.h"
#include <stdlib.h>

typedef struct CBFtransform_param_struct {

  enum pare {
    NONE, FAST, FULL
  };

  // Preprocessing parameters
  bool USE_RQUAD_CONVERTION;
  bool USE_CONIC_SINGLETON_ELIMINATION;

  bool USE_IMPLIED_INTEGER_ANALYSIS;

  pare USE_VARBOUNDING;
  bool USE_VARBOUNDING_FAST_DECOUPLEDCONE;

  bool USE_BINVAR_REFORMULATION;

  pare USE_LINDEP_ELIMINATION;
  bool USE_LINDEP_ELIMINATION_FULL_NNZCONSERVATIVE;

  bool USE_PROBE;

  pare USE_PROBE_VARBOUNDING;
  bool USE_PROBE_VARBOUNDING_FAST_DECOUPLEDCONE;

  bool USE_PROBE_INFEAS_DECOUPLEDCONE;
  pare USE_PROBE_INFEAS_INEQFREE;

  bool USE_PROBE_REDUND_DECOUPLEDCONE;
  bool USE_PROBE_REDUND_SEMIDEF;

  CBFresponsee init(CBFdata *data) {

    USE_RQUAD_CONVERTION = true;
    USE_CONIC_SINGLETON_ELIMINATION = false;
    USE_IMPLIED_INTEGER_ANALYSIS = false;
    USE_VARBOUNDING = NONE;
    USE_VARBOUNDING_FAST_DECOUPLEDCONE = false;
    USE_BINVAR_REFORMULATION = false;
    USE_LINDEP_ELIMINATION = FULL;
    USE_LINDEP_ELIMINATION_FULL_NNZCONSERVATIVE = false;
    USE_PROBE = false;
    USE_PROBE_VARBOUNDING = NONE;
    USE_PROBE_VARBOUNDING_FAST_DECOUPLEDCONE = false;
    USE_PROBE_INFEAS_DECOUPLEDCONE = false;
    USE_PROBE_INFEAS_INEQFREE = NONE;
    USE_PROBE_REDUND_DECOUPLEDCONE = false;
    USE_PROBE_REDUND_SEMIDEF = false;

    return CBF_RES_OK;
  }

} CBFtransform_param;

typedef struct CBFtransform_struct {

  const char *name;
  CBFresponsee (*transform)(CBFdata *data, CBFtransform_param param);
  CBFresponsee (*revert)(CBFdata *data, CBFtransform_param param);

} CBFtransform;

#endif
