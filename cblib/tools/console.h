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
    const char         **pfix,
    bool                *verbose);

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
    const char *ofile,
    const bool verbose);

#endif
