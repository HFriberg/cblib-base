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

import os, sys, inspect, getopt
import filter
from scipy import dot,sparse,array
from data.CBFdata import CBFdata
from data.CBFset import CBFset
from data.CBFsolution import CBFsolution
#from data.CBFrunstat import CBFrunstat

def summary(prob, sol, printer):

  # Find the directory of this script
  scriptdir = os.path.split(inspect.getfile( inspect.currentframe() ))[0]

  # Import all cones from 'dist'
  distdict = None
  for dirpath, dirnames, filenames in os.walk(os.path.realpath(os.path.abspath(os.path.join(scriptdir, 'dist'))) ):
    distdict = dict([(f, __import__('dist.' + f, fromlist=f)) for f in [os.path.splitext(f)[0] for f in filenames if f[:1] != '_']])
    break

  # Ensure that problem and solution is loaded
  if isinstance(prob, str):
    probfile = prob
    prob = next(CBFdata(probfile).iterator())

  if isinstance(sol, str):
    solfile = sol
    sol = CBFsolution()
    sol.readsol(prob, solfile)

  # Validate information
  priminfo = (len(sol.primvar) + len(sol.primpsdvar) != 0)
  dualinfo = (len(sol.dualvar) + len(sol.dualpsdvar) != 0)

  if priminfo and (
       len(sol.primvar) != prob.varnum or \
       len(sol.primpsdvar) != prob.psdvarnum or \
       [len(x) for x in sol.primpsdvar] != prob.psdvardim) or \
     dualinfo and (
       len(sol.dualvar) != prob.mapnum or \
       len(sol.dualpsdvar) != prob.psdmapnum or \
       [len(x) for x in sol.dualpsdvar] != prob.psdmapdim \
     ):
    printer('Mismatch between problem and solution')
    return

  # Compute summary
  if priminfo:
    cer = primvar_certificates(prob, sol, distdict)
    pobj = cer[0]
    psol_err = cer[1]
    pray_err = cer[2]

  if dualinfo:
    cer = dualvar_certificates(prob, sol, distdict)
    dobj = cer[0]
    dsol_err = cer[1]
    dray_err = cer[2]

  # Report summary
  out = dict([('prob', prob)])

  comment = ''
  if prob.intvarnum >= 1:
    comment = ' (for continuous relaxation)'

  if sol.claim is not None:
    out['claim'] = sol.claim
    printer('CLAIM')
    printer('  ' + sol.claim)

  if priminfo:
    out['psol'] = (pobj, psol_err)
    printer('PRIMAL SOLUTION')
    printer('  ' + str(pobj))
    printer('  ' + str(psol_err))

  if dualinfo:
    if (prob.obj == 'MIN' and dobj > 0) or (prob.obj == 'MAX' and dobj < 0):
      out['pinfeascer'] = (dobj, dray_err)
      printer('PRIMAL INFEASIBILITY CERTIFICATE' + comment)
      printer('  ' + str(dobj))
      printer('  ' + str(dray_err))

  if dualinfo:
    out['dsol'] = (dobj, dsol_err)
    printer('DUAL SOLUTION' + comment)
    printer('  ' + str(dobj))
    printer('  ' + str(dsol_err))

  if priminfo:
    if (prob.obj == 'MIN' and pobj < 0) or (prob.obj == 'MAX' and pobj > 0):
      out['dinfeascer'] = (pobj, pray_err)
      printer('DUAL INFEASIBILITY CERTIFICATE' + comment)
      printer('  ' + str(pobj))
      printer('  ' + str(pray_err))

#  rs = CBFrunstat('')
#  rs.time = 0
#  rs.problem = prob
#  rs.solution = sol
#  printer(rs.report())

  return(out)


def primvar_certificates(prob, sol, distdict):
  psol_err = dict()
  pray_err = dict()

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


def dualvar_certificates(prob, sol, distdict):
  dsol_err = dict()
  dray_err = dict()

  var_activities = array(sol.dualvar)
  vardomainfactor = 1
  mapdomainfactor = 1

  # Objective and variable activities are where solutions and rays agree
  dobj = prob.objbval
  for i in range(prob.bnnz):
    dobj -= prob.bval[i] * var_activities[prob.bsubi[i]]

  if prob.obj == 'MAX':
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

  if prob.obj == 'MIN':
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



if __name__ == "__main__":
  try:
    # Verify command line arguments
    opts, args = getopt.gnu_getopt(sys.argv[1:], "fs:", "filter=")
  except Exception as e:
    print(str(e))
    scriptdir = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
    rootdir = os.path.join(scriptdir,'..')
    print(''.join([
          'Incorrect usage, try summary of \"qssp30.cbf\":', '\n',
          '  python ', sys.argv[0], ' -f ', os.path.realpath(os.path.abspath(os.path.join(rootdir,'instances','cbf','qssp30.cbf'))) ]))
    sys.exit(2)

  filelist = False
  setexpr = None
  filtexpr = None

  for opt, arg in opts:
    if opt == "-f":
      filelist = True
    elif opt == "-s":
      setexpr = arg
    elif opt == "--filter":
      filtexpr = arg

  try:
    # Load problem and solution files
    if setexpr is not None:
      cbfset = CBFset()
      cbfset.read(setexpr)
    elif filelist:
      cbfset = CBFset()
      cbfset.readfilelist(args)
    else:
      cbfset = filter.defaultcbfset()
   
    probfiles = cbfset.cbffiles
    solfiles = cbfset.solfiles

    # Apply filter if any
    if filtexpr:
      probfilter = list()
      filter.filter("", ''.join(["bool(", filtexpr, ")"]), cbfset, lambda x: probfilter.append(x))
      probfiles = [probfiles[i] for i in range(len(probfilter)) if probfilter[i]]
      solfiles = [solfiles[i] for i in range(len(probfilter)) if probfilter[i]]

    # Extract solution summaries
    for (probfile, solfile) in zip(probfiles, solfiles):
      sys.stdout.write('\n' + solfile + '\n')
      summary(probfile, solfile, lambda x: sys.stdout.write(str(x)+'\n'))

  except Exception as e:
    print(str(e))

