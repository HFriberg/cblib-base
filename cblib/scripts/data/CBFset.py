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


import os, csv, re
import filter

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
            cbfpath = os.path.realpath(os.path.abspath(os.path.join(dirpath,filename)))
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

  def filter(self, filtexpr):
    cbffilter = list()
    filter.filter("", ''.join(["bool(", filtexpr, ")"]), self, lambda x: cbffilter.append(x))
    self.cbffiles = [self.cbffiles[i] for i in range(len(cbffilter)) if cbffilter[i]]
    self.solfiles = [self.solfiles[i] for i in range(len(cbffilter)) if cbffilter[i]]    
