#!/bin/bash

if [ -t 1 ] ; then # stdout in terminal
    case "$(tput colors)" in
        256)
            INFO_LINE_COLOR="$(tput sgr0)"                  # White
            INFO_TEXT_COLOR="$(tput bold)$(tput setaf 46)"  # Green

            ERROR_LINE_COLOR="$(tput bold)$(tput setaf 9)"  # Red
            ERROR_TEXT_COLOR="$(tput bold)$(tput setaf 9)"  # Red
            NO_COLOR="$(tput sgr0)"
            ;;
        *)
            INFO_LINE_COLOR="$(tput bold)$(tput setaf 3)"   # Yellow
            INFO_TEXT_COLOR="$(tput bold)$(tput setaf 6)"   # Blue

            ERROR_LINE_COLOR="$(tput bold)$(tput setaf 2)"  # Red
            ERROR_TEXT_COLOR="$(tput bold)$(tput setaf 2)"  # Red
            NO_COLOR="$(tput sgr0)"
            ;;
    esac
else # stdout in file !!!
    INFO_LINE_COLOR=""
    INFO_TEXT_COLOR=""

    ERROR_LINE_COLOR=""
    ERROR_TEXT_COLOR=""

    NO_COLOR=""
fi

print_step()
{
    echo -en ${INFO_LINE_COLOR}
    echo "==================================="
    echo -en ${INFO_TEXT_COLOR}
    echo " "$1
    echo -en ${INFO_LINE_COLOR}
    echo "==================================="
    echo -en ${NO_COLOR}
}

. ./env.sh

print_step "Compiling Decopus/Trace Library"

make -C lib/decopus/trace

print_step "Installing Decopus/Trace Library"

ln -fvs lib.trace lib/decopus/trace/lib 
cd $INSTALL_BASE

print_step "Compiling DebugHelper Library (DWARF)"
cd lib/debughelper
source install.sh
cd debughelperlib
make
cd $INSTALL_BASE


print_step "Compiling Scherlock/Delorean Reverse Debugger Tool"

cd tool/sherlock
source install.sh
make
cd -

print_step "Compiling RABBITS - QEMU"
source platform/hardware/rabbits/rabbits_env
cd platform/hardware/rabbits/rabbits/qemu/scripts
./install.sh
cd $INSTALL_BASE

print_step "Installing Tools ..."
cd platform/hardware/rabbits/rabbits/tools
make 
cd $INSTALL_BASE

print_step "Compiling RABBITS - Bunny Platform"
make -C platform/hardware/rabbits/platforms/bunny

print_step "Getting cross-toolchain"
cd lib
git clone -b dna-xtools --recursive git://git-sls.imag.fr/gnu\_tools/SLSxtools.git
cd SLSxtools
./build\_xtools arm Toolchains
cd $INSTALL_BASE

print_step "Configuring DNAOS Environment"
ln -fvs ../../../../lib/SLSxtools/Toolchains/ platform/software/dnaos/Toolchains/arm-dnaos

print_step "Compiling Applications - ParallelMJPEG - $NCPU cpu(s)"
cd platform/software/parallelmjpeg
./compile_test.sh -ncpu $NCPU
cd $INSTALL_BASE

print_step "Compiling Applications - Splash-2 - Ocean - $NCPU cpu(s)"
cd platform/software/splash-2/codes/apps/ocean/contiguous_partitions/
./compile_test.sh $NCPU
cd $INSTALL_BASE

print_step "Compiling Applications - Splash-2 - Water-Nsquered - $NCPU cpu(s)"
cd platform/software/splash-2/codes/apps/water-nsquared
./compile_test.sh $NCPU
cd $INSTALL_BASE

print_step "Compiling Applications - Splash-2 - Water-Spatial - $NCPU cpu(s)"
cd platform/software/splash-2/codes/apps/water-spatial
./compile_test.sh $NCPU
cd $INSTALL_BASE


