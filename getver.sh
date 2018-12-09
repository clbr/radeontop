#!/bin/sh

ver=unknown

if [ -n "`which git`" ] && git rev-parse HEAD 2>/dev/null; then
  ver=`git describe`
fi

cat > include/version.h << EOF
#ifndef VER_H
#define VER_H

#define VERSION "$ver"

#endif
EOF
