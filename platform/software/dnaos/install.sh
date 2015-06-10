#!/bin/sh
export APES_ROOT=/home/cunha/devel/TIMA/reverse/reverse/platform/software/dnaos
export APES_COMPONENT_PATH=$APES_ROOT/Components


export RUBYLIB=$RUBYLIB:$APES_ROOT/Tools/Ruby/lib
export PATH=$PATH:$APES_ROOT/Tools/Ruby/bin
export PATH=$PATH:$APES_ROOT/Tools/Shell/bin


if [ "x$APES_ROOT" = "x" ] ; then
  echo "The \$APES_ROOT environment variable is not defined."
elif [ ! -d $APES_ROOT ] ; then
  echo "\$APES_ROOT invalid."
else
  export APES_PATH=$APES_ROOT/Components
  export RUBYLIB=$RUBYLIB:$APES_ROOT/Tools/Ruby/lib
  export PATH=$PATH:$APES_ROOT/Tools/Ruby/bin
  export PATH=$PATH:$APES_ROOT/Tools/Shell/bin

  if [ -e $APES_ROOT/Toolchains ] ; then
    for i in $(\ls $APES_ROOT/Toolchains) ; do
      export PATH=$APES_ROOT/Toolchains/$i/bin:$PATH
      export MANPATH=$APES_ROOT/Toolchains/$i/man:$MANPATH
      export INFOPATH=$APES_ROOT/Toolchains/$i/info:$INFOPATH
    done ;
  fi
fi 
