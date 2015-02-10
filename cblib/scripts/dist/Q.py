import math

def primdist(x):
  ss1 = x[0]*x[0]
  ss2 = sum([i*i for i in x[1:]])

  if ss1 >= ss2:
    return 0
  elif -ss1 >= ss2:
    return math.sqrt(ss1 + ss2)
  else:
    return (math.sqrt(ss2) - x[0]) / math.sqrt(2)

dualdist = primdist
