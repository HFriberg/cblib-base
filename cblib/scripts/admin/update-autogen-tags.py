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


import re, csv, textwrap, getopt, os, sys, inspect, gzip, shutil
import filter
from data.CBFset import CBFset
from data.BIBdata import BIBdata

def autogen_tags_writer(name, csvdata, bibdata):
  row = None
  for r in csvdata:
    if r[1].find(name) != -1:
      row = r
      break
      
  if not row:
    raise Exception(''.join(['Instance named "', name, '" not found in ref.csv.\n']))

  wrapwidth = 78
  text = list()

  # Get contributor
  text.append(''.join(['Contributor: ', row[2]]))

  # Get description
  text.extend(textwrap.wrap(row[4], wrapwidth))

  # Get references
  for key in row[3].split(','):
    key = key.strip()
    text.append('')
    text.extend(bibdata.get(key).split('\n'))

  return '# ' + '\n# '.join(text) + '\n'


def update_file(file, tempfile, tagpattern, csvdata, bibdata):
  [instancename, filetype] = os.path.splitext(os.path.basename(file))
  print(file)

  # Open file
  if filetype.lower() == '.gz':
    instancename = os.path.splitext(instancename)[0]
    f = gzip.open(file,'r')
  else:
    f = open(file,'r')

  try:
    # Prepare metadata replacement
    autogen_tag = '#<CBLIB:AUTOGEN>\n' + autogen_tags_writer(instancename, csvdata, bibdata) + '#</CBLIB:AUTOGEN>\n'

    buf = f.readline()
    metadata = buf
    while (buf != '#</CBLIB:AUTOGEN>\n' and buf != 'VER\n'):
      buf = f.readline()
      metadata += buf

    metadata = tagpattern.sub('', metadata, count=1)

    # Copy file with new metadata
    if filetype.lower() == '.gz':
      f2 = gzip.open(tempfile,'wb')
    else:
      f2 = open(tempfile,'wb')

    try:
      f2.write(autogen_tag)
      f2.write(metadata)
      shutil.copyfileobj(f,f2)
    finally:
      f2.close()

  finally:
    f.close()

  # Replace original file
  os.rename(tempfile, file)

def update_autogen_tags(cbfset, filtexpr):
  # Find the directory of this script
  scriptfile = inspect.getfile( inspect.currentframe() )
  scriptdir = os.path.split(scriptfile)[0]
  rootdir = os.path.join(scriptdir,'..','..')

  # Read 'bibdata'
  bibpath = os.path.join(rootdir,'instances','ref.bib')
  bibdata = BIBdata(bibpath)

  # Read 'csvdata'
  csvpath = os.path.join(rootdir,'instances','ref.csv')
  csvfile = open(csvpath, 'rt')
  try:
    csvdialect = csv.Sniffer().sniff(csvfile.read(), ';\t')
    csvfile.seek(0)
    
    csvreader = csv.reader(csvfile, csvdialect, quotechar='"')
    next(csvreader)
    csvdata = list(csvreader)
  finally:
    csvfile.close()

  # Define 'tagpattern':
  #  (1) Pattern starts with file or after newline
  #  (2) Anything may follow except a newline
  #  (3) Then comes <CBLIB:AUTOGEN>
  #  (4) Anything may follow
  #  (5) Then comes </CBLIB:AUTOGEN>
  #  (6) Anything may follow except a newline
  #  (7) Pattern ends after newline
  tagpattern = re.compile('^.*<CBLIB:AUTOGEN>[\s\S]*</CBLIB:AUTOGEN>.*\\n', re.MULTILINE)

  # Update instances
  filter.filter(filtexpr, None, cbfset, lambda x: update_file(x, scriptfile+'.temp', tagpattern, csvdata, bibdata))


if __name__ == "__main__":
  try:
    # Verify command line arguments
    opts, args = getopt.gnu_getopt(sys.argv[1:], "fs:", "filter=")
  except Exception as e:
    print(str(e))
    print(''.join(['Try all mixed-integer second order cone instances:', '\n',
          '  python ', sys.argv[0], ' <STATFILE> --filter="||int|| and ||cones|so|| and not ||psdcones||"', '\n']))
    sys.exit(2)

  filtexpr = ""
  filelist = False
  setexpr = None

  for opt, arg in opts:
    if opt == '-s':
      setexpr = arg
    elif opt == '-f':
      filelist = True
    elif opt == "--filter":
      filtexpr = arg

  # Prepare set of instances
  if setexpr is not None:
    cbfset = CBFset()
    cbfset.read(setexpr)
  elif filelist:
    cbfset = CBFset()
    cbfset.readfilelist(args)
  else:
    cbfset = filter.defaultcbfset()

  try:
    update_autogen_tags(cbfset, filtexpr)
  except Exception as e:
    print(str(e))
