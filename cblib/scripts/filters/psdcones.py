import psdvarcones
import psdmapcones

def keyquery(cdim=None):
  return( psdvarcones.keyquery(cdim) | psdmapcones.keyquery(cdim) )

def getval(prob, cdim=None):
  return( psdvarcones.getval(prob,cdim) + psdmapcones.getval(prob,cdim) )
