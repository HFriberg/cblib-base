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

#ifndef CBF_CPF_FORMAT_H
#define CBF_CPF_FORMAT_H

#include "cbf-data.h"
#include "programmingstyle.h"

CBFresponsee CBF_conetostr(CBFscalarconee cone, const char **str);
CBFresponsee CBF_strtocone(const char *str, CBFscalarconee *cone);
CBFresponsee CBF_objsensetostr(CBFobjsensee cone, const char **str);
CBFresponsee CBF_strtoobjsense(const char *str, CBFobjsensee *cone);

// Use CBF_NAME_FORMAT instead of %s when parsing lines,
// to avoid buffer overflow.
#define MACRO_STR_EXPAND(tok) #tok
#define MACRO_STR(tok) MACRO_STR_EXPAND(tok)
#define CBF_NAME_FORMAT "%" MACRO_STR(CBF_MAX_NAME) "s"

extern char CBF_LINE_BUFFER[CBF_MAX_LINE];
extern char CBF_NAME_BUFFER[CBF_MAX_NAME];

extern const char * CBF_CONENAM_FREE;
extern const char * CBF_CONENAM_ZERO;
extern const char * CBF_CONENAM_POS;
extern const char * CBF_CONENAM_NEG;
extern const char * CBF_CONENAM_QUAD;
extern const char * CBF_CONENAM_RQUAD;

extern const char * CBF_OBJSENSENAM_MIN;
extern const char * CBF_OBJSENSENAM_MAX;

#endif

