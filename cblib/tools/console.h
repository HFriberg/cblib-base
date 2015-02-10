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

#ifndef CBF_CONSOLE_H
#define CBF_CONSOLE_H

#include "frontend.h"
#include "backend.h"
#include "transform.h"
#include "programmingstyle.h"

#include <string>

void printoptions(
    const CBFfrontend  **plugs_frontend,
    const CBFbackend   **plugs_backend,
    const CBFtransform **plugs_transform,
    const CBFfrontend   *default_frontend,
    const CBFbackend    *default_backend,
    const CBFtransform  *default_transform);

CBFresponsee getoptions(int argc, char *argv[],
    const CBFfrontend  **plugs_frontend,
    const CBFbackend   **plugs_backend,
    const CBFtransform **plugs_transform,
    const CBFfrontend  **frontend,
    const CBFbackend   **backend,
    const CBFtransform **transform,
    const char         **opath,
    const char         **pfix);

const std::string swapfiledirandext(
    const char *ifile,
    const char *newpath,
    const char *newpostfix,
    const char *newformat);

CBFresponsee processfile(
    const CBFfrontend  *frontend,
    const CBFbackend   *backend,
    const CBFtransform *transform,
    const char *ifile,
    const char *ofile);

#endif
