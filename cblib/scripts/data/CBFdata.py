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


import os, sys, gzip

class CBFdata:
  def __init__(self, file, keyquery = None):
    self.file = file
    self.keywordset = [set(['VER']), \
                       set(['OBJSENSE','PSDVAR','VAR','INT','PSDCON','CON']), \
                       set(['OBJFCOORD','OBJACOORD','OBJBCOORD','FCOORD','ACOORD','BCOORD','HCOORD','DCOORD','CHANGE'])]
    self.keywordqueryset = [self.keywordset[i] | set([x + ':HEAD' for x in self.keywordset[i]]) for i in range(len(self.keywordset))]
    self.keywords = self.keywordset[0] | self.keywordset[1] | self.keywordset[2]

    if keyquery is not None:
      self.fullkeyquery = keyquery
    else:
      self.fullkeyquery = self.keywords | set([None])   # Parse all keywords and do not exit prematurely

  def iterator(self):
    self.ver = None               # File version
    self.obj = None               # Problem structure
    self.mapnum = 0
    self.mapstacknum = 0
    self.mapstackdim = list()
    self.mapstackdomain = list()
    self.varnum = 0
    self.varstacknum = 0
    self.varstackdim = list()
    self.varstackdomain = list()
    self.intvarnum = 0
    self.intvar = list()
    self.psdmapnum = 0
    self.psdmapdim = list()
    self.psdvarnum = 0
    self.psdvardim = list()
    self.objfnnz = 0              # Objective coefficients
    self.objfsubj = list()
    self.objfsubk = list()
    self.objfsubl = list()
    self.objfval = list()
    self.objannz = 0
    self.objasubj = list()
    self.objaval = list()
    self.objbval = 0
    self.fnnz = 0                 # Scalar map coefficients
    self.fsubi = list()
    self.fsubj = list()
    self.fsubk = list()
    self.fsubl = list()
    self.fval = list()
    self.annz = 0
    self.asubi = list()
    self.asubj = list()
    self.aval = list()
    self.bnnz = 0
    self.bsubi = list()
    self.bval = list()
    self.hnnz = 0                 # PSD map coefficients
    self.hsubi = list()
    self.hsubj = list()
    self.hsubk = list()
    self.hsubl = list()
    self.hval = list()
    self.dnnz = 0
    self.dsubi = list()
    self.dsubk = list()
    self.dsubl = list()
    self.dval = list()

    self.change = False

    self.simplebounds = False
    simplemapvaridx = list()
    simplemapsign   = list()
    simplemapconst  = list()

    keyset = 0
    keyquery = self.fullkeyquery.copy()
    (linenum,line) = (-1, "")

    [self.name, filetype] = os.path.splitext(os.path.basename(self.file))
    if filetype.lower() == '.gz':
      self.name = os.path.splitext(self.name)[0]
      ff = gzip.open(self.file,'rt')
    else:
      ff = open(self.file,'rt')

    f = enumerate(ff)
    try:
      for (linenum,line) in f:
        line = self.__prepare_line(line)

        # Ignore comments between blocks
        if line.startswith('#'):
          continue

        # Ignore empty lines between blocks
        if not line:
          continue

        # Stop when requested information has been gathered
        if len(keyquery) == 0:
          break


        #
        # Keyword set: File description keywords
        #
        if keyset == 0:
          if line == "VER":
            (linenum,line) = next(f)
            self.ver = int(self.__prepare_line(line))
            keyquery.discard("VER")
            keyquery.discard("VER:HEAD")
            continue

          # Unrecognized line. Going to next set of keywords.
          if line in self.keywords:
            keyset = self.__inc_keyset(keyset)
            keyquery -= self.keywordqueryset[keyset-1]
          else:
            raise Exception('Keyword not recognized')


        #
        # Keyword set: Structural keywords (note the default values)
        #
        if keyset == 1:
          if line == "OBJSENSE":
            if self.obj is not None:
              raise Exception('Keyword also found earlier and can only appear once')

            (linenum,line) = next(f)
            self.obj = self.__prepare_line(line)
            keyquery.discard("OBJSENSE")
            keyquery.discard("OBJSENSE:HEAD")
            continue

          if line == "PSDVAR":
            if self.psdvarnum > 0:
              raise Exception('Keyword also found earlier and can only appear once')

            (linenum,line) = next(f)
            self.psdvarnum = int(self.__prepare_line(line))

            if "PSDVAR" in keyquery:
              self.psdvardim = [0]*self.psdvarnum
              for i in range(self.psdvarnum):
                (linenum,line) = next(f)
                self.psdvardim[i] = int(self.__prepare_line(line))

            elif not keyquery <= set(["PSDVAR:HEAD"]):
              for i in range(self.psdvarnum):
                next(f)

            keyquery.discard("PSDVAR")
            keyquery.discard("PSDVAR:HEAD")
            continue

          if line == "VAR":
            if self.varnum > 0:
              raise Exception('Keyword also found earlier and can only appear once')

            (linenum,line) = next(f)
            buf = self.__prepare_line(line).split(' ')
            self.varnum = int(buf[0])
            self.varstacknum = int(buf[1])

            if "VAR" in keyquery:
              self.varstackdomain = ['']*self.varstacknum
              self.varstackdim    =  [0]*self.varstacknum
              for i in range(self.varstacknum):
                (linenum,line) = next(f)
                buf = self.__prepare_line(line).split(' ')
                self.varstackdomain[i] = buf[0]
                self.varstackdim[i]    = int(buf[1])

            elif not keyquery <= set(["VAR:HEAD"]):
              for i in range(self.varstacknum):
                next(f)

            keyquery.discard("VAR")
            keyquery.discard("VAR:HEAD")
            continue

          if line == "INT":
            if self.intvarnum > 0:
              raise Exception('Keyword also found earlier and can only appear once')

            (linenum,line) = next(f)
            self.intvarnum = int(self.__prepare_line(line))

            if "INT" in keyquery:
              self.intvar = [0]*self.intvarnum
              for i in range(self.intvarnum):
                (linenum,line) = next(f)
                self.intvar[i] = int(self.__prepare_line(line))

            elif not keyquery <= set(["INT:HEAD"]):
              for i in range(self.intvarnum):
                next(f)

            keyquery.discard("INT")
            keyquery.discard("INT:HEAD")
            continue

          if line == "PSDCON":
            if self.psdmapnum > 0:
              raise Exception('Keyword also found earlier and can only appear once')

            (linenum,line) = next(f)
            self.psdmapnum = int(self.__prepare_line(line))

            if "PSDCON" in keyquery:
              self.psdmapdim = [0]*self.psdmapnum
              for i in range(self.psdmapnum):
                (linenum,line) = next(f)
                self.psdmapdim[i] = int(self.__prepare_line(line))

            elif not keyquery <= set(["PSDCON:HEAD"]):
              for i in range(self.psdmapnum):
                next(f)

            keyquery.discard("PSDCON")
            keyquery.discard("PSDCON:HEAD")
            continue

          if line == "CON":
            if self.mapnum > 0:
              raise Exception('Keyword also found earlier and can only appear once')

            (linenum,line) = next(f)
            buf = self.__prepare_line(line).split(' ')
            self.mapnum = int(buf[0])
            self.mapstacknum = int(buf[1])

            if "CON" in keyquery:
              self.mapstackdomain = ['']*self.mapstacknum
              self.mapstackdim    =  [0]*self.mapstacknum
              for i in range(self.mapstacknum):
                (linenum,line) = next(f)
                buf = self.__prepare_line(line).split(' ')
                self.mapstackdomain[i] = buf[0]
                self.mapstackdim[i]    = int(buf[1])

            elif not keyquery <= set(["CON:HEAD"]):
              for i in range(self.mapstacknum):
                next(f)

            keyquery.discard("CON")
            keyquery.discard("CON:HEAD")
            continue

          # Unrecognized line. Going to next set of keywords.
          if line in self.keywords:
            keyset = self.__inc_keyset(keyset)
            keyquery = self.__resolve_keyquery_logic(keyquery)
            keyquery -= self.keywordqueryset[keyset-1]

            self.simplebounds = (all(x in self.fullkeyquery for x in ['VAR','CON'])
                                 and (not self.mapnum or 'BCOORD' in keyquery)
                                 and (not self.mapnum or not self.varnum or 'ACOORD' in keyquery)
                                 and (not self.mapnum or not self.psdvarnum or 'FCOORD' in keyquery))
            
            if self.simplebounds:
              simplemapvaridx = [-1]*self.mapnum
              simplemapsign   = [1.0]*self.mapnum
              simplemapconst  = [0.0]*self.mapnum

            if len(keyquery) == 0:
              break
          else:
            raise Exception('Keyword not recognized')


        #
        # Keyword set: Data keywords
        #
        if keyset == 2:

          if line == "OBJFCOORD":
            if not self.change and self.objfnnz != 0.0:
              raise Exception('Keyword also found earlier and can only appear once')

            (linenum,line) = next(f)
            curnnz         = int(self.__prepare_line(line))

            if "OBJFCOORD" in keyquery:
              self.objfsubj += [0]*curnnz
              self.objfsubk += [0]*curnnz
              self.objfsubl += [0]*curnnz
              self.objfval  += [0.0]*curnnz
              for i in xrange(self.objfnnz, self.objfnnz + curnnz):
                (linenum,line) = next(f)
                buf = self.__prepare_line(line).split(' ')
                self.objfsubj[i] = int(buf[0])
                self.objfsubk[i] = int(buf[1])
                self.objfsubl[i] = int(buf[2])
                self.objfval[i]  = float(buf[3])

            elif not keyquery <= set(["OBJFCOORD:HEAD"]):
              for i in xrange(self.objfnnz, self.objfnnz + curnnz):
                next(f)

            self.objfnnz += curnnz
            keyquery.discard("OBJFCOORD")
            keyquery.discard("OBJFCOORD:HEAD")
            continue

          if line == "OBJACOORD":
            if not self.change and self.objannz != 0.0:
              raise Exception('Keyword also found earlier and can only appear once')

            (linenum,line) = next(f)
            curnnz         = int(self.__prepare_line(line))

            if "OBJACOORD" in keyquery:
              self.objasubj += [0]*curnnz
              self.objaval  += [0.0]*curnnz
              for i in xrange(self.objannz, self.objannz + curnnz):
                (linenum,line) = next(f)
                buf = self.__prepare_line(line).split(' ')
                self.objasubj[i] = int(buf[0])
                self.objaval[i]  = float(buf[1])

            elif not keyquery <= set(["OBJACOORD:HEAD"]):
              for i in xrange(self.objannz, self.objannz + curnnz):
                next(f)

            self.objannz += curnnz
            keyquery.discard("OBJACOORD")
            keyquery.discard("OBJACOORD:HEAD")
            continue

          if line == "OBJBCOORD":
            if not self.change and self.objbval != 0.0:
              raise Exception('Keyword also found earlier and can only appear once')

            (linenum,line) = next(f)
            self.objbval   = float(self.__prepare_line(line))

            keyquery.discard("OBJBCOORD")
            keyquery.discard("OBJBCOORD:HEAD")
            continue

          if line == "FCOORD":
            if not self.change and self.fnnz > 0:
              raise Exception('Keyword also found earlier and can only appear once')

            (linenum,line) = next(f)
            curnnz         = int(self.__prepare_line(line))

            if "FCOORD" in keyquery:
              self.fsubi += [0]*curnnz
              self.fsubj += [0]*curnnz
              self.fsubk += [0]*curnnz
              self.fsubl += [0]*curnnz
              self.fval  += [0.0]*curnnz
              for i in xrange(self.fnnz, self.fnnz + curnnz):
                (linenum,line) = next(f)
                buf = self.__prepare_line(line).split(' ')
                self.fsubi[i] = int(buf[0])
                self.fsubj[i] = int(buf[1])
                self.fsubk[i] = int(buf[2])
                self.fsubl[i] = int(buf[3])
                self.fval[i]  = float(buf[4])

                if self.simplebounds:
                  simplemapvaridx[self.fsubi[i]] = -2

            elif not keyquery <= set(["FCOORD:HEAD"]):
              for i in xrange(self.fnnz, self.fnnz + curnnz):
                next(f)

            self.fnnz  += curnnz
            keyquery.discard("FCOORD")
            keyquery.discard("FCOORD:HEAD")
            continue

          if line == "ACOORD":
            if not self.change and self.annz > 0:
              raise Exception('Keyword also found earlier and can only appear once')

            (linenum,line) = next(f)
            curnnz         = int(self.__prepare_line(line))

            if "ACOORD" in keyquery:
              self.asubi += [0]*curnnz
              self.asubj += [0]*curnnz
              self.aval  += [0.0]*curnnz
              for i in xrange(self.annz, self.annz + curnnz):
                (linenum,line) = next(f)
                buf = self.__prepare_line(line).split(' ')
                self.asubi[i] = int(buf[0])
                self.asubj[i] = int(buf[1])
                self.aval[i]  = float(buf[2])

                if self.simplebounds:
                  if abs(self.aval[i]) == 1.0 and simplemapvaridx[self.asubi[i]] == -1:
                    simplemapvaridx[self.asubi[i]] = self.asubj[i]
                    simplemapsign[self.asubi[i]] = self.aval[i]
                  else:
                    simplemapvaridx[self.asubi[i]] = -2

            elif not keyquery <= set(["ACOORD:HEAD"]):
              for i in xrange(self.annz, self.annz + curnnz):
                next(f)

            self.annz  += curnnz
            keyquery.discard("ACOORD")
            keyquery.discard("ACOORD:HEAD")
            continue

          if line == "BCOORD":
            if not self.change and self.bnnz > 0:
              raise Exception('Keyword also found earlier and can only appear once')

            (linenum,line) = next(f)
            curnnz      = int(self.__prepare_line(line))

            if "BCOORD" in keyquery:
              self.bsubi += [0]*curnnz
              self.bval  += [0.0]*curnnz
              for i in xrange(self.bnnz, self.bnnz + curnnz):
                (linenum,line) = next(f)
                buf = self.__prepare_line(line).split(' ')
                self.bsubi[i] = int(buf[0])
                self.bval[i]  = float(buf[1])

                if self.simplebounds:
                  simplemapconst[self.bsubi[i]] = self.bval[i]

            elif not keyquery <= set(["BCOORD:HEAD"]):
              for i in xrange(self.bnnz, self.bnnz + curnnz):
                next(f)

            self.bnnz  += curnnz
            keyquery.discard("BCOORD")
            keyquery.discard("BCOORD:HEAD")
            continue

          if line == "HCOORD":
            if not self.change and self.hnnz > 0:
              raise Exception('Keyword also found earlier and can only appear once')

            (linenum,line) = next(f)
            curnnz      = int(self.__prepare_line(line))

            if "HCOORD" in keyquery:
              self.hsubi += [0]*curnnz
              self.hsubj += [0]*curnnz
              self.hsubk += [0]*curnnz
              self.hsubl += [0]*curnnz
              self.hval  += [0.0]*curnnz
              for i in xrange(self.hnnz, self.hnnz + curnnz):
                (linenum,line) = next(f)
                buf = self.__prepare_line(line).split(' ')
                self.hsubi[i] = int(buf[0])
                self.hsubj[i] = int(buf[1])
                self.hsubk[i] = int(buf[2])
                self.hsubl[i] = int(buf[3])
                self.hval[i]  = float(buf[4])

            elif not keyquery <= set(["HCOORD:HEAD"]):
              for i in xrange(self.hnnz, self.hnnz + curnnz):
                next(f)

            self.hnnz  += curnnz
            keyquery.discard("HCOORD")
            keyquery.discard("HCOORD:HEAD")
            continue

          if line == "DCOORD":
            if not self.change and self.dnnz > 0:
              raise Exception('Keyword also found earlier and can only appear once')

            (linenum,line) = next(f)
            curnnz      = int(self.__prepare_line(line))

            if "DCOORD" in keyquery:
              self.dsubi += [0]*curnnz
              self.dsubk += [0]*curnnz
              self.dsubl += [0]*curnnz
              self.dval  += [0.0]*curnnz
              for i in xrange(self.dnnz, self.dnnz + curnnz):
                (linenum,line) = next(f)
                buf = self.__prepare_line(line).split(' ')
                self.dsubi[i] = int(buf[0])
                self.dsubk[i] = int(buf[1])
                self.dsubl[i] = int(buf[2])
                self.dval[i]  = float(buf[3])

            elif not keyquery <= set(["DCOORD:HEAD"]):
              for i in xrange(self.dnnz, self.dnnz + curnnz):
                next(f)

            self.dnnz  += curnnz
            keyquery.discard("DCOORD")
            keyquery.discard("DCOORD:HEAD")
            continue

          if line == "CHANGE":
            self.change = True
            self.__missing_keyword_scan(keyset)

            # Stop at current state of variables
            yield self
            keyset = 2
            keyquery = self.fullkeyquery & (self.keywordqueryset[2] | set([None]))
            continue


        raise Exception('Keyword not recognized')

      #
      # End of file reached at this point
      #
      (linenum,line) = (linenum+1, "")
      if len(keyquery) != 0:
        self.__missing_keyword_scan(keyset)

      # Compute variable bounds when information is available
      if self.simplebounds:
        self.blx = [float("-inf")] * self.varnum
        self.bux = [float("+inf")] * self.varnum

        j = -1
        for k in range(self.varstacknum):
          for km in range(self.varstackdim[k]):
            j = j + 1
            if self.varstackdomain[k] in ['L=','L+'] or (self.varstackdomain[k] == 'Q' and km <= 1) or (self.varstackdomain[k] == 'QR' and km <= 2):
              self.blx[j] = max(self.blx[j], 0.0)
            if self.varstackdomain[k] in ['L=','L-']:
              self.bux[j] = min(self.bux[j], 0.0)

        i = -1
        for k in range(self.mapstacknum):
          for km in range(self.mapstackdim[k]):
            i = i + 1
            j = simplemapvaridx[i]
            if j >= 0:
              if self.mapstackdomain[k] in ['L=','L+'] or (self.mapstackdomain[k] == 'Q' and km <= 1) or (self.mapstackdomain[k] == 'QR' and km <= 2):
                if simplemapsign[i] > 0:
                  self.blx[j] = max(self.blx[j], -simplemapconst[i]*simplemapsign[i])
                else:
                  self.bux[j] = min(self.bux[j], -simplemapconst[i]*simplemapsign[i])
              if self.mapstackdomain[k] in ['L=','L-']:
                if simplemapsign[i] > 0:
                  self.bux[j] = min(self.bux[j], -simplemapconst[i]*simplemapsign[i])
                else:
                  self.blx[j] = max(self.blx[j], -simplemapconst[i]*simplemapsign[i])

      # Stop at current state of variables
      yield self

    except Exception as e:
      if isinstance(e, StopIteration):
        msg = 'Unexpected end of file'
      else:
        msg = str(e)

      raise Exception(''.join([
            msg, '. File: ', self.file, '\n',
            str(linenum+1), ': ', line, '\n']))

    finally:
      ff.close()


  def __prepare_line(self, line):
    line = line.rstrip('\r\n')
    if len(line) > 510:
      # Line content should fit within 512 bytes with room for '\r\n'
      raise Exception('Line too wide')
    else:
      # Ignore leading and trailing whitespace characters
      return line.strip(' ')

  def __missing_keyword_scan(self, keyset):
    while keyset <= 2:
      keyset = self.__inc_keyset(keyset)

  def __inc_keyset(self, keyset):
    if keyset == 0:
      if self.ver is None:
        raise Exception('Expected keyword "VER"')

    elif keyset == 1:
      if self.obj is None:
        raise Exception('Expected keyword "OBJSENSE"')

    return keyset+1

  def __resolve_keyquery_logic(self, keyquery):
    res_keyquery = set([])

    for x in keyquery:
      if x is not None:
        xx = x.split('=>')
        for i in xrange(len(xx)-1):
          if xx[i] =='PSDVAR':
            if not self.psdvarnum:
              break
          elif xx[i] =='PSDCON':
            if not self.psdmapnum:
              break
          elif xx[i] =='INT':
            if not self.intvarnum:
              break
          elif xx[i] =='VAR':
            if not self.varnum:
       	      break
          elif xx[i] =='CON':
            if not self.mapnum:
       	      break
          else:
            raise Exception('Keyword "' + xx[i] + '" not supported as structural dependency in partial file parsing')
        else:
          res_keyquery.add(xx[-1])

    return(res_keyquery)
    
