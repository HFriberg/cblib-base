def primdist(x):
  return max(map(abs, x))

dualcone = __import__('dist.F', fromlist='F')
dualdist = dualcone.primdist
