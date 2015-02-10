import psdvar
import psdmap

def keyquery(cdim=None):
  return( psdvar.keyquery(cdim) | psdmap.keyquery(cdim) )

def getval(prob, cdim=None):
  return( psdvar.getval(prob,cdim) + psdmap.getval(prob,cdim) )
