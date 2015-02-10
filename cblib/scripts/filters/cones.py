import varcones
import mapcones

def keyquery(cnam=None, cdim=None):
  return( varcones.keyquery(cnam,cdim) | mapcones.keyquery(cnam,cdim) )

def getval(prob, cnam=None, cdim=None):
  return( varcones.getval(prob,cnam,cdim) + mapcones.getval(prob,cnam,cdim) )
