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

#ifndef CBF_BACKEND_MPS_H
#define CBF_BACKEND_MPS_H

#include "cbf-data.h"
#include "programmingstyle.h"
#include <stdio.h>      // Unfortunately, no portable forward declaration of FILE

CBFresponsee
  MPS_writeNAME(FILE *pFile, const CBFdata data);

CBFresponsee
  MPS_writeOBJSENSE(FILE *pFile, const CBFdata data);

CBFresponsee
  MPS_writeROWS(FILE *pFile, const CBFdata data);

CBFresponsee
  MPS_writeCOLUMNS(FILE *pFile, const CBFdata data);

CBFresponsee
  MPS_writeRHS(FILE *pFile, const CBFdata data);

CBFresponsee
  MPS_writeBOUNDS(FILE *pFile, const CBFdata data);

CBFresponsee
  MPS_writeENDATA(FILE *pFile, const CBFdata data);

#endif
