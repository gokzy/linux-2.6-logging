#!/bin/sh

if [ ! -c nsl0 ]; then
	sudo mknod nsl0 c 261 0
fi
if [ ! -L /usr/include/linux/net_stack_logger.h ]; then
	sudo ln -sf ../../include/linux/net_stack_logger.h /usr/include/linux
fi
gcc -o nsl nsl.c
