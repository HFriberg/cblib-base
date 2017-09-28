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
