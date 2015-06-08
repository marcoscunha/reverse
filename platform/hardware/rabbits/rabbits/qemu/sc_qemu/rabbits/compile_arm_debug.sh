#!/bin/bash

HERE=`pwd`

failwith ()
{
    printf "$@"
    cd ${HERE}
    exit 1
}

cd ${HERE}/`dirname $0`/..

QEMU_DIR=`pwd`

OPTIONS="--enable-debug --disable-debug-tcg --disable-sparse --disable-sdl --disable-vnc --disable-xen --disable-brlapi --disable-vnc-tls --disable-vnc-sasl --disable-vnc-jpeg --disable-vnc-png --disable-vnc-thread --disable-curses --disable-curl --disable-fdt --disable-check-utests --disable-bluez --disable-slirp --disable-kvm --disable-nptl --disable-user --disable-linux-user --disable-darwin-user --disable-bsd-user --disable-guest-base --disable-pie --disable-uuid --disable-linux-aio --disable-attr --disable-blobs --disable-docs --disable-vhost-net --disable-spice --disable-smartcard --disable-smartcard-nss --disable-usb-redir --disable-guest-agent --audio-card-list= --audio-drv-list="

if test -n "$1"
then
    make distclean >& /dev/null
fi

if test ! -f config-host.mak
then
    echo "Configuring qemu ..."
    ./configure --target-list=arm-softmmu --prefix=${QEMU_DIR}/release --rabbits ${OPTIONS} || failwith "Configure qemu failed."
fi

echo "Compiling and installing qemu ..."
make || failwith "Compilation qemu failed."

cd ${HERE}

