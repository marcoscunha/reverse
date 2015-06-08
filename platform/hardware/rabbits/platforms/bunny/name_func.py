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
    opts, args = getopt.getopt(argv,"hi:b:o:", [] )
except getopt.GetoptError:
    print("name_parse -i <input.text> -b <binary> -o <output.text>")    
    sys.exit(2)

for opt, arg in opts:
    if opt == '-h' :
        print("name_parse -i <input.text> -b <binary> -o <output.text>")
    elif opt == '-i' :
        infile_name = arg
    elif opt == '-b' :
        binfile_name = arg
    elif opt == '-o' :
        outfile_name = arg
       
if infile_name == None or binfile_name == None or outfile_name == None :
    print("name_parse -i <input.text> -b <binary> -o <output.text>")  
    sys.exit(1)

infile = open(infile_name)
outfile = open(outfile_name, 'w')
os.system("nm --defined-only -n "+binfile_name+"  > "+binfile_name+".tmp")
os.system("grep -vF $ "+binfile_name+".tmp > "+binfile_name+".nm")

symbolfile = open(binfile_name+".nm")
# HASH TABLE with Symbols
func = dict()
for iline in symbolfile:
    symbol = parse("{addr} {type} {func}\n", iline)
    func[symbol['addr']] = symbol['func']


for iline in infile:    
    address = parse("__entry_function: 0x{addr}\n", iline)
    if address == None :
        address = parse("__exit_function: 0x{addr}\n", iline)
        if address == None:
            print("Error: not desired pattern")
            sys.exit(1)
        
    addr = address['addr'].rjust(8, '0')

    func_name = func[addr]

    if func_name:
       outfile.write(iline.replace("0x"+address['addr']+'\n', func_name+"\n"))
    else :
       outfile.write(iline.replace("\n", ""))
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

symbolfile.close()
os.remove(binfile_name+".nm")
infile.close()
outfile.close()



