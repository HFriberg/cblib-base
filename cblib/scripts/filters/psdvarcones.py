import __common__

def keyquery(cdim=None):
  if cdim is None:
    return( set(['PSDVAR:HEAD']) )
  else:
    return( set(['PSDVAR']) )

def getval(prob, cdim=None):
  if (cdim is None) or (prob.psdvarnum == 0):
    return( prob.psdvarnum )
  else:
    cdim = __common__.parse_cdim(cdim)
    return( sum([1 for n in prob.psdvardim if cdim(n) ]) )
