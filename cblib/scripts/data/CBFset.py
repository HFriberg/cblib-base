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

import os, csv, re

class CBFset:
  def getroot(self, setpath):
    if os.path.isfile(setpath):
       return os.path.realpath(os.path.abspath(os.path.join(os.path.dirname(setpath), 'cbf')))
    else:
      rootdir = os.path.realpath(os.path.abspath(setpath))
      while os.path.basename(rootdir) != 'cbf':
        if rootdir == os.path.dirname(rootdir):
          raise Exception('Failed to find "cbf" directory in path')
        else:
          rootdir = os.path.dirname(rootdir)
      return rootdir

  def getpack(self, filepath, rootdir):
    if rootdir == "":
      return "";
    else:
      return os.path.dirname(os.path.relpath(filepath,rootdir)).replace('\\','/')

  def getinstance(self, filepath):
    [instance, filetype] = os.path.splitext(os.path.basename(filepath))
    if filetype.lower() == '.gz':
      instance = os.path.splitext(instance)[0]
    return instance

  def read(self, setpath):
    self.setpath = setpath
    self.rootdir = self.getroot(setpath)
    self.cbffiles = list()
    self.solfiles = list()

    if os.path.isfile(setpath):
      csvfile = open(setpath, 'rt')
      csvdialect = csv.Sniffer().sniff(csvfile.read(), ';\t')
      csvfile.seek(0)
  
      csvreader = csv.reader(csvfile, csvdialect, quotechar='"')

      # Skip header
      next(csvreader)

      # Read data
      for row in csvreader:
        for instance in row[1].split(', '):
          cbfpath = os.path.join(self.rootdir, *(row[0].split('/') + [instance]))

          if os.path.isfile(cbfpath + '.cbf'):
            cbfpath += '.cbf'
          elif os.path.isfile(cbfpath + '.cbf.gz'):
            cbfpath += '.cbf.gz'
          else:
            raise Exception(''.join(['Instance ', instance, ' not found in: ', os.path.dirname(cbfpath + 'x')]))
  
          self.cbffiles.append(cbfpath)
          self.solfiles.append(os.path.join(os.path.realpath(os.path.abspath(self.setpath)) + '.sol', *(row[0].split('/') + [os.path.basename(cbfpath) + '.sol'])))

    else:
      for dirpath, dirnames, filenames in os.walk(setpath):
        convert = lambda text: int(text) if text.isdigit() else text
        alphanum_key = lambda key: [ convert(c) for c in re.split('([0-9]+)', key) ]
        filenames.sort(key=alphanum_key)
        dirnames.sort(key=alphanum_key)
  
        for filename in filenames:
          [fbase,fext] = os.path.splitext(filename)
          if fext.lower() == '.gz':
            fext = os.path.splitext(fbase)[1] + fext
  
          if fext.lower() in ['.cbf','.cbf.gz']:
            cbfpath = os.path.join(dirpath,filename)
            self.cbffiles.append(cbfpath)
            self.solfiles.append(os.path.join(os.path.realpath(os.path.abspath(self.setpath)) + '.sol', os.path.relpath(cbfpath,self.rootdir) + '.sol'))

  def readfilelist(self, filelist):

    # Unorganised list of files
    self.setpath = ""
    self.rootdir = ""
    self.cbffiles = list()
    self.solfiles = list()

    if isinstance(filelist, basestring):
      # Read data from newline separated file
      cbflist = open(filelist, 'rt')
    else:
      # Read data from list argument
      cbflist = filelist

    for row in cbflist:
      [fbase,fext] = os.path.splitext(row)
      if fext.lower() == '.gz':
        fext = os.path.splitext(fbase)[1] + fext

      if fext.lower() in ['.cbf','.cbf.gz']:
        cbfpath = os.path.realpath(os.path.abspath(row))

        if os.path.isfile(cbfpath):
          self.cbffiles.append(cbfpath)
          self.solfiles.append(cbfpath + '.sol')
        else:
          raise Exception(''.join(['Instance "', cbfpath, '" not found.']))
