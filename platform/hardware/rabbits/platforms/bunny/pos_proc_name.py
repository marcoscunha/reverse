#! /usr/bin/env python

import os 
import sys
import getopt
import re
from parse import *

print 'Number of Arguments', len(sys.argv), 'arguments'

argv = sys.argv[1:]

infile_name = None
binfile_name = None
outfile_name = None

try:
    opts, args = getopt.getopt(argv,"hi:r:o:", [] )
except getopt.GetoptError:
    print("name_parse -i <input.text> -r <reference> -o <output.text>")    
    sys.exit(2)

for opt, arg in opts:
    if opt == '-h' :
        print("name_parse -i <input.text> -r <reference> -o <output.text>")
    elif opt == '-i' :
        infile_name = arg
    elif opt == '-r' :
        reffile_name = arg
    elif opt == '-o' :
        outfile_name = arg
       
if infile_name == None or reffile_name == None or outfile_name == None :
    print("name_parse -i <input.text> -r <reference> -o <output.text>")  
    sys.exit(1)

reference = open(reffile_name)

# HASH TABLE with Symbols
func = dict()
i = 0 
for iline in reference:
    symbol = parse("{type}: {func_name}\n", iline)
#    print symbol
    if func.get(symbol['func_name']) == None :
        i += 1
        func[symbol['func_name']] = str(i)
        
reference.close()

infile = open(infile_name)
outfile = open(outfile_name, 'w')

j = 0
for iline in infile:    
    function = parse("__entry_function: {func_name}\n", iline)
    if function == None :
        function = parse("__exit_function: {func_name}\n", iline)
        continue
        if function == None:
            print("Error: not desired pattern")
            sys.exit(1)
        
    func_name = function['func_name']
    func_index = func[func_name]

    if func_index :
      j+=1;
      outfile.write(iline.replace('\n', " "+func_index+" "+str(j)+"\n"))
#       outfile.write(iline.replace("0x"+address['addr']+'\n', func_name+"\n"))
    else :
       outfile.write(iline.replace("\n", "0" +str(j)+" \n"))
       print("Error: "+iline )

#    string = addr + " T {func}"
#    symbolfile.seek(0)
#    for sline in symbolfile :
#        ex = parse(string, sline)
#        if ex : 
#            if ex['func'] != "$a" and ex['func'] != "$d": 
#                break

#    if ex:
#       outfile.write(iline.replace("0x"+address['addr']+'\n', ex['func']+"\n"))
#    else :
#       outfile.write(iline.replace("\n", ""))
#       print("Error: "+iline )

#os.remove(binfile_name+".nm")
infile.close()
outfile.close()



