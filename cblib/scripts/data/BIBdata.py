# Copyright (c) 2012 by Zuse-Institute Berlin and the Technical University of Denmark.
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.


class BIBdata:
  def __init__(self, bibpath):
    self.refs = dict()
    
    bibfile = open(bibpath,'rt')
    try:
      curkey = list()
      curref = list()
      inRef = False
      noRefComma = True
      cntBraces = 0
      escaping = False

      while True:
        c = bibfile.read(1)
        if not c:
          break

        # Parse special chars
        if not escaping:
          if c == "@":
            inRef = True

          if c == ",":
            if inRef:
              noRefComma = False

          if c == "{":
            cntBraces = cntBraces + 1

          if c == "}":
            cntBraces = cntBraces - 1

          if c == "\\":
            escaping = True
        else:
          escaping = False

        # Keep reference updated
        if inRef:
          curref.append(c)

        # Keep reference key updated
        if inRef and noRefComma and cntBraces == 1 and not c == "{":
          curkey.append(c)

        # Add finished references to dictionary
        if inRef and cntBraces == 0 and c == "}":
          self.refs[''.join(curkey).strip()] = ''.join(curref)
          curkey = list()
          curref = list()
          inRef = False
          noRefComma = True

    finally:
      bibfile.close()

  def get(self, key):
    return(self.refs[key])

