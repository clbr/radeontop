#!/bin/sh

ver=unknown

[ -n "`which git`" ] && ver=`git describe`

cat > include/version.h << EOF
#ifndef VER_H
#define VER_H

#define VERSION "$ver"

#endif
EOF
