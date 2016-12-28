#!/usr/bin/env python2

# This little python program creates the datatype specific variants

import re
import sys
import os
import fnmatch

def replaceFunc(data, d):
  return data.replace("<DATATYPE>", d).replace("<DATATYPE_UPPER>", d.upper().replace("_T", ""))

def parsefile(infile, outfile):
  DATATYPES=["float","double"]
  DATATYPES_FULL=["float","double","int8_t","int16_t","int32_t","int64_t"]

  fin = open(infile, "r")
  data = fin.read()
  fin.close()
  m = re.search("//Supported datatypes:(.*)", data)
  if (m):
    datatypes_list = m.group(1).split(" ")
  else:
    datatypes_list = DATATYPES

  datatypes_functions = [ "NULL" for x in DATATYPES_FULL for y in ["compress", "decompress"]]
  datatypes_supported = []
  for d in datatypes_list:
    d = d.strip()
    if (d in DATATYPES_FULL):
      pos = DATATYPES_FULL.index(d)
      datatypes_supported.append(d)
      datatypes_functions[pos*2] = "compress_" + d
      datatypes_functions[pos*2+1] = "decompress_" + d

  m = re.search("CREATE_INITIALIZER[(](.*)[)]", data)
  if(m):
    init_name = m.group(1)
    datatypes_list = []
    for d in datatypes_functions:
      if (d != "NULL"):
        datatypes_list.append(init_name + "_" + d)
      else:
        datatypes_list.append(d)
    datatype_list = ",\n      ".join( datatypes_list )
    data = re.sub("CREATE_INITIALIZER(.*)", datatype_list, data)

  # identify the regions to replace
  lines = data.split("\n")
  outData = []
  repeatData = []
  repeat = False
  for l in lines:
    if re.search("//.*Repeat for each data type", l):
      repeat = True
      next
    if re.search("//.*End repeat", l):
      repeat = False
      # paste the lines
      for d in datatypes_supported:
        outData.append( replaceFunc("\n".join(repeatData), d) )
      next
    if not repeat:
      outData.append(l)
    else:
      repeatData.append(l)

  data = "\n".join(outData)

  fout = open(outfile, "w")
  fout.write(data)
  fout.close()



if (len(sys.argv) < 3):
  print("ERROR, synopsis: <indir> <outdir>")
  sys.exit(1)
else:
  indir=sys.argv[1]
  outdir=sys.argv[2]

files = [os.path.join(path, f)
      for path, dirnames, files in os.walk(indir)
      for f in fnmatch.filter(files, '*.dtype.*')]


for f in files:
  m = re.match("(.*)[.]dtype[.](.*)", f[len(indir)+1:])
  if not m:
    print("Error processing " + f)
    continue
  suffix = m.group(1) + "." + m.group(2)
  outfile = outdir + "./" + suffix

  if not os.path.isfile(outfile) or (os.path.getctime(f) > os.path.getctime(outfile)):
    print("Processing " + suffix)
    match = re.search("/(.*)/.*?", suffix)
    if match:
        directory = outdir + match.group(1)
        if not os.path.exists(directory):
            os.mkdir(directory)

    parsefile(f, outfile)
