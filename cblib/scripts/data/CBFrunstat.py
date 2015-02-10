# Copyright (c) 2012 by Zuse-Institute Berlin and the Technical University of Denmark.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     1. Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#     2. Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#     3. Neither the name of the copyright holders nor contributors may not
#        be used to endorse or promote products derived from this software
#        without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS NOR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os, sys, inspect
from scipy import sparse
from data.CBFdata import CBFdata

class CBFrunstat:
  def __init__(self, file):
    self.problem = CBFdata(file)
    self.solution = None
    self.time = None

    # Find the directory of this script
    scriptdir = os.path.split(inspect.getfile( inspect.currentframe() ))[0]

    # Import all cones from 'dist'
    for dirpath, dirnames, filenames in os.walk(os.path.realpath(os.path.abspath(os.path.join(scriptdir, '..', 'dist'))) ):
      self.distdict = dict([(f, __import__('dist.' + f, fromlist=f)) for f in [os.path.splitext(f)[0] for f in filenames if f[:1] != '_']])
      break


  def report(self):
    maxprimdist = dict()
    maxdualdist = dict()
    primobjval = None
    dualobjval = None

    primobjval = self.problem.objbval
    for j in range(self.problem.objannz):
      primobjval += self.problem.objaval[j] * self.solution.primvar[self.problem.objasubj[j]]

    j = 0
    for k in range(self.problem.varstacknum):
      dist = self.distdict[self.problem.varstackdomain[k]].primdist(self.solution.primvar[j:j+self.problem.varstackdim[k]])
      if self.problem.varstackdomain[k] in maxprimdist:
        maxprimdist[self.problem.varstackdomain[k]] = max(maxprimdist[self.problem.varstackdomain[k]], dist)
      else:
        maxprimdist[self.problem.varstackdomain[k]] = dist

      j += self.problem.varstackdim[k]

    A = sparse.coo_matrix((self.problem.aval, (self.problem.asubi, self.problem.asubj)), shape=(self.problem.mapnum, self.problem.varnum))
    primmap = A.dot(list(self.solution.primvar))
    for i in range(self.problem.bnnz):
      primmap[self.problem.bsubi[i]] += self.problem.bval[i]

    i = 0
    for k in range(self.problem.mapstacknum):
      dist = self.distdict[self.problem.mapstackdomain[k]].primdist(primmap[i:i+self.problem.mapstackdim[k]])
      if self.problem.mapstackdomain[k] in maxprimdist:
        maxprimdist[self.problem.mapstackdomain[k]] = max(maxprimdist[self.problem.mapstackdomain[k]], dist)
      else:
        maxprimdist[self.problem.mapstackdomain[k]] = dist

      i += self.problem.mapstackdim[k]

    if self.problem.intvarnum >= 1:
      maxprimdist['INTEGER'] = self.distdict['INTEGER'].primdist([self.solution.primvar[j] for j in self.problem.intvar])

    else:
      dualobjval = self.problem.objbval
      for i in range(self.problem.bnnz):
        dualobjval -= self.problem.bval[i] * self.solution.dualvar[self.problem.bsubi[i]]

      i = 0
      for k in range(self.problem.mapstacknum):
        dist = self.distdict[self.problem.mapstackdomain[k]].dualdist(self.solution.dualvar[i:i+self.problem.mapstackdim[k]])
        if self.problem.mapstackdomain[k]+'*' in maxdualdist:
          maxdualdist[self.problem.mapstackdomain[k]+'*'] = max(maxdualdist[self.problem.mapstackdomain[k]+'*'], dist)
        else:
          maxdualdist[self.problem.mapstackdomain[k]+'*'] = dist

        i += self.problem.mapstackdim[k]

      dualmap = A.transpose().dot(list(self.solution.dualvar))
      for j in range(self.problem.objannz):
        dualmap[self.problem.objasubj[j]] -= self.problem.objaval[j]
      dualmap = dualmap.dot(-1)

      j = 0
      for k in range(self.problem.varstacknum):
        dist = self.distdict[self.problem.varstackdomain[k]].dualdist(dualmap[j:j+self.problem.varstackdim[k]])
        if self.problem.varstackdomain[k]+'*' in maxdualdist:
          maxdualdist[self.problem.varstackdomain[k]+'*'] = max(maxdualdist[self.problem.varstackdomain[k]+'*'], dist)
        else:
          maxdualdist[self.problem.varstackdomain[k]+'*'] = dist

        j += self.problem.varstackdim[k]

    return((self.time, primobjval, dualobjval, maxprimdist, maxdualdist))
