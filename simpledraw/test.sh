#!/bin/sh

OsName=$(uname -s)

case $OsName in
  Linux*)
    echo "Compiling for Linux"
    cc -o test test.c -lX11 -lm -lGL -I..
    ;;
  Darwin*)
    echo "Compiling for MacOS"
    cc -o test test.c -framework Cocoa -framework AudioUnit -framework OpenGL -I.. -g
    ;;
  *)
    echo "OS Not supported"
    exit 1
    ;;
esac
