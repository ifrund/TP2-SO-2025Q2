#!/bin/bash
if [[ "$1" = "gdb" ]]; then
qemu-system-x86_64 -s -S -hda Image/x64BareBonesImage.qcow2 -m 4096
else
qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 4096
fi