def parse_cnam(cnam):
  if cnam is None:
    return( lambda x: True )
  else:
    shortname = dict({'lin': ['F','L+','L-','L='], 'so': ['Q','QR']})    
    
    carg = []
    for x in cnam.split(','):
      if x in shortname:
        carg += shortname[x]
      else:
        carg += [x]
    return( lambda x: x in carg )

def parse_cdim(cdim):
  if cdim is None:
    return( lambda x: True )
  else:
    return( eval('lambda x: x ' + cdim) )
  
