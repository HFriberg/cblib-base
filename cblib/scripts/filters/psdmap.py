import __common__

def keyquery(cdim=None):
  return( set(['PSDCON']) )

def getval(prob, cdim=None):
  if prob.psdmapnum == 0:
    return( 0 )
  else:
    cdim = __common__.parse_cdim(cdim)
    return( sum([n*(n+1)/2 for n in prob.psdmapdim if cdim(n) ]) )
