gcc -g -pthread `pkg-config --cflags gtk+-3.0` -o ddcbc-gtk-debug main.c `pkg-config --libs gtk+-3.0` -lddcutil
