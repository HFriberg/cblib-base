import __common__

def keyquery(cdim=None):
  if cdim is None:
    return( set(['PSDCON:HEAD']) )
  else:
    return( set(['PSDCON']) )

def getval(prob, cdim=None):
  if (cdim is None) or (prob.psdmapnum == 0):
    return( prob.psdmapnum )
  else:
    cdim = __common__.parse_cdim(cdim)
    return( sum([1 for n in prob.psdmapdim if cdim(n) ]) )
