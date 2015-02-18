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


import os, inspect, getopt, csv
from filter import filter, defaultcbfset
from data.CBFset import CBFset
from data.CBFrunstat import primobjstat_integer, primobjstat_continuous

statdictnames = ['pack','name','psense','pcer','perr','pobj','dcer','derr','dobj','claim']

class textable:
  def __init__(self):
    self.pack = ''
    self.statdict = None
    self.objstatmark = {'': '\\hphantom{\\tnote{a}}', 'v': '\\tnote{v}', 'a': '\\tnote{a}'}

  def opentable(self):
    print('''\
% Requires package "longtable", "booktabs" and "threeparttable"
\\setlength{\\LTleft}{-20cm plus -1fill}
\\setlength{\\LTright}{\\LTleft}
\\newcolumntype{R}{>{\\raggedleft\\arraybackslash}p{7.8em}}
\\begin{scriptsize}
\\begin{ThreePartTable}
\\begin{TableNotes}
\\item[a(currancy)] Infeasibility meassures exceed $10^{-4}$ on some primitive cones or integer requirements (points not normalized).
\item[v(alue)] Objective neither claimed by a solver to be within an absolute and relative gap of 0.0 from optimality (mixed-integer case), nor certified to be within an absolute gap of $10^{-4}$ or relative gap of $10^{-7}$ from optimality (continuous case).
\\end{TableNotes}
\\begin{longtable}{l|rrr|rR|rrrr|r}
\\toprule
\\multicolumn{1}{l}{} & \\multicolumn{3}{c}{\\bf Size} & \\multicolumn{2}{c}{\\bf Conic domains} & \\multicolumn{2}{c}{\\bf Binary} & \\multicolumn{2}{c}{\\bf Integer} & \\multicolumn{1}{c}{\\bf Status} \\\\
\\cmidrule(r){2-4} \\cmidrule(lr){5-6} \\cmidrule(lr){7-10} \\cmidrule(l){11-11}
\\multicolumn{1}{l}{\\bf Instances} & \\multicolumn{1}{r}{var} & \\multicolumn{1}{r}{map} & \\multicolumn{1}{r}{nnz} & \\multicolumn{1}{r}{lin} & \\multicolumn{1}{r}{so} & \\multicolumn{1}{r}{lin} & \\multicolumn{1}{r}{so} & \\multicolumn{1}{r}{lin} & \\multicolumn{1}{r}{so} & \\multicolumn{1}{r}{obj} \\\\''')

  def closetable(self):
    print('''\
\\bottomrule
\\hiderowcolors
\insertTableNotes\\\\
\\hiderowcolors
\\caption{\\cblibstattablecaption}
\\label{cblib:stattable}
\\end{longtable}
\\end{ThreePartTable}
\\end{scriptsize}''')

  def addrow(self, data):
    column = ['']*11

    # Pack
    if data[0] in self.statdict:
      pack = self.statdict[data[0]]['pack']
    else:
      pack = ''

    if self.pack != pack:
      column[0] = '\\hline\\textbf{' + pack + '}'
      print('\n & '.join(column))
      print('\\\\')
      self.pack = pack

    # Name
    column[0] = data[0].replace('_','\_')

    # var, map, nnz
    column[1] = str(data[1])
    column[2] = str(data[2])
    column[3] = str(data[3])

    # lin, so
    column[4] = str(data[4])
    column[5] = '[' + ', '.join([':'.join([str(k), str(data[5][k])]) for k in sorted(data[5].keys())]) + ']'

    if data[7] + data[9] != 0:
      # bin, int (linear)
      column[6] = str(data[6])
      column[7] = str(data[7]-data[6])

      # bin, int (second-order)
      column[8] = str(data[8])
      column[9] = str(data[9]-data[8])

    # obj
    if data[0] in self.statdict:
      res = primobjstat(self.statdict[data[0]], data[7] + data[9] >= 1)      
      column[10] = res[0].replace('E-','E$-$') + self.objstatmark[res[1]]

    print('\n & '.join(column))
    print('\\\\')

class htmltable:
  def __init__(self):
    self.pack = ''
    self.statdict = None
    self.objstatmark = {'': '<sup>&ensp;</sup>', 'v': '<sup>v</sup>', 'a': '<sup>a</sup>'}
    self.withcss = True
    self.oddrow = True

  def opentable(self):
    print('''<div>''')

    if self.withcss:
      print('''\
  <style type="text/css" scoped>
  #cblib-stattable
  {
    margin: 45px;
    border-collapse: collapse;
  }
  #cblib-stattable th
  {
    font-family: "Lucida Sans Unicode", "Lucida Grande", Sans-Serif;
    font-size: 14px;
    font-weight: normal;
    text-align: center;
    padding: 0px 0px;
    color: #039;
  }
  #cblib-stattable td
  {
    font-family: "Lucida Sans Unicode", "Lucida Grande", Sans-Serif;
    font-size: 12px;
    font-weight: normal;
    padding: 4px 8px;
    color: #669;
  }
  #cblib-stattable div.nogroup
  {
    padding: 4px 8px;
    padding-top: 2px;
  }
  #cblib-stattable div.group
  {
    padding-bottom: 2px;
  }
  #cblib-stattable div.lborder
  {
    margin-right: 8px;
    border-bottom: 1px dotted black;
  }
  #cblib-stattable div.cborder
  {
    margin: 0px 8px;
    border-bottom: 1px dotted black;
  }
  #cblib-stattable div.rborder
  {
    margin-left: 8px;
    border-bottom: 1px dotted black;
  }
  #cblib-stattable .rsep
  {
    border-right: 1px solid #dde;
  }
  #cblib-stattable .strcol
  {
    text-align: left;
  }
  #cblib-stattable .numcol
  {
    text-align: right;
  }
  #cblib-stattable .oddrow
  {
    background: #e8edff;
  }
  </style>''')

    print('''\
  <table id="cblib-stattable">
    <thead>
      <tr>
        <th colspan="1"><div class="group">&nbsp;</div></th>
        <th colspan="3"><div class="group lborder">Size</div></th>
        <th colspan="2"><div class="group cborder">Conic&nbsp;domains</div></th>
        <th colspan="2"><div class="group rborder">Binary</div></th>
        <th colspan="2"><div class="group lborder">Integer</div></th>
        <th colspan="1"><div class="group rborder">Status</div></th>
      </tr>''')

    column = ['Instances','var','map','nnz','lin','so','b<sub>lin</sub>','b<sub>so</sub>','I<sub>lin</sub>','I<sub>so</sub>','obj']
    out = ''
    for i in range(0,11):
      out += '<th><div class="nogroup ' + self.__colclass(i, True) + '">' + str(column[i]) + '</div></th>'
    print(self.__row(out))

    print('''\
    </thead>
    <tfoot>
      <tr><td colspan="11"><br/><sup>a(ccurancy)</sup> Infeasibility meassures exceed 10<sup>-4</sup> on some primitive cones or integer requirements (points not normalized).</td></tr>
      <tr><td colspan="11"><sup>v(alue)</sup> Objective neither claimed by a solver to be within an absolute and relative gap of 0.0 from optimality (mixed-integer case), nor certified to be within an absolute gap of 10<sup>-4</sup> or relative gap of 10<sup>-7</sup> from optimality (continuous case).</td></tr>
    </tfoot>
    <tbody>''')

  def closetable(self):
    print('''\
    </tbody>
  </table>
</div>''')

  def addrow(self, data):
    column = ['']*11

    # Pack
    if data[0] in self.statdict:
      pack = self.statdict[data[0]]['pack']
    else:
      pack = ''

    if self.pack != pack:
      column[0] = '<b>' + pack + '</b>'
      out = ''
      for i in range(0,11):
        out += '<td class="' + self.__colclass(i, False) + '">' + str(column[i]) + '</td>'
      print(self.__row(out))
      self.pack = pack

    # Name
    column[0] = data[0]

    # var, map, nnz
    column[1] = data[1]
    column[2] = data[2]
    column[3] = data[3]

    # lin, so
    column[4] = str(data[4])
    column[5] = '[' + ', '.join([':'.join([str(k), str(data[5][k])]) for k in sorted(data[5].keys())]) + ']'

    if data[7] + data[9] != 0:

      # bin, int (linear)
      column[6] = str(data[6])
      column[7] = str(data[7]-data[6])

      # bin, int (second-order)
      column[8] = str(data[8])
      column[9] = str(data[9]-data[8])

    # obj
    if data[0] in self.statdict:
      res = primobjstat(self.statdict[data[0]], data[7] + data[9] >= 1)
      column[10] = res[0].replace('E-','E&minus;') + self.objstatmark[res[1]]

    out = ''
    for i in range(0,11):
      out += '<td class="' + self.__colclass(i, False) + '">' + str(column[i]) + '</td>'
    print(self.__row(out))

  def __colclass(self, idx, isheader):
    postfix = ''
    if not isheader and idx in [0,3,5,9]:
      postfix = ' rsep'

    if idx == 0:
      return('strcol' + postfix)
    else:
      return('numcol' + postfix)

  def __row(self, value):
    self.oddrow = not self.oddrow
    if self.oddrow:
      return('      <tr class="oddrow">' + value + '</tr>')
    else:
      return('      <tr>' + value + '</tr>')


def primobjstat(csvstat, isinteger):
  pfeas_obj = float('nan')
  pfeas_err = float('inf')
  pinfeas_err = float('inf')

  dfeas_obj = float('nan')
  dfeas_err = float('inf')
  dinfeas_err = float('inf')

  isminimize = (csvstat['psense'].strip().upper() == 'MIN')

  if csvstat['pcer'] == 'FEASIBILITY':
    pfeas_obj = float(csvstat['pobj'])
    pfeas_err = float(csvstat['perr'])
  elif csvstat['pcer'] == 'INFEASIBILITY':
    pinfeas_err = float(csvstat['perr'])

  if csvstat['dcer'] == 'FEASIBILITY':
    dfeas_obj = float(csvstat['dobj'])
    dfeas_err = float(csvstat['derr'])
  elif csvstat['pcer'] == 'INFEASIBILITY':
    dinfeas_err = float(csvstat['derr'])

  if isinteger:
    return primobjstat_integer(pfeas_obj, pfeas_err, pinfeas_err, dinfeas_err, csvstat['claim'])
  else:
    return primobjstat_continuous(pfeas_obj, pfeas_err, pinfeas_err, dfeas_obj, dfeas_err, dinfeas_err, isminimize)


def stattable(out, statfile, filtexpr, cbfset):
  # Find the directory of this script
  scriptdir = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
  rootdir = os.path.join(scriptdir,'..','..')

  # Read instance status data
  csvfile = open(statfile, 'rt')
  csvdialect = csv.Sniffer().sniff(csvfile.read(), ';\t')
  csvfile.seek(0)
  csvreader = csv.reader(csvfile, csvdialect, quotechar='"')
  next(csvreader)
  out.statdict = dict((rows[1], dict((x,y) for x,y in zip(statdictnames, rows))) for rows in csvreader)
  csvfile.close()

  # Print table
  out.opentable()
  filter(filtexpr, "[||name||, ||var||, ||map||, ||nnz||, ||entries|lin||, ||dimlist|so||, ||binary|lin||, ||int|lin||, ||binary|so||, ||int|so||]", cbfset, out.addrow)
  out.closetable()


if __name__ == "__main__":
  try:
    # Verify command line arguments
    opts, args = getopt.gnu_getopt(sys.argv[1:], "s:o:", "filter=")
    if len(args) != 1:
      raise Exception('Incorrect usage, exactly one stat file should be specified!')
  except Exception as e:
    print(str(e))
    print(''.join(['Try all mixed-integer second order cone instances:', '\n',
          '  python ', sys.argv[0], ' <STATFILE> --filter="||int|| and ||cones|so|| and not ||psdcones||"', '\n']))
    sys.exit(2)

  filtexpr = ""
  setexpr = None
  out = htmltable()
  statfile = args[0]

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

  # Prepare set of instances
  if setexpr is not None:
    cbfset = CBFset()
    cbfset.read(setexpr)
  else:
    cbfset = defaultcbfset()

  try:
    stattable(out, statfile, filtexpr, cbfset)
  except Exception as e:
    print(str(e))

