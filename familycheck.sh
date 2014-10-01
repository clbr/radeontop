#!/bin/sh
# Check that the two places with family enums are in good condition.
# The next-best thing to the compiler enforcing it.

fam=`cut -d, -f3 include/r600_pci_ids.h | cut -d\) -f1 | sed -e 's@^ *@@' -e '/^$/d' | uniq`

for i in $fam; do
	grep -q $i family_str.c || echo $i missing from family_str.c
	grep -q $i include/radeontop.h || echo $i missing from include/radeontop.h
done
