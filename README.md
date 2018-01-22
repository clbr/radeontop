RadeonTop
=========

View your GPU utilization, both for the total activity percent and individual blocks.

Requires access to /dev/dri/cardN files or /dev/mem (root privileges).

Supported cards
---------------

R600 and up, even Southern Islands should work fine.
Works with both the open drivers and AMD Catalyst.

For the amdgpu driver, only the mem path is currently supported -
for those cards, this means radeontop won't run on the default Ubuntu
kernels that block /dev/mem.

The total GPU utilization is also valid for OpenCL loads; the other blocks
are only useful in GL loads.

Translations
------------

If you'd like to translate RadeonTop to your own language, please go here:

https://translations.launchpad.net/radeontop

Need to install:
sudo apt-get install libpciaccess-dev
sudo apt-get install libncurses5-dev
sudo apt-get install libdrm-dev
sudo apt-get install libxcb-dri2-0-dev
