import var
import map

def keyquery(cnam=None, cdim=None):
  return( var.keyquery(cnam,cdim) | map.keyquery(cnam,cdim) )

def getval(prob, cnam=None, cdim=None):
  return( var.getval(prob,cnam,cdim) + map.getval(prob,cnam,cdim) )
