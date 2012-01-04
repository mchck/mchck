#!/bin/sh

#
# Extract toolchain headers/sources from Nuvoton and CooCox code drops
#

set -e

if [ $# -ne 2 ]
then
	echo "usage: $0 <nuc source dir> <build destionation>" >&2
	exit 1
fi

vendor=Nuvoton
platform=NUC1xx

base=$(dirname "$0")
from=$1; shift
dest=$1; shift

cmsis=$from/CMSIS/CM0
core=$cmsis/CoreSupport
dev=$cmsis/DeviceSupport/$vendor/$platform
sysplt=$from/${vendor}Platform_Keil

i=$dest/include
c=$dest/crt0

install_source() {
    f=$1; shift
    d=$1; shift
    extrased=$1
    fn=$(basename "$f")
    o=$(mktemp --tmpdir="$d")

    sed -Ee "$extrased"'/^[[:space:]]*#[[:space:]]*include/y,\\,/,' < $f > $o
    mv "$o" "$d/$fn"
}

mkdir "$dest"
mkdir "$i"
mkdir "$c"

for inc in "$core/core_cm0.h" "$dev/$platform.h"
do
    echo "INC $inc"
    install_source "$inc" "$i"
done

echo "INC $dev/system_$platform.h"
install_source "$dev/system_$platform.h" "$i" "/^[[:space:]]*#[[:space:]]*define[[:space:]]+__XTAL[[:space:]]*/d;"


for dir in System Driver USB
do
	mkdir "$i/$dir"
	for inc in "$sysplt/Include/$dir/"*
	do
		echo "INC $inc"
		install_source "$inc" "$i/$dir"
	done
done


for crt in "$base/startup_$platform.c" "$core/core_cm0.c" "$dev/system_$platform.c"
do
    echo "CRT $crt"
    install_source "$crt" "$c"
done


mkdir "$dest/lib"

for lib in "$sysplt/Src"/*/*.c
do
	echo "LIB $lib"
        install_source "$lib" "$dest/lib"
done


echo "MK nuc1xx.mk"
cp "$base/nuc1xx.mk" "$dest"

mkdir "$dest/ld"

for ld in "$base"/link.ld "$base/ld"/*.ld
do
	echo "LD $ld"
	cp "$ld" "$dest/ld"
done
