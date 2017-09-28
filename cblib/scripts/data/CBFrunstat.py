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


import os, sys, inspect
from scipy import sparse, array, dot
from data.CBFdata import CBFdata

def getdistdict():
  # Find the directory of this script
  scriptdir = os.path.split(inspect.getfile( inspect.currentframe() ))[0]

  # Import all cones from 'dist'
  for dirpath, dirnames, filenames in os.walk(os.path.join(scriptdir, '..', 'dist')):
    return dict([(f, __import__('dist.' + f, fromlist=f)) for f in [os.path.splitext(f)[0] for f in filenames if f[:1] != '_']])


def primvar_certificates(prob, sol):
  psol_err = dict()
  pray_err = dict()
  distdict = getdistdict()

  # Objective and variable activities are where solutions and rays agree
  pobj = prob.objbval
  for j in range(prob.objannz):
    pobj += prob.objaval[j] * sol.primvar[prob.objasubj[j]]

  j = 0
  for k in range(prob.varstacknum):
    dist = distdict[prob.varstackdomain[k]].primdist(sol.primvar[j:j+prob.varstackdim[k]])
    if prob.varstackdomain[k] in psol_err:
      psol_err[prob.varstackdomain[k]] = max(psol_err[prob.varstackdomain[k]], dist)
    else:
      psol_err[prob.varstackdomain[k]] = dist

    j += prob.varstackdim[k]

  # Map activities and integer requirements are where solutions and rays differ
  pray_err = psol_err.copy()

  A = sparse.coo_matrix((prob.aval, (prob.asubi, prob.asubj)), shape=(prob.mapnum, prob.varnum))
  map_activity = A * list(sol.primvar)

  i = 0
  for k in range(prob.mapstacknum):
    dist = distdict[prob.mapstackdomain[k]].primdist(map_activity[i:i+prob.mapstackdim[k]])
    if prob.mapstackdomain[k] in pray_err:
      pray_err[prob.mapstackdomain[k]] = max(pray_err[prob.mapstackdomain[k]], dist)
    else:
      pray_err[prob.mapstackdomain[k]] = dist

    i += prob.mapstackdim[k]

  for i in range(prob.bnnz):
    map_activity[prob.bsubi[i]] += prob.bval[i]

  i = 0
  for k in range(prob.mapstacknum):
    dist = distdict[prob.mapstackdomain[k]].primdist(map_activity[i:i+prob.mapstackdim[k]])
    if prob.mapstackdomain[k] in psol_err:
      psol_err[prob.mapstackdomain[k]] = max(psol_err[prob.mapstackdomain[k]], dist)
    else:
      psol_err[prob.mapstackdomain[k]] = dist

    i += prob.mapstackdim[k]

  if prob.intvarnum >= 1:
    psol_err['INTEGER'] = distdict['INTEGER'].primdist([sol.primvar[j] for j in prob.intvar])

  return((pobj, psol_err, pray_err))


def dualvar_certificates(prob, sol):
  dsol_err = dict()
  dray_err = dict()
  distdict = getdistdict()

  var_activities = array(sol.dualvar)
  vardomainfactor = 1
  mapdomainfactor = 1

  # Objective and variable activities are where solutions and rays agree
  dobj = prob.objbval
  for i in range(prob.bnnz):
    dobj -= prob.bval[i] * var_activities[prob.bsubi[i]]

  if prob.obj.strip().upper() == 'MAX':
    vardomainfactor = -1

  i = 0
  for k in range(prob.mapstacknum):
    dist = distdict[prob.mapstackdomain[k]].dualdist( dot(var_activities[i:i+prob.mapstackdim[k]], vardomainfactor) )
    if prob.mapstackdomain[k]+'*' in dsol_err:
      dsol_err[prob.mapstackdomain[k]+'*'] = max(dsol_err[prob.mapstackdomain[k]+'*'], dist)
    else:
      dsol_err[prob.mapstackdomain[k]+'*'] = dist

    i += prob.mapstackdim[k]

  # Map activities are where solutions and rays differ
  dray_err = dsol_err.copy()

  AT = sparse.coo_matrix((prob.aval, (prob.asubj, prob.asubi)), shape=(prob.varnum, prob.mapnum))
  map_activity = AT * list(sol.dualvar)

  if prob.obj.strip().upper() == 'MIN':
    mapdomainfactor = -1

  j = 0
  for k in range(prob.varstacknum):
    dist = distdict[prob.varstackdomain[k]].dualdist( dot(map_activity[j:j+prob.varstackdim[k]], mapdomainfactor) )
    if prob.varstackdomain[k]+'*' in dray_err:
      dray_err[prob.varstackdomain[k]+'*'] = max(dray_err[prob.varstackdomain[k]+'*'], dist)
    else:
      dray_err[prob.varstackdomain[k]+'*'] = dist

    j += prob.varstackdim[k]

  for j in range(prob.objannz):
    map_activity[prob.objasubj[j]] -= prob.objaval[j]

  j = 0
  for k in range(prob.varstacknum):
    dist = distdict[prob.varstackdomain[k]].dualdist( dot(map_activity[j:j+prob.varstackdim[k]], mapdomainfactor) )
    if prob.varstackdomain[k]+'*' in dsol_err:
      dsol_err[prob.varstackdomain[k]+'*'] = max(dsol_err[prob.varstackdomain[k]+'*'], dist)
    else:
      dsol_err[prob.varstackdomain[k]+'*'] = dist

    j += prob.varstackdim[k]


  return((dobj, dsol_err, dray_err))


def primobjstat_continuous(pfeas_obj, pfeas_err, pinfeas_err, dfeas_obj, dfeas_err, dinfeas_err, isminimize):

  if all([err == float('inf') for err in [pfeas_err, pinfeas_err, dinfeas_err]]):
    return(('?', ''))

  if isminimize:
    primGTdual = 1.0
  else:
    primGTdual = -1.0

  # OPTIMAL
  if pfeas_err <= 1e-4 and dfeas_err <= 1e-4 and ( \
       primGTdual*(pfeas_obj - dfeas_obj) <= 1e-4 or \
       primGTdual*(pfeas_obj - dfeas_obj) / max(1, abs(pfeas_obj), abs(dfeas_obj)) <= 1e-7 \
     ):
    return(('{0:.4E}'.format(pfeas_obj), ''))

  # PRIMAL INFEASIBLE
  if pinfeas_err <= 1e-4:
    return(('Primal infeasible', ''))

  # DUAL INFEASIBLE
  if dinfeas_err <= 1e-4:
    return(('Dual infeasible', ''))

  # PRIMAL FEASIBLE
  if pfeas_err <= 1e-4:
    return(('{0:.4E}'.format(pfeas_obj), 'v'))

  # BEST GUESS
  if pfeas_err <= pinfeas_err and pfeas_err <= dinfeas_err:
    return(('{0:.4E}'.format(pfeas_obj), 'a'))
  elif pinfeas_err <= dinfeas_err:
    return(('Primal infeasible', 'a'))
  else:
    return(('Dual infeasible', 'a'))


def primobjstat_integer(pfeas_obj, pfeas_err, pinfeas_err, dinfeas_err, claim):

  if all([err == float('inf') for err in [pfeas_err, pinfeas_err, dinfeas_err]]):
    return(('?', ''))

  # OPTIMAL
  if claim == 'INTEGER_OPTIMALITY' and pfeas_err <= 1e-4:
    return(('{0:.4E}'.format(pfeas_obj), ''))

  # INFEASIBLE
  if claim == 'INTEGER_INFEASIBILITY' or pinfeas_err <= 1e-4:
    return(('Infeasible', ''))

  # UNBOUND
  if dinfeas_err <= 1e-4:
    if pfeas_err <= 1e-4:
      return(('Unbound', ''))
    else:
      return(('Unbound if feasible', ''))

  # FEASIBLE
  if pfeas_err <= 1e-4:
    return(('{0:.4E}'.format(pfeas_obj), 'v'))

  # BEST GUESS
  if pfeas_err <= pinfeas_err and pfeas_err <= dinfeas_err:
    return(('{0:.4E}'.format(pfeas_obj), 'a'))
  elif pinfeas_err <= dinfeas_err:
    return(('Infeasible', 'a'))
  else:
    return(('Unbound if feasible', 'a'))


class CBFrunstat:

  def __init__(self, problem, id, solver, solution, soltime):
    self.problem = problem
    self.id = id
    self.solver = solver
    self.solution = solution
    self.soltime = soltime

  def primobjstatus(self):
    pfeas_obj = float('nan')
    pfeas_err = float('inf')
    pinfeas_err = float('inf')

    dfeas_obj = float('nan')
    dfeas_err = float('inf')
    dinfeas_err = float('inf')

    isminimize = (self.problem.obj.strip().upper() == 'MIN')

    if self.solution.primvar:
      cer = primvar_certificates(self.problem, self.solution)
      pfeas_obj = cer[0]
      pfeas_err = max(cer[1].values())

      # Any dual infeasibility certificate?
      if (isminimize and cer[0] < 0) or (not isminimize and cer[0] > 0):
        dinfeas_err = max(cer[2].values())
    
    if self.solution.dualvar:
      cer = dualvar_certificates(self.problem, self.solution)
      dfeas_obj = cer[0]
      dsol_err = max(cer[1].values())

      # Any primal infeasibility certificate?
      if (isminimize and cer[0] > 0) or (not isminimize and cer[0] < 0):
        pinfeas_err = max(cer[2].values())

    if self.problem.intvarnum >= 1:
      return primobjstat_integer(pfeas_obj, pfeas_err, pinfeas_err, dinfeas_err, self.solution.claim)
    else:
      return primobjstat_continuous(pfeas_obj, pfeas_err, pinfeas_err, dfeas_obj, dfeas_err, dinfeas_err, isminimize)
