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


import csv, os, inspect, getopt, re
import filter
from data.CBFset import CBFset

class htmltable:
  def __init__(self):
    self.withcss = True
    self.pack = None
    self.oddrow = False

  def opentable(self):
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
    <div>''')

    if self.withcss:
      print('''\
      <style type="text/css" scoped>
      #cblib-reftable
      {
        margin: 45px;
        border-collapse: collapse;
      }
      #cblib-reftable th
      {
        font-family: "Lucida Sans Unicode", "Lucida Grande", Sans-Serif;
        font-size: 14px;
        font-weight: normal;
        text-align: left;
        padding: 0px 8px;
        color: #039;
      }
      #cblib-reftable td
      {
        font-family: "Lucida Sans Unicode", "Lucida Grande", Sans-Serif;
        font-size: 12px;
        font-weight: normal;
        padding: 4px 8px;
        color: #669;
      }
      #cblib-reftable div.nogroup
      {
        padding: 4px 8px;
        padding-top: 2px;
      }
      #cblib-reftable div.group
      {
        padding-bottom: 2px;
      }
      #cblib-reftable div.lborder
      {
        margin-right: 8px;
        border-bottom: 1px dotted black;
      }
      #cblib-reftable div.cborder
      {
        margin: 0px 8px;
        border-bottom: 1px dotted black;
      }
      #cblib-reftable div.rborder
      {
        margin-left: 8px;
        border-bottom: 1px dotted black;
      }
      #cblib-reftable .rsep
      {
        border-right: 1px solid #dde;
      }
      #cblib-reftable .strcol
      {
        text-align: left;
      }
      #cblib-reftable .numcol
      {
        text-align: right;
      }
      #cblib-reftable .oddrow
      {
        background: #e8edff;
      }
      </style>''')

    print('''\
      <table id="cblib-reftable">
        <thead>''')

    column = ['Pack','Description','Citations','Contributor','Instances']
    out = ''
    for i in range(0,len(column)):
      out += '<th><div class="group">' + str(column[i]) + '</div></th>'
    print(self.__row(out))

    print('''\
        </thead>
        <tbody>''')
    
  def closetable(self):
    print('''\
        </tbody>
      </table>
    </div>
  </body>
</html>''')

  def addrow(self, pack, instances, contributor, refkeys, text):
    self.oddrow = not self.oddrow
    
    out = ''

    if self.pack != pack:
      out += '<td><a href="download/' + str(pack) + '.tar.gz">' + str(pack) + '</a></td>'
      self.pack = pack
    else:
      out += '<td>...' + str(pack) + '</td>'

    out += '<td><div style="width:320px;">' + text + '</div></td>'

    if refkeys:
      out += '<td>' + ', '.join(['<a href="ref.bib.html#' + r + '">' + r + '</a>' for r in refkeys.replace(" ","").split(',')]) + '</td>'
    else:
      out += '<td>None</td>'

    out += '<td>' + contributor + '</td>'
    out += '<td><div style="max-height:120px; overflow:auto"><span style="white-space: nowrap;">' + '</span>, <span style="white-space: nowrap;">'.join(instances) + '</span></div></td>'
      
    print(self.__row(out))
    
  def __row(self, value):
    if self.oddrow:
      return('          <tr class="oddrow">' + value + '</tr>')
    else:
      return('          <tr>' + value + '</tr>')

class textable:
  def opentable(self):
    print('% Requires package "longtable" and "booktabs"')
    print('\\begin{scriptsize}')
    print('\\begin{longtable}{>{\\raggedright\\arraybackslash}lp{0.7\\linewidth}r}')
    print('\\toprule')
    print(' & '.join(['{\\bf Packs}', '{\\bf Origin and description}', '{\\bf Instances}']) + '\\\\')
    print('\\midrule')

  def closetable(self):
    print('\\bottomrule')
    print('\\hiderowcolors')
    print('\\caption{\\cblibreftablecaption}')
    print('\\label{cblib:reftabel}')
    print('\\end{longtable}')
    print('\\end{scriptsize}')

  def addrow(self, pack, instances, contributor, refkeys, text):
    print(pack.replace('_','\_'))
    print(' & ' + ''.join(['\\textsf{', \
                   ', '.join([''.join(['\\citet{', x, '}']) for x in refkeys.replace(' ','').split(',')]), \
                   '.}\\newline']))
    print(text)
    print(' & ' + str(len(instances)))
    print('\\\\')
    print('%')


def files_add(pack, instance, filemap):
  if pack in filemap:
    filemap[pack].add(instance)
  else:
    filemap[pack] = set([instance])


def reftable(out, filtexpr, setexpr):
  # Find the directory of this script
  scriptdir = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
  rootdir = os.path.join(scriptdir,'..','..')

  # Default value
  if setexpr == None:
    setexpr = os.path.realpath(os.path.abspath(os.path.join(rootdir,'instances','cbf')))

  # Define files
  filemap = dict()
  cbfset = CBFset()
  cbfset.read(setexpr)

  filter.filter(filtexpr, None, cbfset, lambda x: files_add(cbfset.getpack(x,cbfset.rootdir), cbfset.getinstance(x), filemap))
  
  # Define sorting
  convert = lambda text: int(text) if text.isdigit() else text
  alphanum_key = lambda key: [ convert(c) for c in re.split('([0-9]+)', key) ]
  
  out.opentable()

  csvpath = os.path.join(rootdir,'instances','ref.csv')
  csvfile = open(csvpath, 'rt')
  try:
    csvdialect = csv.Sniffer().sniff(csvfile.read(), ';\t')
    csvfile.seek(0)
    
    csvreader = csv.reader(csvfile, csvdialect, quotechar='"')
    next(csvreader)
    for row in csvreader:
      if row[0] in filemap:
        mylst = list(set(row[1].split(', ')) & filemap[row[0]])
        if len(mylst) >= 1:
          mylst.sort(key=alphanum_key)
          out.addrow(row[0], mylst, row[2], row[3], row[4])

  except Exception as e:
    print(str(e))
  finally:
    csvfile.close()

  out.closetable()


if __name__ == "__main__":
  try:
    # Verify command line arguments
    opts, args = getopt.gnu_getopt(sys.argv[1:], "s:o:", "filter=")
    if len(args) >= 1:
      raise Exception('Incorrect usage!')
  except Exception as e:
    print(str(e))
    print(''.join(['Try all mixed-integer second order cone instances:', '\n',
          '  python ', sys.argv[0], ' --filter="||int|| and ||cones|so|| and not ||psdcones||"', '\n']))
    sys.exit(2)

  filtexpr = ""
  setexpr = None
  out = htmltable()

  for opt, arg in opts:
    if opt == '-s':
      setexpr = arg
    elif opt == '-o':
      if arg == 'latex':
        out = textable()
      elif arg == 'html':
        out = htmltable()
      else:
        raise Exception('Backend not recognized!')
    elif opt == "--filter":
      filtexpr = arg

  try:
    reftable(out, filtexpr, setexpr)
  except Exception as e:
    print(str(e))

