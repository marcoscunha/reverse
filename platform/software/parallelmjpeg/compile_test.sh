#!/bin/bash

PARALLELMJPEG_DIR=$RABBITS_DIR/platforms/bunny/soft/ParallelMjpeg

function d2h
{
echo "ibase=10;obase=16;$1" | bc; 
}

function change_install
{
REP_LINE="export MJPEG_CC_FLAGS=\"-DNB_DECODER=1 -DDISPATCH_TIME -DINFO -DVERBOSE\""
NEW_LINE="export MJPEG_CC_FLAGS=\"-DNB_DECODER=$NCPU -DDISPATCH_TIME -DINFO -DVERBOSE\""

REP_LINE_WRITE="\"-gdwarf-3 -Wall -Werror -Wno-format -std=c99\""
NEW_LINE_WRITE="\"-gdwarf-3 -Wall -Werror -Wno-format -std=c99 $WRITE\""

echo $NEW_LINE_WRITE

sed -e "s/$REP_LINE/$NEW_LINE/" $PARALLELMJPEG_DIR/scripts/install.sh | sed -e "s/$REP_LINE_WRITE/$NEW_LINE_WRITE/" > $PARALLELMJPEG_DIR/install.sh




}

function change_ldscript
{
#NCPU=$1

REP_LINE="CPU_ARMV6_COUNT = .; LONG(0x1)"
NEW_LINE="CPU_ARMV6_COUNT = .; LONG($NCPU)"

sed -e "s/$REP_LINE/$NEW_LINE/" $PARALLELMJPEG_DIR/scripts/ldscript > $PARALLELMJPEG_DIR/ldscript
}

function show_help
{
   echo "Usage: ./compile_test.sh [N_CPUS] [OPTIONS]"
   echo ""
   echo "         Chose just one option above"
   echo "         -wt : Enable corrections for writethrough protocol."
   echo "         -wb : Enable corrections for writeback protocol."
   exit 0;
}


while [[ $# > 0 ]]; do
    key="$1"
    shift
    case $key in
    -wt)
        WRITE=-DWRITETHROUGH
        ;;
    -wb)
        WRITE=-DWRITEBACK
        ;;
    -ncpu)
        NCPU=$1
        shift
        ;;
    -?|*)
        echo $key
        show_help
        ;;
    esac
done

echo "modifing files"
change_install $1
change_ldscript $1


echo "setting environment"
cd $PARALLELMJPEG_DIR
. $PARALLELMJPEG_DIR/../Apes/soft_env
. $PARALLELMJPEG_DIR/../Apes/install.sh
. $PARALLELMJPEG_DIR/install.sh
echo Compiling
#apes-cache -p && apes-compose -c && apes-compose
apes-compose -c && apes-compose

cd -







