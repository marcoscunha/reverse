#!/bin/bash
export APES_CC_FLAGS="-Wall -Wno-format -std=c99"
export APES_AS_FLAGS="-Wall -Werror -Wno-format -std=c99"

#export APES_CC_FLAGS="-Wall -Wno-format -std=c99 -DWRITEBACK"
#export APES_AS_FLAGS="-Wall -Werror -Wno-format -std=c99 -DWRITEBACK"
export MJPEG_CC_FLAGS="-DNB_DECODER=1 -DDISPATCH_TIME -DINFO -DVERBOSE"

APES_CC_OPTIMIZATIONS="-g3 -mlittle-endian -O3 -march=armv6zk"
# -march=armv6zk -mfpu=fpa
export APES_CC_OPTIMIZATIONS

APES_AS_OPTIMIZATIONS="-g3 -mlittle-endian -O3 -march=armv6zk"
# -march=armv6zk  -mfpu=fpa
export APES_AS_OPTIMIZATIONS

export APES_ASSEMBLER="arm-sls-dnaos-gcc"
export APES_COMPILER="arm-sls-dnaos-gcc"
export APES_LINKER="arm-sls-dnaos-gcc"
export APES_LINKER_FLAGS="-mlittle-endian -T$PWD/ldscript -march=armv6zk"
#  -march=armv6zk -mfpu=fpa

# export DNACORE_CC_FLAGS="-DDNA_ENABLE_LOG=VERBOSE_LEVEL"
unset DNACORE_CC_FLAGS
