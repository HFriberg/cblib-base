import __common__

def keyquery(cnam=None, cdim=None):
  return( set(['INT','VAR','CON','INT=>PSDVAR=>FCOORD','INT=>CON=>ACOORD','INT=>CON=>BCOORD']) )

def getval(prob, cnam=None, cdim=None):
  if prob.intvarnum == 0:
    return( 0 )
  
  elif cnam is None and cdim is None:
    return( sum([1 for j in prob.intvar if -0.5 <= prob.blx[j] and prob.bux[j] <= 1.5]) )
  
  else:
    cnam = __common__.parse_cnam(cnam)
    cdim = __common__.parse_cdim(cdim)
    
    prob.intvar.sort()
    numint = 0
    curint = 0
    j = -1
    for k in range(prob.varstacknum):
      if cnam(prob.varstackdomain[k]) and cdim(prob.varstackdim[k]):
        for km in range(prob.varstackdim[k]):
          j += 1
  
          while j > prob.intvar[curint]:
            curint += 1
            if curint >= prob.intvarnum:
              return( numint )
  
          if j == prob.intvar[curint] and -0.5 <= prob.blx[j] and prob.bux[j] <= 1.5:
            numint += 1
      else:
        j += prob.varstackdim[k]
  
    return( numint )
