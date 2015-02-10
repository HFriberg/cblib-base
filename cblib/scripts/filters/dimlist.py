import __common__
import collections

def keyquery(cnam=None):
  return( set(['VAR','CON']) )

def getval(prob, cnam=None):
  cnam = __common__.parse_cnam(cnam)
  
  return dict( collections.Counter([prob.mapstackdim[k] for k in range(prob.mapstacknum) if cnam(prob.mapstackdomain[k])]) + 
               collections.Counter([prob.varstackdim[k] for k in range(prob.varstacknum) if cnam(prob.varstackdomain[k])]) )

