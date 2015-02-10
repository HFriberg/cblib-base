def keyquery():
  return( set(['CON=>PSDVAR=>FCOORD:HEAD','CON=>VAR=>ACOORD:HEAD','PSDCON=>VAR=>HCOORD:HEAD']) )

def getval(prob):
  return( prob.fnnz + prob.annz + prob.hnnz )
