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


import os, sys, inspect, tarfile, glob, stat, getopt
from data.CBFset import CBFset
from filter import filter

def addwritepermission(tarinfo):
  tarinfo.mode = tarinfo.mode | stat.S_IWRITE
  return tarinfo

def pack(packname, filtexpr, setexpr, packall):
  # tarfile 'filter' requires v2.7
  if sys.version_info < (2,7):
    raise Exception('Python 2.7 or later required..')

  # Get the root directory of cblib
  scriptdir = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
  rootdir = os.path.join(scriptdir,'..','..')

  if not packall and setexpr != None:
    if os.path.isfile(setexpr):
      rootdir = os.path.dirname(setexpr)
    else:
      rootdir = setexpr

  # Find all instances
  files = list()
  cbfset = CBFset()
  cbfset.read(setexpr)
  filter(filtexpr, None, cbfset, lambda x: files.append(x))

  if packall:
    # Find all instance information
    files = files + glob.glob(os.path.join(rootdir,'instances','*.csv'))
    files = files + glob.glob(os.path.join(rootdir,'instances','*.bib'))
  
    # Find all source files from 'tools'
    files = files + glob.glob(os.path.join(rootdir,'tools','*.c'))
    files = files + glob.glob(os.path.join(rootdir,'tools','*.h'))
    files = files + glob.glob(os.path.join(rootdir,'tools','Makefile.*'))
  
    # Find all documents from 'docs'
    files = files + glob.glob(os.path.join(rootdir,'docs','*.pdf'))
  
    # Find all python files from 'scripts'
    files = files + glob.glob(os.path.join(rootdir,'scripts','*.py'))
    files = files + glob.glob(os.path.join(rootdir,'scripts','admin','*.py'))
    files = files + glob.glob(os.path.join(rootdir,'scripts','data','*.py'))
    files = files + glob.glob(os.path.join(rootdir,'scripts','dist','*.py'))
    files = files + glob.glob(os.path.join(rootdir,'scripts','filters','*.py'))
    files = files + glob.glob(os.path.join(rootdir,'scripts','solvers','*.py'))
  
    # Find all other important files
    files.append(os.path.join(rootdir,'README'))
    files.append(os.path.join(rootdir,'instances','cbf','README'))

  # Create compressed tar file
  print('Writing '+packname+'.tar.gz')
  tar = tarfile.open(os.path.join(scriptdir,packname+'.tar.gz'), 'w:gz')
  for f in files:
    extractname = os.path.join(packname, os.path.relpath(f, rootdir))
    print(extractname)
    tar.add(f, arcname=extractname, filter=addwritepermission)
  tar.close()

if __name__ == "__main__":

  try:
    # Verify command line arguments
    opts, args = getopt.gnu_getopt(sys.argv[1:], "n:s:a", "filter=")
    if len(args) >= 1:
      raise Exception('Incorrect usage!')
  except Exception as e:
    print(str(e))
    raise Exception(''.join([
          'Incorrect usage, try all instances', '\n',
          '  python ', sys.argv[0], ' cblib', '\n',
          'or try all mixed-integer second order cone instances:', '\n',
          '  python ', sys.argv[0], ' cblib-misoco --filter="||int|| and ||cones|so|| and not ||psdcones||"']))
    sys.exit(2)

  packname = None
  filtexpr = ""
  setexpr = None
  packall = False
  
  for opt, arg in opts:
    if opt == '-n':
      packname = arg
    elif opt == "-s":
      setexpr = arg
    elif opt == "-a":
      packall = True
    elif opt == "--filter":
      filtexpr = arg

  try:
    if not packname:
      if setexpr and os.path.exists(setexpr) and not os.path.isfile(setexpr):
        packname = os.path.basename(setexpr)
        if not packname:
          packname = os.path.basename(os.path.dirname(setexpr))
      else:
        raise Exception('No pack name specified!')

    print(setexpr)

    pack(packname, filtexpr, setexpr, packall)
  except Exception as e:
    print(str(e))
