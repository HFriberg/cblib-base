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

import mosek
from data.CBFsolution import CBFsolution

class solver:
  def __init__(self, printer):
    self.printer = printer
    self.env = mosek.Env()
    self.task = None
    self.cbfsupport = False

  def read(self, file, paramfile, cbfdata):
    self.cbfdata = cbfdata
    self.task = self.env.Task(0, 0)

    if self.printer:
      self.task.set_Stream(mosek.streamtype.log, self.printer)

    if paramfile:
      self.task.readparamfile(paramfile)

    if self.cbfsupport:
      self.task.readdata(file)
    else:
      self.__read_through_pythonapi(cbfdata)

  def optimize(self):
    self.task.optimize()

  def getsizeoftree(self):
    return(self.task.getintinf(mosek.iinfitem.mio_num_branch))

  def getsolution(self):
    sol = CBFsolution()
    varnum    = self.cbfdata.varnum
    intvarnum = self.cbfdata.intvarnum
    mapnum    = self.cbfdata.mapnum

    prosta = ''
    solsta = ''
    xx  = mosek.array.zeros(varnum, float)
    slx = mosek.array.zeros(mapnum, float)
    sux = mosek.array.zeros(mapnum, float)
    snx = mosek.array.zeros(mapnum, float)

    # Get solution from MOSEK
    if intvarnum >= 1 and self.task.solutiondef(mosek.soltype.itg):
      self.task.getxxslice(mosek.soltype.itg, 0, varnum, xx)
      sol.primvar = xx

      prosta = self.task.getprosta(mosek.soltype.itg)
      solsta = self.task.getsolsta(mosek.soltype.itg)
      if solsta == mosek.solsta.integer_optimal:
        sol.claim = sol.claimlist[0]
      elif prosta == mosek.prosta.prim_infeas:
        sol.claim = sol.claimlist[1]

    elif self.task.solutiondef(mosek.soltype.itr):
      self.task.getxxslice(mosek.soltype.itr, 0, varnum, xx)
      sol.primvar = xx

      self.task.getslxslice(mosek.soltype.itr, varnum, varnum+mapnum, slx)
      self.task.getsuxslice(mosek.soltype.itr, varnum, varnum+mapnum, sux)
      self.task.getsnxslice(mosek.soltype.itr, varnum, varnum+mapnum, snx)
      sol.dualvar = [snxval+slxval+(-suxval) for snxval,suxval,slxval in zip(snx,sux,slx)]

    return(sol)


  def __read_through_pythonapi(self, cbfdata):

    # Append row, columns and non-zeros
    self.task.appendcons(cbfdata.mapnum)
    self.task.appendvars(cbfdata.varnum + cbfdata.mapnum)
    self.task.putmaxnumanz(cbfdata.annz + cbfdata.mapnum)

    # Objective sense
    if self.cbfdata.obj == 'MAX':
      self.task.putobjsense(mosek.objsense.maximize)
    else:
      self.task.putobjsense(mosek.objsense.minimize)

    # Objective (cx)
    self.task.putcfix(cbfdata.objbval)
    self.task.putclist(cbfdata.objasubj, cbfdata.objaval)

    # Constraints (Ax - s = -b)
    rhs = [-x for x in cbfdata.bval]
    self.task.putaijlist(cbfdata.asubi, cbfdata.asubj, cbfdata.aval)
    self.task.putaijlist(range(cbfdata.mapnum), range(cbfdata.varnum, cbfdata.varnum + cbfdata.mapnum), [-1.0]*cbfdata.mapnum)
    self.task.putboundlist(mosek.accmode.con, range(cbfdata.mapnum), [mosek.boundkey.fx]*cbfdata.mapnum, [0.0]*cbfdata.mapnum, [0.0]*cbfdata.mapnum)
    self.task.putboundlist(mosek.accmode.con, cbfdata.bsubi, [mosek.boundkey.fx]*cbfdata.bnnz, rhs, rhs)

    # Integer variables
    self.task.putvartypelist(cbfdata.intvar, [mosek.variabletype.type_int]*cbfdata.intvarnum)

    # Variable bounds and conic domains
    bkdict = dict([('F',mosek.boundkey.fr), ('L+',mosek.boundkey.lo), ('L-',mosek.boundkey.up), ('L=',mosek.boundkey.fx)])

    j = 0
    for k in range(cbfdata.varstacknum):
      if cbfdata.varstackdomain[k] == 'Q':
        self.task.appendcone(mosek.conetype.quad, 0.0, range(j, j+cbfdata.varstackdim[k]))
        simplebound = mosek.boundkey.fr
      elif cbfdata.varstackdomain[k] == 'QR':
        self.task.appendcone(mosek.conetype.rquad, 0.0, range(j, j+cbfdata.varstackdim[k]))
        simplebound = mosek.boundkey.fr
      else:
        simplebound = bkdict[cbfdata.varstackdomain[k]]

      for km in range(cbfdata.varstackdim[k]):
        self.task.putbound(mosek.accmode.var, j, simplebound, 0.0, 0.0)
        j += 1

    for k in range(cbfdata.mapstacknum):
      if cbfdata.mapstackdomain[k] == 'Q':
        self.task.appendcone(mosek.conetype.quad, 0.0, range(j, j+cbfdata.mapstackdim[k]))
        simplebound = mosek.boundkey.fr
      elif cbfdata.mapstackdomain[k] == 'QR':
        self.task.appendcone(mosek.conetype.rquad, 0.0, range(j, j+cbfdata.mapstackdim[k]))
        simplebound = mosek.boundkey.fr
      else:
        simplebound = bkdict[cbfdata.mapstackdomain[k]]

      for km in range(cbfdata.mapstackdim[k]):
        self.task.putbound(mosek.accmode.var, j, simplebound, 0.0, 0.0)
        j += 1

  def report(self):
    self.task.solutionsummary(mosek.streamtype.log)
