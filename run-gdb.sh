#!/bin/sh

gdb-multiarch -ex "set architecture armv7" -ex "target remote :1234" $1
