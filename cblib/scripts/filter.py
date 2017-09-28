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


import os, sys, inspect, re, getopt
import data.CBFdata, data.CBFset


def defaultcbfset():

  # Find the directory of this script
  scriptdir = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
  rootdir   = os.path.join(scriptdir,'..')
  
  if os.path.exists(os.path.join(rootdir,'instances','cbf')):
    cbfset = data.CBFset.CBFset()
    cbfset.read(os.path.join(rootdir,'instances','cbf'))
    return(cbfset)
  else:
    raise Exception(''.join(['Could not find default location: ', os.path.realpath(os.path.abspath(os.path.join(rootdir,'instances','cbf')))]))


def filter(expr, printexpr, cbfset, printer):
  hasprintexpr = (printexpr is not None)

  # Find the directory of this script
  scriptdir = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
  rootdir   = os.path.join(scriptdir,'..')

  # Default value
  if cbfset is None:
    cbfset = defaultcbfset()

  # Validate expressions
  expr = expr.split('||')
  if len(expr)%2 != 1:
    raise Exception('Unbalanced use of filter identifier \'||\'')
  if hasprintexpr:
    printexpr = printexpr.split('||')
    if len(printexpr)%2 != 1:
      raise Exception('Unbalanced use of filter identifier \'||\' in print expression')

  # Load expression filters
  usingfilters = False
  filt = [0]*len(expr)
  for i in range(1,len(expr),2):
    expr[i] = expr[i].split('|')
    filt[i] = __import__('filters.' + expr[i][0], fromlist=expr[i])
    usingfilters = True
  if hasprintexpr:
    printfilt = [0]*len(printexpr)
    for i in range(1,len(printexpr),2):
      printexpr[i] = printexpr[i].split('|')
      printfilt[i] = __import__('filters.' + printexpr[i][0], fromlist=printexpr[i])
      usingfilters = True

  # Compile expression commands
  cmd = expr[:] #memcopy
  for i in range(1,len(expr),2):
    cmd[i] = ''.join(['filt[', str(i), '].getval(prob, *expr[', str(i) ,'][1:])'])
  cmd = ''.join(cmd)
  if hasprintexpr:
    printcmd = printexpr[:] #memcopy
    for i in range(1,len(printexpr),2):
      printcmd[i] = ''.join(['printfilt[', str(i), '].getval(prob, *printexpr[', str(i), '][1:])'])
    printcmd = ''.join(printcmd)

  # What keywords do expressions require from data parsing?
  keyquery = set()
  for i in range(1,len(expr),2):
    keyquery |= set(filt[i].keyquery(*expr[i][1:]))
  if hasprintexpr:
    for i in range(1,len(printexpr),2):
      keyquery |= set(printfilt[i].keyquery(*printexpr[i][1:]))

  # Search files in benchmark library
  for f in cbfset.cbffiles:
    if usingfilters:
      prob = next(data.CBFdata.CBFdata(f, keyquery).iterator())

    if not cmd or bool(eval(cmd)) == True:
      if hasprintexpr:
        printer(eval(printcmd))
      else:
        printer(f)


def csvformatprinter(cbfset, x):
  if csvformatprinter.pack != cbfset.getpack(x, cbfset.rootdir):
    csvformatprinter.pack = cbfset.getpack(x, cbfset.rootdir)
    sys.stdout.write("\n" + csvformatprinter.pack + ";")
  else:
    sys.stdout.write(", ")
  
  sys.stdout.write(cbfset.getinstance(x))
  sys.stdout.flush()

csvformatprinter.pack = None


if __name__ == "__main__":

  try:
    # Verify command line arguments
    opts, args = getopt.gnu_getopt(sys.argv[1:], "s:x:f")
  except Exception as e:
    print(str(e))
    print(''.join(['Try all mixed-integer second order cone instances:', '\n',
          '  python ', sys.argv[0], ' "||int|| and ||cones|so|| and not ||psdcones||"', '\n']))
    sys.exit(2)

  filelist = False
  filtexpr = args[0]  
  setexpr = None
  printexpr = None
  printer = lambda x: sys.stdout.write(str(x) + '\n')
  
  for opt, arg in opts:
    if opt == "-f":
      filelist = True
    elif opt == "-s":
      setexpr = arg
    elif opt == "-x":
      printexpr = arg

  # Load problem and solution files
  if filelist:
    cbfset = data.CBFset.CBFset()
    cbfset.readfilelist(args[1:])
  elif setexpr is not None:
    cbfset = data.CBFset.CBFset()
    cbfset.read(setexpr)
  else:
    cbfset = defaultcbfset()

  # Default print: Header
  if printexpr is None:
    printer = lambda x: csvformatprinter(cbfset, x)
    sys.stdout.write("PACK;INSTANCES")

  try:
    filter(filtexpr, printexpr, cbfset, printer)
  except Exception as e:
    print(str(e))

  # Default print: Footer
  if printexpr is None:
    sys.stdout.write('\n')

