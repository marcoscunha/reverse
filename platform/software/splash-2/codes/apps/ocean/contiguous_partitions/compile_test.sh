#!/bin/bash

OCEAN_DIR=$RABBITS_DIR/platforms/bunny/soft/splash-2/codes/apps/ocean/contiguous_partitions

function d2h
{
echo "ibase=10;obase=16;$1" | bc; 
}

function change_code
{
REP_LINE="DEFAULT_P        1"
NEW_LINE="DEFAULT_P        $1"

sed -e "s/$REP_LINE/$NEW_LINE/" $OCEAN_DIR/scripts/main.C > $OCEAN_DIR/main.C
}

function change_ldscript
{
NCPU=$1

REP_LINE="CPU_ARMV6_COUNT = .; LONG(0x1)"
NEW_LINE="CPU_ARMV6_COUNT = .; LONG($NCPU)"

sed -e "s/$REP_LINE/$NEW_LINE/" $OCEAN_DIR/scripts/ldscript > $OCEAN_DIR/ldscript
}


echo "Modifing files..."
change_code $1
change_ldscript $1
echo "Setting up the environment..."
cd $OCEAN_DIR
. $RABBITS_DIR/platforms/bunny/soft/Apes/install.sh
. $OCEAN_DIR/install.sh
echo "Compiling..."
make
cd -







