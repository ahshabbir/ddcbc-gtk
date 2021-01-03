#!/bin/bash

# This script builds and installs ddcbc-gtk for all users. Root permissions
# are needed for this.

if [ `whoami` != "root" ]
then
    echo "Not root. Exiting..."
    exit
fi

echo "Building ddcbc-gtk..."
./build.sh
echo "Copying ddcbc-gtk to /usr/bin..."
cp ./ddcbc-gtk /usr/bin/ddcbc-gtk
echo "Copying desktop file to /usr/share/applications..."
cp ./ddcbc-gtk.desktop /usr/share/applications/ddcbc-gtk.desktop
echo "Done!"