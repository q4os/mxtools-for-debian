Qt6Gtk2 - GTK+2.0 integration plugins for Qt6

Official home page: https://github.com/trialuser02/qt6gtk2

Requirements:

- GNU Linux or FreeBSD
- qtbase >= 6.0.0 (with private headers)
- GTK+ 2.0
- libX11

Installation:

- Arch AUR
  https://aur.archlinux.org/packages/qt6gtk2/

- Source Code
```
  qmake PREFIX=<your installation path>
  make
  sudo make install
```

To change default installation root you should run the following
command:

`make install INSTALL_ROOT="custom root"`

Add line `export QT_QPA_PLATFORMTHEME=gtk2` to `~/.profile` and re-login.
Alternatively, create the file `/etc/X11/Xsession.d/100-qt6gtk2` with
the following line:

`export QT_QPA_PLATFORMTHEME=gtk2`

(`qt5gtk2` for compatibility with Qt5Gtk2, `qt6gtk2` is also a valid value)

Now restart X11 server to take the changes effect.

Files and directories:

`libqt6gtk2.so` - GTK+2.0 platform plugin
`libqt6gtk2-style.so` - GTK+2.0 style plugin

Attention!
Environment variable `QT_STYLE_OVERRIDE` should be removed before usage.
