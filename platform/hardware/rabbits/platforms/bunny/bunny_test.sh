#!/bin/bash

BUNNY_DIR=$RABBITS_DIR/platforms/bunny
PARALLEL_DIR=$BUNNY_DIR/soft/ParallelMjpeg
DINNER_DIR=$BUNNY_DIR/soft/Dinner
BARNES_DIR=$BUNNY_DIR/soft/splash-2/codes/apps/barnes
OCEAN_DIR=$BUNNY_DIR/soft/splash-2/codes/apps/ocean/contiguous_partitions
WATERS_DIR=$BUNNY_DIR/soft/splash-2/codes/apps/water-spatial
WATERN_DIR=$BUNNY_DIR/soft/splash-2/codes/apps/water-nsquared


QEMU_DIR=$RABBITS_DIR/rabbits/qemu/sc_qemu
#GEN=
#EXEC=1
#STORE_TRACE=1

#Possible CPU_SET="1 2 4 8 12"
#CPU_SET="8"


#include some commands like: clean traces, applis conf, generation conf, 


#PLATFORMS="trace_perf"
#APPLICATIONS="ParallelMjpeg"
#PLATFORMS="base_perf trace_cpu_perf trace_cache_perf trace_perf"
#PLATFORMS="trace_cache_perf trace_perf"
#PLATFORMS="base_perf trace_perf trace_ind_perf trace_cache_perf trace_cache_ind_perf" 
#PLATFORMS="trace_cache_perf" 



#Replace this function for a generic function !

function gen_parallel 
{
    mkdir -p $PARALLEL_DIR/bin
    rm -f $PARALLEL_DIR/bin/*
    for NCPU in 1 2 4 8 12; do
        echo "MJPEG - $NCPU CPUs"
        $PARALLEL_DIR/compile_test.sh $NCPU
        cp $PARALLEL_DIR/MJPEG $PARALLEL_DIR/bin/ParallelMjpeg_$NCPU
    done
}

function gen_dinner
{
    mkdir -p $DINNER_DIR/bin
    rm -f $DINNER_DIR/bin/*
    for NCPU in 1 2 4 8 12; do
        echo "DINNER - $NCPU CPUs"
        $DINNER_DIR/compile_test.sh $NCPU
        cp $DINNER_DIR/dinner $DINNER_DIR/bin/dinner_$NCPU
    done
}

function gen_barnes
{
    mkdir -p $BARNES_DIR/bin
    rm  -f $BARNES_DIR/bin/*
    for NCPU in 1 2 4 8 12; do
        echo "BARNES - $NCPU CPUs"
        $BARNES_DIR/compile_test.sh $NCPU
        cp $BARNES_DIR/barnes $BARNES_DIR/bin/barnes_$NCPU
    done
}

function gen_ocean
{
    mkdir -p $OCEAN_DIR/bin
    rm -f $OCEAN_DIR/bin/*
    for NCPU in 1 2 4 8 12; do
        echo "OCEAN - $NCPU CPUs"
        $OCEAN_DIR/compile_test.sh $NCPU
        cp $OCEAN_DIR/ocean $OCEAN_DIR/bin/ocean_$NCPU
    done
}

function gen_waters
{
    mkdir -p $WATERS_DIR/bin
    rm  -f $WATERS_DIR/bin/*
    for NCPU in 1 2 4 8 12; do
        echo "WATER SPATIAL - $NCPU CPUs"
        $WATERS_DIR/compile_test.sh $NCPU
        cp $WATERS_DIR/waterspatial $WATERS_DIR/bin/waterspatial_$NCPU
    done
}

function gen_watern
{
    mkdir -p $WATERN_DIR/bin
    rm -f  $WATERN_DIR/bin/*
    for NCPU in 1 2 4 8 12; do
        echo "WATER NSQUARED - $NCPU CPUs"
        $WATERN_DIR/compile_test.sh $NCPU
        cp $WATERN_DIR/waternsquared $WATERN_DIR/bin/waternsquared_$NCPU
    done
}

function gen_qemu
{
    mkdir -p $QEMU_DIR/rabbits/bin
    rm -f $QEMU_DIR/rabbits/bin/*

    # BASE
    $QEMU_DIR/rabbits/compile_test.sh > /dev/null 2>&1 
    cp $RABBITS_DIR/rabbits/libs/libqemu-system-arm.so $QEMU_DIR/rabbits/bin/libqemu-system-arm.so
   
    # BASE + PERF
    $QEMU_DIR/rabbits/compile_test.sh -p  > /dev/null 2>&1 
    cp $RABBITS_DIR/rabbits/libs/libqemu-system-arm.so $QEMU_DIR/rabbits/bin/libqemu-system-arm.so.p

    # TRACE
    $QEMU_DIR/rabbits/compile_test.sh -t  > /dev/null 2>&1
    cp $RABBITS_DIR/rabbits/libs/libqemu-system-arm.so $QEMU_DIR/rabbits/bin/libqemu-system-arm.so.t

    # TRACE + PERF
    $QEMU_DIR/rabbits/compile_test.sh -p -t > /dev/null 2>&1
    cp $RABBITS_DIR/rabbits/libs/libqemu-system-arm.so $QEMU_DIR/rabbits/bin/libqemu-system-arm.so.p.t

    # TRACE + CPU + PERF
    $QEMU_DIR/rabbits/compile_test.sh -p -t -r  > /dev/null 2>&1
    cp $RABBITS_DIR/rabbits/libs/libqemu-system-arm.so $QEMU_DIR/rabbits/bin/libqemu-system-arm.so.p.t.r
    
    # TRACE + CACHE + PERF
    $QEMU_DIR/rabbits/compile_test.sh -p -t -c > /dev/null 2>&1
    cp $RABBITS_DIR/rabbits/libs/libqemu-system-arm.so $QEMU_DIR/rabbits/bin/libqemu-system-arm.so.p.t.c

}

function gen_platform
{
    mkdir -p $BUNNY_DIR/bin
    rm -f $BUNNY_DIR/bin/*

    $BUNNY_DIR/compile_test.sh 
    cp $BUNNY_DIR/run.x $BUNNY_DIR/bin/run.x

    # INDIVIDUAL TIME
    $BUNNY_DIR/compile_test.sh -i    
    cp $BUNNY_DIR/run.x $BUNNY_DIR/bin/run.x.i

    # CPU + CACHE
    $BUNNY_DIR/compile_test.sh -r
    cp $BUNNY_DIR/run.x $BUNNY_DIR/bin/run.x.r

    # CPU + CACHE  + INDIVIDUAL TIME
    $BUNNY_DIR/compile_test.sh -r -i
    cp $BUNNY_DIR/run.x $BUNNY_DIR/bin/run.x.r.i

    # CACHE 
    $BUNNY_DIR/compile_test.sh -c 
    cp $BUNNY_DIR/run.x $BUNNY_DIR/bin/run.x.c

    # CACHE + INDIVIDUAL TIME
    $BUNNY_DIR/compile_test.sh -c -i
    cp $BUNNY_DIR/run.x $BUNNY_DIR/bin/run.x.c.i


}



function set_platform
{
    echo "======================================"
    
    echo $1 

    case $1 in
    base)
        PLAT_FILE=run.x
        QEMU_FILE=libqemu-system-arm.so
        echo "     BASE PLATFORM"
    ;;
    base_perf)
        PLAT_FILE=run.x
        QEMU_FILE=libqemu-system-arm.so.p
        echo "     BASE PLATFORM"
        echo "     WITH PROFILING"
    ;;
    trace)
        PLAT_FILE=run.x
        QEMU_FILE=libqemu-system-arm.so.t
        echo "     TRACE PLATFORM"
    ;;
    trace_perf)
        PLAT_FILE=run.x
        QEMU_FILE=libqemu-system-arm.so.p.t
        echo "     TRACE PLATFORM"
        echo "     WITH PROFILING"
    ;;
    trace_ind_perf)
        PLAT_FILE=run.x.i
        QEMU_FILE=libqemu-system-arm.so.p.t
        echo "     TRACE PLATFORM"
        echo "     WITH CPU REQ, PROFILING and INDIVIDUAL TIMESTAMPING"
    ;;
    trace_cpu_perf)
        PLAT_FILE=run.x.r
        QEMU_FILE=libqemu-system-arm.so.p.t.r
        echo "     TRACE PLATFORM"
        echo "     WITH CPU REQ and PROFILING"
    ;;
    trace_cpu_ind_perf)
        PLAT_FILE=run.x.r.i
        QEMU_FILE=libqemu-system-arm.so.p.t.r
        echo "     TRACE PLATFORM"
        echo "     WITH CPU REQ, PROFILING and INDIVIDUAL TIMESTAMPING"
    ;;
   
    trace_cache_perf)
        PLAT_FILE=run.x.c
        QEMU_FILE=libqemu-system-arm.so.p.t.c
        echo "     TRACE PLATFORM"
        echo "     WITH CACHE and PROFILING"
    ;;
    trace_cache_ind_perf)
        PLAT_FILE=run.x.c.i
        QEMU_FILE=libqemu-system-arm.so.p.t.c
        echo "     TRACE PLATFORM"
        echo "     WITH CACHE, PROFILING and INDIVIDUAL TIMESTAMPING"
    ;;
    *)
        echo "Unknown platform => $1"
        exit 1;
    ;;
    esac
    
    echo "======================================"
    echo "Setting Platform ..."

    if [ -e "$BUNNY_DIR/bin/$PLAT_FILE" ]; then
        cp $BUNNY_DIR/bin/$PLAT_FILE $BUNNY_DIR/run.x
    else
        echo "File not found: $PLAT_FILE"
        exit 1 
    fi
    echo "Setting QEmu Base..."
    if [ -e "$QEMU_DIR/rabbits/bin/$QEMU_FILE" ]; then 
        cp $QEMU_DIR/rabbits/bin/$QEMU_FILE $RABBITS_DIR/rabbits/libs/libqemu-system-arm.so
    else
        echo "File not found: $QEMU_FILE" 
        exit 1
    fi
}

function exec_platform
{
    rm -f RABBITS.hwe

    echo "--------------------------"
    echo "     $3 Processor(s)"
    echo "--------------------------"
#    echo "Setting $2..."
#    cp $1/bin/$2_$3 $1/$2
    echo "Executing..."
    echo "$BUNNY_DIR/bunny -kernel $1/bin/$2_$3 -ncpu $3"
#    $BUNNY_DIR/bunny -kernel $1/$2 -ncpu $3 > /dev/null 2>&1
    if [ -e  "$1/bin/$2_$3" ]; then
        $BUNNY_DIR/bunny -kernel $1/bin/$2_$3 -ncpu $3
        if [ -z $CLEAN_TRACE ]; then 
            mv RABBITS.hwe trace/RABBITS.$2.$3.hwe
        fi
    else 
        echo "File not found: $1/bin/$2_$3"
        exit 1
    fi
}

function run_test
{
    if [ ! -z $REMOVE_TRACE ]; then
        rm -f trace/*
        sync
        sleep 5 
    fi

    for CPU in $NCPU; do
        exec_platform $1 $2 $CPU 
        sleep $CPU # time to free memory and erase completly residual files from the system 
    done
}

function show_help
{
   echo ""
   echo "Usage: ./bunny_test.sh -app \"<app_list>\" -ncpu \"<ncpu_list>\" -platform \"<platform_list>\" [OPTIONS] -"
   echo ""
   echo "         -app \"<app_list>\"             : Simulate or Generate applications."
   echo "         -ncpu \"<ncpu_list>\"           : Number of plataform's simulatior."
   echo "         -compile                      : Compile applications."
   echo "         -execute                      : Execute applications."
   echo "         -platform \"<platform_list>\"   : Set platform."
   echo "         -clean_after                  : Clean trace files after execution."
   echo "         -clean_before                 : Clean trace files before execution."
   echo ""
   exit 1;
}


if [ $# == 0 ]; then
    echo "ERROR: You must define parameters"
    show_help
fi

while [[ $# > 0 ]]; do
    key="$1"
    shift
    case $key in
    -app)
        if [ -z $1 ]; then 
           echo "ERROR: Empty application list"
           show_help 
        fi
        APPLICATIONS=$1
        shift
        ;;
    -ncpu)
        if [ -z $1 ]; then 
           show_help 
        fi
        NCPU=$1
        shift
        ;;
    -compile)
        GEN=1
        ;;
    -execute)
        EXEC=1
        ;;
    -platform)
        if [ -z $1 ]; then
           echo "ERROR: Empty platform list"
           show_help 
        fi
        PLATFORMS=$1
        shift
        ;;
    -clean_after)
        CLEAN_TRACE=1
      uu;;
    -clean_before)
        REMOVE_TRACE=1
        ;;
    -?|*)
        echo "ERROR: $1 is not a parameter"
        show_help
        ;;
    esac
done

# Consistency check
if [[ -z $EXEC && -z $GEN ]]; then
    echo "ERROR: You must configure either -execute or -compile"
    show_help
fi

if [[ ! -z $EXEC ]]; then
    if [[ -z $PLATFORMS || -z $NCPU || -z $APPLICATIONS  ]]; then
        echo "ERROR: You must configure all -app, -platform and -ncpu"
        show_help
    fi
fi


mkdir -p trace

if [ ! -z $GEN ]; then
   echo "Generating applications/platforms/qemu"

#   gen_dinner
#  gen_parallel
#   gen_barnes
#   gen_ocean
#  gen_waters 
  gen_watern 
#   gen_qemu
#   gen_platform
fi

if [ ! -z $EXEC ]; then
echo "Executing..."

for APP in $APPLICATIONS; do
    if [ -z $PLATFORMS ]; then 
        echo "Platforms were not configured."
        show_help
    fi
    for PLAT in $PLATFORMS; do
        set_platform $PLAT
        echo "NCPU;CLK;SIM;TOTAL;SIZE;DBT;TLM;#$PLAT;$APP" >> perf
        case $APP in
        dinner)
            APP_PATH=$DINNER_DIR
        ;;
        ParallelMjpeg)
            APP_PATH=$PARALLEL_DIR
        ;;
        ocean)
            APP_PATH=$OCEAN_DIR
        ;;
        barnes)
            APP_PATH=$BARNES_DIR
        ;;
        waterspatial)
            APP_PATH=$WATERS_DIR
        ;;
        waternsquared)
            APP_PATH=$WATERN_DIR
        ;;
        *)
            echo "Application $APP is not valid: try ParallelMjpeg Dinner ocean barnes waternsquared waterspatial"
            show_help
        ;;
        esac
        echo "$APP_PATH $APP"
        run_test $APP_PATH $APP
    done
done

fi





