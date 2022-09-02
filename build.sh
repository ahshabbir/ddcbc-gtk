#!/bin/sh
set -e

if [ -z $PREFIX ]; then
	PREFIX="/usr/local"
fi

sed -e "s,%PREFIX%,$PREFIX," ddcbc-gtk.desktop.in > ddcbc-gtk.desktop

gcc -g -pthread `pkg-config --cflags gtk+-3.0` -o ddcbc-gtk main.c `pkg-config --libs gtk+-3.0` -lddcutil
