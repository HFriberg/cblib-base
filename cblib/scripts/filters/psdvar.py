import __common__

def keyquery(cdim=None):
  return( set(['PSDVAR']) )

def getval(prob, cdim=None):
  if prob.psdvarnum == 0:
    return( 0 )
  else:
    cdim = __common__.parse_cdim(cdim)
    return( sum([n*(n+1)/2 for n in prob.psdvardim if cdim(n) ]) )
