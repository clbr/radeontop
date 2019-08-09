RadeonTop
=========

View your GPU utilization, both for the total activity percent and
individual blocks.

Requires access to /dev/dri/cardN files or /dev/mem (root privileges).

Supported cards
---------------

R600 and up, even Southern Islands should work fine.
Works with both the open drivers and AMD Catalyst.

For the Catalyst driver, only the mem path is currently supported - this
means it won't run on the default Ubuntu kernels that block /dev/mem.

The total GPU utilization is also valid for OpenCL loads; the other blocks
are only useful in GL loads.

Translations
------------

If you'd like to translate RadeonTop to your own language, please go here:

https://translations.launchpad.net/radeontop

Running
-------

#### Prerequisites

* libdrm
* libncurses
* libpciaccess
* libxcb


Simply start radeontop and it auto-selects the first supported GPU:

    ./radeontop


Running radeontop on a bus 0f:

    ./radeontop -b 0f


Writing values to stdout instead of showing a GUI:

    ./radeontop -d -


Getting all options:

    ./radeontop --help


Building
--------

#### Prerequisites
* all run time prerequisites with dev files
* gcc / clang
* pkgconf

### Building
If all prerequisites are fullfilled, it can be build by simply running:

    make

#### Build options

Build options can be specified to having the following variables being set to "1"

    nls     enable translations, default on
    debug   enable debug symbols, default off
    nostrip disable stripping, default off
    plain   apply neither gcc's -g nor -s.
    xcb     enable libxcb to run unprivileged in Xorg, default on
    amdgpu  enable amdgpu usage reporting, default auto (requires libdrm >= 2.4.63)


Example:

    make amdgpu=1 xcb=1

This will build radeontop with amdgpu reporting and xcb support.
