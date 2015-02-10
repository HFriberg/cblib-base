import __common__

def keyquery(cnam=None, cdim=None):
  if cnam is None and cdim is None:
    return( set(['VAR:HEAD']) )
  else:
    return( set(['VAR']) )

def getval(prob, cnam=None, cdim=None):
  if (cnam is None and cdim is None) or (prob.varstacknum == 0):
    return( prob.varstacknum )
  
  else:
    cnam = __common__.parse_cnam(cnam)
    cdim = __common__.parse_cdim(cdim)
    return sum([1 for k in range(prob.varstacknum) if cnam(prob.varstackdomain[k]) and cdim(prob.varstackdim[k]) ])
