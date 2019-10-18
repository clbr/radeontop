# RadeonTop

View your GPU utilization, both for the total activity percent and individual blocks.

Requires access to /dev/dri/cardN files or /dev/mem (root privileges).

## Installing Compiletime Prerequisites

*   clang or gcc
*   Development headers for
    *   libdrm
    *   libncurses
    *   libpciaccess
    *   libxcb
*   pkgconf

Under Ubuntu:

    sudo apt install --yes \
        clang \
        gcc \
        libdrm-dev \
        libncurses-dev \
        libpciaccess-dev \
        libxcb1-dev \
        pkgconf

## Building RadeonTop

RadeonTop can be build by simply running:

    make

## Build options

Build options can be specified to having the following variables being set to "1"

    nls     enable translations, default on
    debug   enable debug symbols, default off
    nostrip disable stripping, default off
    plain   apply neither gcc's -g nor -s.
    xcb     enable libxcb to run unprivileged in Xorg, default on
    amdgpu  enable amdgpu usage reporting, default auto (requires libdrm >= 2.4.63)

Example:

    make amdgpu=1 xcb=1

This will build RadeonTop with amdgpu reporting and xcb support.

## Using RadeonTop

Simply start RadeonTop and it auto-selects the first supported GPU:

    ./radeontop

Running RadeonTop on a bus 0f:

    ./radeontop -b 0f

Writing values to stdout instead of showing a GUI:

    ./radeontop -d -

Getting all options:

    ./radeontop --help

## Supported Cards

R600 and up, even Southern Islands should work fine.
Works with both the open drivers and AMD Catalyst.

For the Catalyst driver, only the mem path is currently supported - this
means it won't run on the default Ubuntu kernels that block /dev/mem.

The total GPU utilization is also valid for OpenCL loads; the other blocks
are only useful in GL loads.

## Translating RadeonTop

If you'd like to translate RadeonTop to your own language, please go here:

https://translations.launchpad.net/radeontop
