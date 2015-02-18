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

def update_stattable(statfile, probfiles, solfiles):

  # Find the directory of this script
  scriptdir = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
  rootdir = os.path.join(scriptdir,'..','..')

  # Read instance status data
  csvfile = open(statfile, 'rt')
  csvdialect = csv.Sniffer().sniff(csvfile.read(), ';\t')
  csvfile.seek(0)
  csvreader = csv.reader(csvfile, csvdialect, quotechar='"')
  header = next(csvreader)
  statdictnames = ['pack','name','psense','pcer','perr','pobj','dcer','derr','dobj','claim']
  statdict = dict((rows[1], dict((x,y) for x,y in zip(statdictnames, rows))) for rows in csvreader)
  csvfile.close()

  # Update instance status data
  for (probfile, solfile) in zip(probfiles, solfiles):
    sys.stdout.write('\n' + solfile + '\n')
    solstat = summary(probfile, solfile, lambda x: None)
    csvstat = statdict[solstat['prob'].name]
    isminimize = (solstat['prob'].obj.strip().upper() == 'MIN')

    cchanged = False
    pchanged = False
    dchanged = False

    if 'claim' in solstat:
      if not csvstat['claim'].strip() and solstat['claim'].strip():
        cchanged = True
        csvstat['claim'] = solstat['claim']
      elif csvstat['claim'].strip() and solstat['claim'].strip() and csvstat['claim'].strip().upper() != solstat['claim']:
        raise Exception(solstat['prob'].name + ': The claim ' + solstat['claim'] + ' is incompatible with the existing claim ' + csvstat['claim'])

    if 'psol' in solstat:
      psol = solstat['psol']
      perr = max(psol[1].values())
      if worty_replacement(csvstat['perr'], csvstat['pobj'], perr, psol[0], isminimize):
        pchanged = True
        csvstat['pcer'] = 'FEASIBILITY'
        csvstat['perr'] = '{0:.16g}'.format(perr)
        csvstat['pobj'] = '{0:.16g}'.format(psol[0])

    if 'pinfeascer' in solstat:
      pinfeascer = solstat['pinfeascer']
      perr = max(pinfeascer[1].values())
      if worty_replacement(csvstat['perr'], csvstat['pobj'], perr, pinfeascer[0], isminimize):
        pchanged = True        
        csvstat['pcer'] = 'INFEASIBILITY'
        csvstat['perr'] = '{0:.16g}'.format(perr)
        csvstat['pobj'] = '{0:.16g}'.format(pinfeascer[0])

    if solstat['prob'].intvarnum == 0:
      if 'dsol' in solstat:
        dsol = solstat['dsol']
        derr = max(dsol[1].values())
        if worty_replacement(csvstat['derr'], csvstat['dobj'], derr, dsol[0], not isminimize):
          dchanged = True
          csvstat['dcer'] = 'FEASIBILITY'
          csvstat['derr'] = '{0:.16g}'.format(derr)
          csvstat['dobj'] = '{0:.16g}'.format(dsol[0])

      if 'dinfeascer' in solstat:
        dinfeascer = solstat['dinfeascer']
        derr = max(dinfeascer[1].values())
        if worty_replacement(csvstat['derr'], csvstat['dobj'], derr, dinfeascer[0], not isminimize):
          dchanged = True
          csvstat['dcer'] = 'INFEASIBILITY'
          csvstat['derr'] = '{0:.16g}'.format(derr)
          csvstat['dobj'] = '{0:.16g}'.format(dinfeascer[0])

    if cchanged or pchanged or dchanged:
      sys.stdout.write('PENDING CHANGES: ')
      if cchanged:
        sys.stdout.write('CLAIMN ')
      if pchanged:
        sys.stdout.write('PRIMAL ')
      if dchanged:
        sys.stdout.write('DUAL ')

  # Define instance sorting
  convert = lambda text: int(text) if text.isdigit() else text
  alphanum_key = lambda key: [ convert(c) for c in re.split('([0-9]+)', key) ]
  sortedstatdict = sorted(statdict, key=alphanum_key)

  # Write instance status data
  csvfile = open(statfile, 'wt')
  csvwriter = csv.writer(csvfile, csvdialect, quotechar='"')
  csvwriter.writerow(header)
  for k in sortedstatdict:
    csvwriter.writerow([statdict[k][x] for x in statdictnames])
  csvfile.close()


def worty_replacement(csverr, csvobj, solerr, solobj, objminimize):
  if csverr == '':
    return(True)

  csverr = float(csverr)
  csvobj = float(csvobj)

  if csverr > 1e-4:
    # Error measure first, then objective
    return (
      (solerr < csverr) or
      (solerr == csverr and solobj < csvobj and objminimize) or
      (solerr == csverr and solobj > csvobj and not objminimize)
    )
  elif solerr <= 1e-4:
    # Objective first, then error measure
    return (
      (solobj < csvobj and objminimize) or
      (solobj > csvobj and not objminimize) or
      (solobj == csvobj and solerr < csverr)
    )
  else:
    return(False)


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
  filtexpr = None
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

  # Apply filter if any
  if filtexpr:
    cbfset.filter(filtexpr)

  try:
    update_stattable(statfile, cbfset.cbffiles, cbfset.solfiles)
  except Exception as e:
    print(str(e))
