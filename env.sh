#!/bin/bash

export INSTALL_BASE=$PWD
export RABBITS_SYSTEMC=/opt/systemc-2.3.0
export NCPU=1

if [ -z $RABBITS_SYSTEMC ]; then
    echo "SystemC did not found. Configure RABBITS_SYSTEMC in env.sh file"
    exit 1
fi
