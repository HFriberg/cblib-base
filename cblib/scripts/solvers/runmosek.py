# Copyright (c) 2012 by Zuse-Institute Berlin and the Technical University of Denmark.
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.


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
