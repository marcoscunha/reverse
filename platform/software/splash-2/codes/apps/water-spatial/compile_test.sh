#!/bin/bash

if [ -z $INSTALL_BASE ]; then
    echo "Configure the environment: INSTALL_BASE is null"
    exit 1
fi

WATERS_DIR=$INSTALL_BASE/platform/software/splash-2/codes/apps/water-spatial


function change_code
{
REP_LINE="NB_P  8"
NEW_LINE="NB_P  $1"

sed -e "s/$REP_LINE/$NEW_LINE/" $WATERS_DIR/scripts/water.C > $WATERS_DIR/water.C
}

function change_ldscript
{
NCPU=$1

REP_LINE="CPU_ARMV6_COUNT = .; LONG(0x1)"
NEW_LINE="CPU_ARMV6_COUNT = .; LONG($NCPU)"

sed -e "s/$REP_LINE/$NEW_LINE/" $WATERS_DIR/scripts/ldscript > $WATERS_DIR/ldscript
}

echo "Modifing files..."
change_code $1
change_ldscript $1
echo "Setting up the environment..."
cd $WATERS_DIR
. $INSTALL_BASE/platform/software/dnaos/install.sh
. $WATERS_DIR/install.sh
echo "Compiling..."
make
cd -

