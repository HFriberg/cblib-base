import math

def primdist(x):
  Tx = [ (x[0] + x[1])/math.sqrt(2), (x[0] - x[1])/math.sqrt(2) ] + list(x[2:])
  return rotcone.primdist(Tx)

rotcone = __import__('dist.Q', fromlist='Q')
dualdist = primdist
