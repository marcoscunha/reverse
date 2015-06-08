#! /bin/bash
#

# This script compiles all application defined in $APPS variable with 
# default number of processors.
# If you want to change the number of processors of each application,
# then use the script ./compile_test.sh inside each application folder.
# Usage: ./compile_test <number_of_proc>

APPS="barnes ocean/contiguous_partitions water-spatial water-nsquared"

if [ -z "$RABBITS_DIR" ]; then
    echo "Rabbits environment is not configured."
    exit 1
fi

if [ -z $APES_ROOT ]; then 
    echo "Apes environment is not configured."
    exit 1
fi

. ./install.sh

for APP in $APPS; do
   cd apps/$APP 
   echo "Compiling... : $APP"
   if [ $? == 0 ]; then
      . install.sh
      make > /dev/null 
      cd - > /dev/null
   else
      echo "Wrong application configuration: $APP"
      exit 1
   fi
done



#echo "[ compiling all the applications]"
#for j in soclib_dspin_16p_gm soclib_dspin_16_lm 
#do
#	for i in mini_ocean_c 
#	do
#		echo "[. install.sh $j $i]"
#		. install.sh $j $i 
#		make clean -s
#		make app_clean -s
#		make -s
#	done	
#done
#
