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


import os, sys, inspect, getopt, csv, re
from data.CBFset import CBFset
from summary import summary
import filter

def make_stattable(statfile, cbfset, filtexpr):

  # Find the directory of this script
  scriptdir = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
  rootdir = os.path.join(scriptdir,'..','..')

  # Open status data file
  csvfile = open(statfile, 'wt')

  try:
    csvwriter = csv.writer(csvfile, delimiter=';', quotechar='"')

    # Write header
    statdictnames = ['pack','name','psense','pcer','perr','pobj','dcer','derr','dobj','claim']
    header = ["PACK","NAME","PRIMAL SENSE","PRIMAL CERTIFICATE","ERROR","OBJECTIVE","DUAL CERTIFICATE","ERROR","OBJECTIVE","CLAIM (INTEGER_OPTIMALITY or INTEGER_INFEASIBILITY)"]
    csvwriter.writerow(header)

    # Write empty instance status data
    filter.filter(filtexpr, "[||path||, 'MIN' if ||minimize|| else 'MAX']", cbfset, lambda x: csvwriter.writerow([cbfset.getpack(x[0],cbfset.rootdir), cbfset.getinstance(x[0]), x[1], "", "", "", "", "", "", ""]))

  finally:
    csvfile.close()


if __name__ == "__main__":
  try:
    # Verify command line arguments
    opts, args = getopt.gnu_getopt(sys.argv[1:], "s:f", "filter=")
    if len(args) < 1:
      raise Exception('Incorrect usage, exactly one stat file should be specified!')
  except Exception as e:
    print(str(e))
    print(''.join(['Try all mixed-integer second order cone instances:', '\n',
          '  python ', sys.argv[0], ' <STATFILE> --filter="||int|| and ||cones|so|| and not ||psdcones||"', '\n']))
    sys.exit(2)

  filelist = False
  filtexpr = ""
  setexpr = None
  statfile = args[0]

  for opt, arg in opts:
    if opt == "-f":
      filelist = True
    elif opt == "-s":
      setexpr = arg
    elif opt == "--filter":
      filtexpr = arg

  # Prepare set of instances
  if setexpr is not None:
    cbfset = CBFset()
    cbfset.read(setexpr)
  elif filelist:
    cbfset = CBFset()
    cbfset.readfilelist(args[1:])
  else:
    cbfset = filter.defaultcbfset()

  probfiles = cbfset.cbffiles

  try:
    make_stattable(statfile, cbfset, filtexpr)
  except Exception as e:
    print(str(e))
