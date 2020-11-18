#!/bin/sh

# Keep ddcbc-api submodule update
git submodule update --remote
gcc -g -pthread `pkg-config --cflags gtk+-3.0` -o ddcbc-gtk main.c `pkg-config --libs gtk+-3.0` -lddcutil
