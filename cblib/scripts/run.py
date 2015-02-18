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


# Direct execution requires top level directory on python path
if __name__ == "__main__":
  import os, sys, inspect
  scriptdir = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
  packagedir = os.path.realpath(os.path.abspath(os.path.join(scriptdir,'..')))
  if packagedir not in sys.path:
    sys.path.insert(0, packagedir)


import os, sys, inspect, getopt, time
import filter
from timeit import Timer
from summary import summary
from data.CBFrunstat import CBFrunstat
from data.CBFdata import CBFdata
from data.CBFset import CBFset

def run(solver, probfile, solfile, paramfile, printer, callback):

  ss = None
  try:
    timebefore = time.time()
    for (i,pp) in enumerate(CBFdata(probfile).iterator()):
      if printer:
        printer('File read: %.2f seconds' % (time.time() - timebefore))

      timebefore = time.time()
      solver.read(probfile, paramfile, pp)
      if printer:
        printer('Solver read: %.2f seconds' % (time.time() - timebefore))

      pptime = Timer(solver.optimize).timeit(number=1)
      if printer:
        solver.report()

      # Write to file
      ppsol = solver.getsolution()

      if i == 0:
        if os.path.dirname(solfile) and not os.path.exists(os.path.dirname(solfile)):
          os.makedirs(os.path.dirname(solfile))
        ss = open(solfile, 'w')
      else:
        ss.write('CHANGE\n\n')

      ppsol.printsol( lambda x: ss.write(str(x) + '\n') )

      # Write to printer
      if printer:
        printer('\n' + pp.name + '[' + str(i) + ']: ' + str(pptime) + 's ' + solfile)
        summary(pp, ppsol, printer)

      if callback:
        callback(CBFrunstat(pp, i, solver, ppsol, pptime))

      timebefore = time.time()

  finally:
    if ss is not None:
      ss.close()


def benchmark_callback(pack, rs):  
  primobjstat = rs.primobjstatus()
  if primobjstat[1]:
    primobjstat[1] = '(' + primobjstat[1] + ')'
  sys.stdout.write(';'.join([pack, rs.problem.name, str(rs.id), ''.join(primobjstat), '{0:.4E}'.format(rs.soltime), str(rs.solver.getsizeoftree())]) + '\n')
  


if __name__ == "__main__":

  try:
    # Verify command line arguments
    opts, args = getopt.gnu_getopt(sys.argv[1:], "fs:p:", ["filter=","benchmark"])
    if len(args) == 0:
      raise Exception('ERROR: No solver specified!')
  except Exception as e:
    print(str(e))
    scriptdir = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
    rootdir = os.path.join(scriptdir,'..')
    print(''.join([
          'Incorrect usage, try solving \"qssp30.cbf\" using MOSEK:', '\n',
          '  python ', sys.argv[0], ' runmosek -f ', os.path.realpath(os.path.abspath(os.path.join(rootdir,'instances','cbf','qssp30.cbf'))) ]))
    sys.exit(2)

  solvermodule = args[0]
  filelist = False
  setexpr = None
  filtexpr = None
  paramfile = None
  benchmark = False

  for opt, arg in opts:
    if opt == "-f":
      filelist = True
    elif opt == "-s":
      setexpr = arg
    elif opt == "--filter":
      filtexpr = arg
    elif opt == "-p":
      paramfile = arg
    elif opt == "--benchmark":
      benchmark = True

  try:
    # Setup output formatting
    if benchmark:
      rawprinter = None
      printer    = None
      callback   = lambda rs: benchmark_callback(cbfset.getpack(rs.problem.file, cbfset.rootdir), rs)

      # Default print: header
      sys.stdout.write(';'.join(['PACK','NAME','ID','STATUS','TIME','TREE SIZE']) + '\n')
      
    else:
      rawprinter = sys.stdout.write
      printer    = lambda x: rawprinter(str(x) + '\n')
      callback   = None
      
    # Load solver
    solver = __import__('solvers.' + sys.argv[1], fromlist=sys.argv[1]).solver(rawprinter)

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

    # Start runs
    for (probfile, solfile) in zip(cbfset.cbffiles, cbfset.solfiles):
      run(solver, probfile, solfile, paramfile, printer, callback)

  except Exception as e:
    print(str(e))
