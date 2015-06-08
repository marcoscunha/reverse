#!/bin/bash


for ((n=1; n<=$1; n++))
do
   echo EXECUTION $n
   rm RABBITS.hwe > /dev/null
   ./bunny -ncpu $2 
done  

