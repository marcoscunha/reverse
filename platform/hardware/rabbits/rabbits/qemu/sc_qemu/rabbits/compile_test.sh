#!/bin/bash

QEMU_DIR=$RABBITS_DIR/rabbits/qemu/sc_qemu


function change_cfg
{
PERF_REP_LINE="#define RABBITS_PERF"
PERF_NEW_LINE="\/*#define RABBITS_PERF*\/"

TRACE_REP_LINE="#define RABBITS_GEN_TRACE_EVENT"
TRACE_NEW_LINE="\/*#define RABBITS_GEN_TRACE_EVENT*\/"

CPU_REQ_REP_LINE="#define RABBITS_TRACE_EVENT_CPU_REQ"
CPU_REQ_NEW_LINE="\/*#define RABBITS_TRACE_EVENT_CPU_REQ*\/"

CACHE_REP_LINE="#define RABBITS_TRACE_EVENT_CACHE"
CACHE_NEW_LINE="\/*#define RABBITS_TRACE_EVENT_CACHE*\/"


if [ -z $PERF ]; then
    PERF_SED="s/$PERF_REP_LINE/$PERF_NEW_LINE/"
fi
if [ -z $TRACE ]; then
    TRACE_SED="s/$TRACE_REP_LINE/$TRACE_NEW_LINE/"
fi

if [ -z $CPU_REQ ]; then
    CPU_REQ_SED="s/$CPU_REQ_REP_LINE/$CPU_REQ_NEW_LINE/"
fi

if [[ -z $CACHE && -z $CPU_REQ ]]; then
    CACHE_SED="s/$CACHE_REP_LINE/$CACHE_NEW_LINE/"
fi


sed -e "$PERF_SED;$TRACE_SED;$CPU_REQ_SED;$CACHE_SED" $QEMU_DIR/rabbits/cfg.h.model  > $QEMU_DIR/rabbits/cfg.h

}

function show_help
{
   echo "Usage: ./compile_test.sh [OPTIONS]"
   echo ""
   echo "OPTIONS: -t : Enable trace generation."
   echo "         -p : Enable profiling."
   echo "         -r : Enable cpu request events."
   echo "         -c : Enable cache events."
   echo ""
   exit 0;
}

#if [ $# == 0 ]; then
#    show_help
#fi


while [[ $# > 0 ]]; do
    key="$1"
    shift
    case $key in 
    -p)
        PERF=1
        ;;
    -t)
        TRACE=1
        ;;
    -r)
        CPU_REQ=1
        ;;
    -c)
        CACHE=1
        ;;
    -?|*)
        show_help
    ;;
    esac
done

if [[ -z $PERF || -z $TRACE- || -z $CPU_REQ || -z $CACHE ]]; then
    change_cfg 
else
    cp $QEMU_DIR/rabbits/cfg.h.model $QEMU_DIR/rabbits/cfg.h
fi

cd $QEMU_DIR
make clean
make all 
cd - 
