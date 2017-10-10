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
#include "programmingstyle.h"

#include <stdio.h>
#include <string.h>

// -------------------------------------
// Function definitions
// -------------------------------------

int main(int argc, char **argv)
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

