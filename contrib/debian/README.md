
Debian
====================
This directory contains files used to package Woochaind/Woochain-qt
for Debian-based Linux systems. If you compile Woochaind/Woochain-qt yourself, there are some useful files here.

## Woochain: URI support ##


Woochain-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install Woochain-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your Woochain-qt binary to `/usr/bin`
and the `../../share/pixmaps/Woochain128.png` to `/usr/share/pixmaps`

Woochain-qt.protocol (KDE)

