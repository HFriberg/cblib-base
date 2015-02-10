import __common__

def keyquery(cnam=None, cdim=None):
  if cnam is None and cdim is None:
    return( set(['CON:HEAD']) )
  else:
    return( set(['CON']) )

def getval(prob, cnam=None, cdim=None):
  if (cnam is None and cdim is None) or (prob.mapnum == 0):
    return( prob.mapnum )
  
  else:
    cnam = __common__.parse_cnam(cnam)
    cdim = __common__.parse_cdim(cdim)
    return sum([prob.mapstackdim[k] for k in range(prob.mapstacknum) if cnam(prob.mapstackdomain[k]) and cdim(prob.mapstackdim[k]) ])
