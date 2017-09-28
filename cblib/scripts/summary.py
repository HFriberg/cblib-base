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


import os, sys, inspect, getopt
import filter
from scipy import dot,sparse,array
from data.CBFdata import CBFdata
from data.CBFset import CBFset
from data.CBFsolution import CBFsolution
from data.CBFrunstat import primvar_certificates, dualvar_certificates

def summary(prob, sol, printer):

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
  isminimize = (prob.obj.strip().upper() == 'MIN')

  if priminfo:
    cer = primvar_certificates(prob, sol)
    pobj = cer[0]
    psol_err = cer[1]
    pray_err = cer[2]

  if dualinfo:
    cer = dualvar_certificates(prob, sol)
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
    if (isminimize and dobj > 0) or (not isminimize and dobj < 0):
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
    if (isminimize and pobj < 0) or (not isminimize and pobj > 0):
      out['dinfeascer'] = (pobj, pray_err)
      printer('DUAL INFEASIBILITY CERTIFICATE' + comment)
      printer('  ' + str(pobj))
      printer('  ' + str(pray_err))

  return(out)


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
   
    # Apply filter if any
    if filtexpr:
      cbfset.filter(filtexpr)

    # Extract solution summaries
    for (probfile, solfile) in zip(cbfset.cbffiles, cbfset.solfiles):
      sys.stdout.write('\n' + solfile + '\n')
      summary(probfile, solfile, lambda x: sys.stdout.write(str(x)+'\n'))

  except Exception as e:
    print(str(e))

