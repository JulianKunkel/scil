#!/usr/bin/env python2.7

# This little python program creates the datatype specific variants

import re
import sys
import os
import fnmatch


def replaceFunc(data, d):
  datatype_size = { "float": 4,"double": 8,"int8_t" : 1,"int16_t":2,"int32_t":4,"int64_t":8 }
  return data.replace("<DATATYPE>", d).replace("<DATATYPE_UPPER>", d.upper().replace("_T", "")).replace("<DATATYPE_SIZE>", str(datatype_size[d] * 8)).replace("<DATATYPE_SIZE_BYTE>", str(datatype_size[d]))

def createFunctionList(datatypes_list):
  DATATYPES_FULL=["float","double","int8_t","int16_t","int32_t","int64_t"]

  datatypes_functions = [ "NULL" for x in DATATYPES_FULL for y in ["compress", "decompress"]]
  datatypes_supported = []
  for d in datatypes_list:
    d = d.strip()
    if (d in DATATYPES_FULL):
      pos = DATATYPES_FULL.index(d)
      datatypes_supported.append(d)
      datatypes_functions[pos*2] = "compress_" + d
      datatypes_functions[pos*2+1] = "decompress_" + d
  return datatypes_functions


def parsefile(infile, outfile):
  datatypes_list = ["float","double"]
  fin = open(infile, "r")
  data = fin.read()
  fin.close()

  # identify the regions to replace
  lines = data.split("\n")
  outData = []
  repeatData = []
  repeat = False
  all_supported = []

  for l in lines:
    m = re.search("//Supported datatypes:(.*)", l)
    if (m):
      datatypes_list = m.group(1).strip().split(" ")
      all_supported.extend(datatypes_list)
      continue
    if re.search("//.*Repeat for each data type", l):
      repeat = True
      continue
    if re.search("//.*End repeat", l):
      repeat = False
      # paste the lines
      for d in datatypes_list:
        outData.append( replaceFunc("\n".join(repeatData), d) )
      repeatData = []
      continue
    m = re.search("CREATE_INITIALIZER[(](.*)[)]", l)
    if(m):
      init_name = m.group(1)
      all_supported.extend(datatypes_list)
      datatypes_functions = createFunctionList(all_supported)

      datatypes_list = []
      for d in datatypes_functions:
        if (d != "NULL"):
          datatypes_list.append(init_name + "_" + d)
        else:
          datatypes_list.append(d)
      datatype_list = ",\n      ".join( datatypes_list )
      outData.append(datatype_list)
      continue
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
    match = re.search("(.*)/.*?", suffix)
    if match:
        directory = outdir + match.group(1)
        if not os.path.exists(directory):
            os.mkdir(directory)

    parsefile(f, outfile)
