#!/bin/bash

BUNNY_DIR=$RABBITS_DIR/platforms/bunny


function change_makefile
{
CPU_REQ_REP_LINE="-DRABBITS_TRACE_EVENT_CPU_REQ"
CPU_REQ_NEW_LINE=" "
CACHE_REP_LINE="-DRABBITS_TRACE_EVENT_CACHE"
CACHE_NEW_LINE=" "
IND_REP_LINE="-DTRACE_EVENT_BLOCK_CYCLES"
IND_NEW_LINE=" "

if [ -z $CPU_REQ ]; then
    CPU_REQ_SED="s/$CPU_REQ_REP_LINE/$CPU_REQ_NEW_LINE/"
fi
if [[ -z $CACHE && -z $CPU_REQ ]]; then
    CACHE_SED="s/$CACHE_REP_LINE/$CACHE_NEW_LINE/"
fi

if [ ! -z $IND ]; then
    IND_SED="s/$IND_REP_LINE/$IND_NEW_LINE/"
fi

sed -e "$CPU_REQ_SED;$CACHE_SED;$IND_SED" $BUNNY_DIR/Makefile.model  > $BUNNY_DIR/Makefile

}

function show_help
{
   echo "Usage: ./compile_test.sh [OPTIONS]"
   echo ""
   echo "         -r : Enable cpu request and cache event generation."
   echo "         -c : Enable cache event generation."
   echo "         -i : Enable individual timestamping."  
   echo ""
   exit 0;
}

while [[ $# > 0 ]]; do
    key="$1"
    shift
    case $key in
    -r) 
        CPU_REQ=1
        ;;
    -c) 
        CACHE=1
        ;;
    -i) 
        IND=1
        ;;
    -?|*)
        show_help
        ;;
    esac
done

if [[ -z $CPU_REQ || -z $CACHE || ! -z $IND ]]; then
    change_makefile
else
    cp  $BUNNY_DIR/Makefile.model  $BUNNY_DIR/Makefile
fi

cd $BUNNY_DIR
make clean
make
cd - 
