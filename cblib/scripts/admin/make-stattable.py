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
