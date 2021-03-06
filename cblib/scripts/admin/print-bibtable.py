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

