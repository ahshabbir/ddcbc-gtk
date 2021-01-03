# ddcbc-gtk
A GTK Interface for controlling brightness through the DDC/CI protocol. It includes support for controlling the brightness of multiple displays separetly.

# Dependencies
- ddcutil library package (typically libddcutil or libddcutil-dev if not already installed with ddcutil)
- GTK 3.0

# Setup

1. Firstly, ensure that all the dependancies are installed.

Arch Linux:
`pacman -S --needed ddcutil gtk3`

TODO: Add other distributions to this!

2. Ensure that your user has access to the i2c devices:
https://www.ddcutil.com/i2c_permissions/

3. Clone this repo along with its submodules:
`git clone --recurse-submodules https://github.com/ahshabbir/ddcbc-gtk.git`

4. Change into the ddcbc-gtk directory and execute build.sh to build this application:
```
cd ddcbc-gtk
./build.sh
```

5. This should result in a ddcbc-gtk binary that you can execute to contol the brightness:
`./ddcbc-gtk`

To install this binary for all users execute 'install.sh' as root:
`sudo ./install.sh`

You should now be able to find ddcbc-gtk or DDC Brightness Control in your application menu under utilities.

# Known Issues/Todos

1. Unaligned seperator between each monitor.
2. Find/create a logo.
3. Add directions for installing deps on other distros.
