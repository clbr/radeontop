#!/bin/sh

ver=unknown

[ -n "`which git`" ] && ver=`git describe`

cat > version.h << EOF
#ifndef VER_H
#define VER_H

#define VERSION "$ver"

#endif
EOF
