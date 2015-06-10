if [ -z $INSTALL_BASE ]; then
    echo "Environment not configured: INSTALL_BASE is null"
    exit 1
fi

export HWETRACE=$INSTALL_BASE/lib/decopus/trace/
export DEBUGHELPER=$INSTALL_BASE/lib/debughelper/debughelperlib


