# Copyright (c) 2012 by Zuse-Institute Berlin and the Technical University of Denmark.
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     1. Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#     2. Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#     3. Neither the name of the copyright holders nor contributors may not 
#        be used to endorse or promote products derived from this software 
#        without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS NOR CONTRIBUTORS BE LIABLE 
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

