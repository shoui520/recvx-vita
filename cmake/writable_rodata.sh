#!/bin/sh
# Compiler launcher: compile, then move the object's .rodata* into .data.
# The EE game code writes to its string literals and const tables (the PS2
# has no read-only data), which would fault in the Vita's read-only segment.
"$@" || exit $?
obj=
while [ $# -gt 0 ]; do
  if [ "$1" = "-o" ]; then obj=$2; break; fi
  shift
done
[ -n "$obj" ] || exit 0
OBJCOPY=${RECVX_OBJCOPY:-arm-vita-eabi-objcopy}
args=
for s in $(arm-vita-eabi-readelf -SW "$obj" | sed -n 's/^ *\[ *[0-9]*\] \(\.rodata[^ ]*\).*/\1/p'); do
  args="$args --rename-section $s=.data.ro${s#.rodata},alloc,load,data,contents"
done
[ -z "$args" ] || exec "$OBJCOPY" $args "$obj"
