#!/bin/bash
set -e

# This script builds and installs ddcbc-gtk into PREFIX.
# If PREFIX is not specified, it defaults to /usr/local.

if [ -z $PREFIX ]; then
	PREFIX="/usr/local"
fi

echo "Building ddcbc-gtk..."
./build.sh
echo "Copying ddcbc-gtk to $PREFIX/bin..."
cp ./ddcbc-gtk "$PREFIX/bin/ddcbc-gtk"
echo "Copying desktop file to $PREFIX/share/applications..."
cp ./ddcbc-gtk.desktop "$PREFIX/share/applications/ddcbc-gtk.desktop"
echo "Done!"
