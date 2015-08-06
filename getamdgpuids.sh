#!/bin/sh -e
#
#	Extract the amdgpu pci ids from the kernel source.
#	Too bad no more easy libdrm list.
#

quit() {
	echo "Usage: $0 /path/to/kernel/src"
	exit
}

[ "$#" -lt 1 ] && quit
[ ! -d "$1" ] && quit

dir=$1/drivers/gpu/drm/amd/amdgpu

ids=`cpp $dir/amdgpu_drv.c 2>/dev/null | grep 0x1002 | sed -e 's@PCI_AN.*CHIP@CHIP@' \
	-e 's@|.*@@' -e 's@[{,}]@@g' -e 's@^ @@' -e 's@0x1002 @@' -e 's@CHIP_@@'`

fams=`echo "$ids" | awk '{print $2}' | uniq`

# clear previous ones
for i in $fams; do
	sed -i "/$i/d" include/r600_pci_ids.h
done

ids=`echo "$ids" | awk '{print "CHIPSET(" $1 ", " $1 ", " $2 ")"}'`

echo "$ids" >> include/r600_pci_ids.h
