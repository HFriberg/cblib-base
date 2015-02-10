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

 
from data.BIBdata import BIBdata

def bibtable(bibpath):
  # Find the directory of this script
  scriptdir = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
  rootdir = os.path.join(scriptdir,'..','..')

  # Default value
  if bibpath == None:
    bibpath = os.path.realpath(os.path.abspath(os.path.join(rootdir,'instances','ref.bib')))

  bibdata = BIBdata(bibpath)

  print('''\
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">

  <head>
    <meta http-equiv="content-type" content="text/html;charset=UTF-8" />
    <meta name="author" content="Ambros Gleixner" />
    <meta name="description" content="Conic Benchmark Library" />
    <meta name="keywords" content="CBLIB" />
    <meta name="robots" content="index|follow" />
    <link rel="stylesheet" type="text/css" href="cblib.css" />
    <title>CBLIB - Conic Benchmark Library</title>
  </head>

  <body>
    <div id="content">''')
  
  keys = bibdata.refs.keys()
  keys.sort()
  for key in keys:
    print('      <p name="' + key + '" id="' + key + '">' + bibdata.get(key).replace('\n','<br/>') + '</p>')

  print('''\
    </div>
  </body>
</html>''')
    
if __name__ == "__main__":
  try:
    # Verify command line arguments
    if len(sys.argv) >= 3:
      raise Exception('Incorrect usage, only one reference should be specified!')
  except Exception as e:
    print(str(e))
    sys.exit(2)

  bibpath = None
  
  if len(sys.argv) == 2:
    bibpath = sys.argv[1]
    
  try:
    bibtable(bibpath)
  except Exception as e:
    print(str(e))

