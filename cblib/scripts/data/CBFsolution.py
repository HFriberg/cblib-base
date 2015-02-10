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


class CBFsolution:
  def __init__(self):

    # Claim of benchmark library status for instace when no certificate can be given
    self.claimlist = ['INTEGER_OPTIMALITY', 'INTEGER_INFEASIBILITY', 'UNSTABLE']
    self.claim = None

    # Variables in primal problem (affine map values are computed)
    self.primvar = list()
    self.primpsdvar = list()

    # Variables in dual problem (affine map values are computed)
    # NOTE: Assuming continuous relaxation in case of mixed-integer instances...
    self.dualvar = list()
    self.dualpsdvar = list()


  def printsol(self, printer):
    if self.claim is not None:
      printer('CLAIM')
      printer(self.claim)
      printer('')

    if len(self.primvar) > 0:
      printer('PRIMVAR')
      for x in self.primvar:
        printer('{0:.16g}'.format(x))
      printer('')

    if len(self.primpsdvar) > 0:
      printer('PRIMPSDVAR')
      for vech in self.primpsdvar:
        for x in vech:
          printer('{0:.16g}'.format(x))
      printer('')

    if len(self.dualvar) > 0:
      printer('DUALVAR')
      for x in self.dualvar:
       	printer('{0:.16g}'.format(x))
      printer('')

    if len(self.dualpsdvar) > 0:
      printer('DUALPSDVAR')
      for vech in self.dualpsdvar:
       	for x in vech:
          printer('{0:.16g}'.format(x))
      printer('')

  def readsol(self, prob, file):
    (linenum,line) = (-1, "")
    ff = open(file,'rt')
    f = enumerate(ff)
    try:
      for (linenum,line) in f:
        line = self.__prepare_line(line)

        # Ignore empty lines between blocks
        if not line:
          continue

        if line == "CLAIM":
          if self.claim is not None:
            raise Exception('Keyword also found earlier and can only appear once')

          (linenum,line) = next(f)
          self.claim = self.__prepare_line(line)
          if self.claim not in self.claimlist:
            raise Exception('Not a valid claim')
          continue

        if line == "PRIMVAR":
          if len(self.primvar) > 0:
            raise Exception('Keyword also found earlier and can only appear once')

          self.primvar = [0.0] * prob.varnum
          for i in xrange(prob.varnum):
            (linenum,line) = next(f)
            self.primvar[i] = float(self.__prepare_line(line))
          continue

        if line == "DUALVAR":
          if len(self.dualvar) > 0:
            raise Exception('Keyword also found earlier and can only appear once')

       	  self.dualvar = [0.0] * prob.mapnum
          for i in xrange(prob.mapnum):
            (linenum,line) = next(f)
            self.dualvar[i] = float(self.__prepare_line(line))
          continue

        raise Exception('Keyword not recognized')

      #
      # End of file reached at this point
      #
      (linenum,line) = (linenum+1, "")

    except Exception as e:
      if isinstance(e, StopIteration):
        msg = 'Unexpected end of file'
      else:
        msg = str(e)

      raise Exception(''.join([
            msg, '. File: ', file, '\n',
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
