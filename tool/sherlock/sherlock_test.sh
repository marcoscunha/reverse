#!/bin/bash


TRACE_EXEC=$RABBITS_DIR/platforms/bunny/bunny_test.sh
BUNNY_DIR=$RABBITS_DIR/platforms/bunny/
OCEAN_CMDS=
MJPEG_CMDS=

TRACE_DIR=$RABBITS_DIR/platforms/bunny/trace
GEN_TRACE=1

APP_DIR=/work/cunha/devel/Rabbits/platforms/bunny/soft/

MJPEG_DIR=$APP_DIR/ParallelMjpeg/bin
SPLASH_DIR=$APP_DIR/splash-2/codes/apps/
OCEAN_DIR=$SPLASH_DIR/ocean/contiguous_partitions/bin/
WATERS_DIR=$SPLASH_DIR/water-spatial/bin/
WATERN_DIR=$SPLASH_DIR/water-nsquared/bin/


NCPU="1 2 4 8 12"
NCPU_TRACE="\"$NCPU\""
APPS="ocean"


for APP in $APPS; do
    echo "TOTAL;REV;FWD;NCPU;MEM;#trace_cache_perf;$APP" >> perf

    # TODO: TEST IF THE PROGRAMS ARE COMPILED!
    for CPU in $NCPU; do
        if [ ! -z $GEN_TRACE ]; then
            cd $BUNNY_DIR
            echo "$TRACE_EXEC -app $APP -ncpu $CPU -platform trace_cache_perf -execute -clean_before"
            $TRACE_EXEC -app $APP -ncpu $CPU -platform trace_cache_perf -execute -clean_before
            if [ $? == 1 ]; then
                cd -
                exit 1
            fi
        fi
        cd -
         APP_DIR=$OCEAN_DIR
#        APP_DIR=$WATERN_DIR
#        APP_DIR=$WATERS_DIR
#        APP_DIR=$MJPEG_DIR

        cp $BUNNY_DIR/mem_init . 
#    for CPU in $NCPU; do
        ./sherlock $TRACE_DIR/RABBITS.$APP.$CPU.hwe -gdb_port 1234 &
        arm-sls-dnaos-gdb -x gdb_commands_$APP $APP_DIR/$APP\_$CPU
    done
done




#NCPU=12
#$TRACE_EXEC -app ocean -ncpu $NCPU -platform trace_cache_perf -execute -clean_before


#Generate All Traces
#$TRACE_EXEC -app Parallelmjpeg -ncpu "1 2 4 8" -platform trace_cache_perf -execute -clean_before

# Execute Sherlock 
#echo "TOTAL;REV;FWD;NCPU;MEM;#cache_perf;ParallelMjpeg" >> perf
#./sherlock  ../../Rabbits/platforms/bunny/trace/RABBITS.ParallelMjpeg.1.hwe  -gdb_port 1234 &
#arm-sls-dnaos-gdb -x gdb_commands ../../Rabbits/platforms/bunny/soft/ParallelMjpeg/bin/ParallelMjpeg_1




