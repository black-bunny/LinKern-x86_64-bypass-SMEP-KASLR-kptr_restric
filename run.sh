#!/bin/bash -p

qemu-system-x86_64 \
    -s \
    -m 64M \
    -cpu kvm64,+smep \
    -nographic \
    -kernel bzImage_KASLRON \
    -append 'console=ttyS0 loglevel=3 oops=panic panic=1' \
    -monitor /dev/null \
    -initrd initramfs.img
