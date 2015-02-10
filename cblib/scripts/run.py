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
from data.CBFdata import CBFdata
from data.CBFset import CBFset

def run(solver, probfile, solfile, paramfile, printer):

  ss = None
  try:
    timebefore = time.time()
    for (i,pp) in enumerate(CBFdata(probfile).iterator()):
      print('Read in: %.2f seconds' % (time.time() - timebefore))

      timebefore = time.time()
      solver.read(probfile, paramfile, pp)
      print('Moved to solver in: %.2f seconds' % (time.time() - timebefore))

      pptime = Timer(solver.optimize).timeit(number=1)
      ppsol = solver.getsolution()

      # Write to file
      if i == 0:
        if os.path.dirname(solfile) and not os.path.exists(os.path.dirname(solfile)):
          os.makedirs(os.path.dirname(solfile))
        ss = open(solfile, 'w')
      else:
        ss.write('CHANGE\n\n')
      ppsol.printsol( lambda x: ss.write(str(x) + '\n') )

      # Write to printer
      printer('\n' + pp.name + '[' + str(i) + ']: ' + str(pptime) + 's ' + solfile)
      summary(pp, ppsol, printer)

  finally:
    if ss is not None:
      ss.close()



if __name__ == "__main__":

  try:
    # Verify command line arguments
    opts, args = getopt.gnu_getopt(sys.argv[1:], "fs:p:", "filter=")
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

  for opt, arg in opts:
    if opt == "-f":
      filelist = True
    elif opt == "-s":
      setexpr = arg
    elif opt == "--filter":
      filtexpr = arg
    elif opt == "-p":
      paramfile = arg

  try:
    # Load solver
    solver = __import__('solvers.' + sys.argv[1], fromlist=sys.argv[1]).solver(sys.stdout.write)

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

    # Start runs
    for (probfile, solfile) in zip(probfiles, solfiles):
      run(solver, probfile, solfile, paramfile, lambda x: sys.stdout.write(str(x)+'\n'))

  except Exception as e:
    print(str(e))
