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

#include "frontend-cbf.h"
#include "programmingstyle.h"

#include <string>
#include <stdio.h>


// -------------------------------------
// Function definitions
// -------------------------------------

int main (int argc, char *argv[])
{
    CBFresponsee res = CBF_RES_OK;
    CBFfrontendmemory mem = { 0, };
    CBFdata data = { 0, };

    if (argc <= 1)
    {
        printf("\nBad command, syntax is:\n");
        printf(">> minimalreader ifile.cbf\n\n");
    }
    else
    {
        const char * ifile = argv[1];

        res = frontend_cbf.read(ifile, &data, &mem);

        if (res != CBF_RES_OK) {
            printf("Failed to read file: %s\n", ifile);
        }
        else
        {
            printf("CON: %lli, VAR: %lli, PSDCON: %i, PSDVAR: %i\n", data.mapnum, data.varnum, data.psdmapnum, data.psdvarnum);
        }

        // Clean data structure
        frontend_cbf.clean(&data, &mem);
    }

    return 0;
}