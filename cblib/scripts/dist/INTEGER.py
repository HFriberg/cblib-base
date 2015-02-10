import math

def primdist(x):
  return max([abs(xj - math.floor(xj + 0.5)) for xj in x])

