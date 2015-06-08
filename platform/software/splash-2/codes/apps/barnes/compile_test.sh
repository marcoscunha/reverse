#!/bin/bash

BARNES_DIR=$RABBITS_DIR/platforms/bunny/soft/splash-2/codes/apps/barnes

function d2h
{
echo "ibase=10;obase=16;$1" | bc; 
}

function change_code
{
REP_LINE="NPROC=1"
NEW_LINE="NPROC=$1"

sed -e "s/$REP_LINE/$NEW_LINE/" $BARNES_DIR/scripts/code.C > $BARNES_DIR/code.C
}

function change_ldscript
{
NCPU=$1

REP_LINE="CPU_ARMV6_COUNT = .; LONG(0x1)"
NEW_LINE="CPU_ARMV6_COUNT = .; LONG($NCPU)"

sed -e "s/$REP_LINE/$NEW_LINE/" $BARNES_DIR/scripts/ldscript > $BARNES_DIR/ldscript
}


echo "Modifing files..."
change_code $1
change_ldscript $1
echo "Setting up the environment..."
cd $BARNES_DIR
. $RABBITS_DIR/platforms/bunny/soft/Apes/install.sh
. $BARNES_DIR/install.sh
echo "Compiling..."
make
cd -







