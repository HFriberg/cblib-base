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

import cplex
from data.CBFsolution import CBFsolution

class Printer:
    def __init__(self, printer):
      self.write = printer
    def write(x):
      pass
    def flush(x):
      pass


class solver:
  def __init__(self, printer):
    self.printer = Printer(printer)
    self.task = None
    self.cbfsupport = False
  
  def read(self, file, paramfile, cbfdata):
    self.cbfdata = cbfdata
    self.task = cplex.Cplex()

    if self.printer:
      self.task.set_log_stream(self.printer)
      self.task.set_results_stream(self.printer)

    if paramfile:
      self.task.parameters.read_file(paramfile)

    if self.cbfsupport:
      self.task.read(file)
    else:
      self.__read_through_pythonapi(cbfdata)

  def optimize(self):
    self.task.solve()

  def getsizeoftree(self):
    return(self.task.solution.progress.get_num_nodes_processed())

  def getsolution(self):
    sol = CBFsolution()
    varnum    = self.cbfdata.varnum
    intvarnum = self.cbfdata.intvarnum
    mapnum    = self.cbfdata.mapnum

    # Get solution from CPLEX
    sol.primvar = self.task.solution.get_values(0, varnum-1)

    if intvarnum >= 1:
      solsta = self.task.solution.get_status()
      if solsta == self.task.solution.status.MIP_optimal:
        sol.claim = sol.claimlist[0]
      elif solsta == self.task.solution.status.MIP_infeasible:
        sol.claim = sol.claimlist[1]

    else:
      redcost = self.task.solution.get_reduced_costs()
      #dualvar = self.task.solution.get_dual_values()
      dualslack = self.task.solution.get_quadratic_dualslack()

      sol.dualvar = redcost
      for v in dualslack:
        for (var,val) in zip(v.ind, v.val):
            sol.dualvar[var] += val
      sol.dualvar = sol.dualvar[varnum:varnum+mapnum]

    return(sol)

  def __read_through_pythonapi(self, cbfdata):

    # Append row, columns and non-zeros
    mapnum = cbfdata.mapnum
    varnum = cbfdata.varnum + cbfdata.mapnum

    self.task.linear_constraints.add(rhs=[0.0]*mapnum, senses=["E"]*mapnum)
    self.task.variables.add(obj=[0.0]*varnum, lb=[-cplex.infinity]*varnum, ub=[cplex.infinity]*varnum)

    # Objective sense
    if self.cbfdata.obj == 'MAX':
      self.task.objective.set_sense(self.task.objective.sense.maximize)
    else:
      self.task.objective.set_sense(self.task.objective.sense.minimize)

    # Objective (cx)
    self.task.objective.set_linear(zip(cbfdata.objasubj, cbfdata.objaval))

    # Constraints (Ax - s = -b)
    rhs = [-x for x in cbfdata.bval]
    self.task.linear_constraints.set_coefficients(zip(cbfdata.asubi, cbfdata.asubj, cbfdata.aval))
    self.task.linear_constraints.set_coefficients(zip(range(cbfdata.mapnum), range(cbfdata.varnum, cbfdata.varnum + cbfdata.mapnum), [-1.0]*cbfdata.mapnum))
    self.task.linear_constraints.set_rhs(zip(cbfdata.bsubi, rhs))

    # Integer variables
    if cbfdata.intvarnum:
      self.task.variables.set_types(zip(cbfdata.intvar, [self.task.variables.type.integer]*cbfdata.intvarnum))

    # Variable bounds and conic domains
    j = 0
    for k in range(cbfdata.varstacknum):
      if cbfdata.varstackdomain[k] == 'Q':
        self.task.quadratic_constraints.add(quad_expr=[range(j,j+cbfdata.varstackdim[k]), range(j,j+cbfdata.varstackdim[k]), [-1] + [1]*(cbfdata.varstackdim[k]-1)])
        self.task.variables.set_lower_bounds(zip([j], [0.0]))
      elif cbfdata.varstackdomain[k] == 'QR':
        self.task.quadratic_constraints.add(quad_expr=[[j+1] + range(j+2,j+cbfdata.varstackdim[k]), [j] + range(j+2,j+cbfdata.varstackdim[k]), [-2] + [1]*(cbfdata.varstackdim[k]-2)])
        self.task.variables.set_lower_bounds(zip([j, j+1], [0.0, 0.0]))
      elif cbfdata.varstackdomain[k] == 'L+':
        self.task.variables.set_lower_bounds(zip(range(j, j+cbfdata.varstackdim[k]), [0.0]*cbfdata.varstackdim[k]))
      elif cbfdata.varstackdomain[k] == 'L-':
        self.task.variables.set_upper_bounds(zip(range(j, j+cbfdata.varstackdim[k]), [0.0]*cbfdata.varstackdim[k]))
      elif cbfdata.varstackdomain[k] == 'L=':
        self.task.variables.set_lower_bounds(zip(range(j, j+cbfdata.varstackdim[k]), [0.0]*cbfdata.varstackdim[k]))
        self.task.variables.set_upper_bounds(zip(range(j, j+cbfdata.varstackdim[k]), [0.0]*cbfdata.varstackdim[k]))

      j += cbfdata.varstackdim[k]

    for k in range(cbfdata.mapstacknum):
      if cbfdata.mapstackdomain[k] == 'Q':
        self.task.quadratic_constraints.add(quad_expr=[range(j,j+cbfdata.mapstackdim[k]), range(j,j+cbfdata.mapstackdim[k]), [-1] + [1]*(cbfdata.mapstackdim[k]-1)])
        self.task.variables.set_lower_bounds(zip([j], [0.0]))
      elif cbfdata.mapstackdomain[k] == 'QR':
        self.task.quadratic_constraints.add(quad_expr=[[j+1] + range(j+2,j+cbfdata.mapstackdim[k]), [j] + range(j+2,j+cbfdata.mapstackdim[k]), [-2] + [1]*(cbfdata.mapstackdim[k]-2)])
        self.task.variables.set_lower_bounds(zip([j, j+1], [0.0, 0.0]))
      elif cbfdata.mapstackdomain[k] == 'L+':
        self.task.variables.set_lower_bounds(zip(range(j, j+cbfdata.mapstackdim[k]), [0.0]*cbfdata.mapstackdim[k]))
      elif cbfdata.mapstackdomain[k] == 'L-':
        self.task.variables.set_upper_bounds(zip(range(j, j+cbfdata.mapstackdim[k]), [0.0]*cbfdata.mapstackdim[k]))
      elif cbfdata.mapstackdomain[k] == 'L=':
        self.task.variables.set_lower_bounds(zip(range(j, j+cbfdata.mapstackdim[k]), [0.0]*cbfdata.mapstackdim[k]))
        self.task.variables.set_upper_bounds(zip(range(j, j+cbfdata.mapstackdim[k]), [0.0]*cbfdata.mapstackdim[k]))

      j += cbfdata.mapstackdim[k]


  def report(self):
    self.printer(str(self.task.solution.get_quality_metrics()));
    
