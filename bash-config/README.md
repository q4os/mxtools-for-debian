# mx-bashrc-config
A GUI tool written in Qt C++ to modify the bashrc

## Builing and running
 - Install the Qt5 development environment via MX Package Installer->Popular Applications->Development 
 - Get the zip file or git clone it
 - unzip the repository(if you downloaded the zip)
 - cd mx-bashrc-config
 - cd application
 - qmake
 - make
 - ./bash-config
## Dependencies

Qt5 Core

Qt5 GUI

Qt5 Widgets

## Usage and Features

### Suggesting Aliases

Using standard bash alias syntax: `alias ls="ls -l"` you can suggest aliases to the user.

File path easily changed in the global.h file `#define SUGGEST_ALIASES`



