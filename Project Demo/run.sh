#!/bin/bash

SFML_LIB=$(realpath "inc/SFML/lib")
CSFML_LIB=$(realpath "inc/CSFML/lib")

if [ $# -gt 0 ]
then
if [ "$1" = "-r" -a -e window ];
then rm window
fi
fi
if ! [ -e window ]
then
echo "Compiling Project"
LD_LIBRARY_PATH="$SFML_LIB:$CSFML_LIB"
gcc -w -o window window.c inc/CSFML/lib/libcsfml-graphics.so.2.5.0 inc/CSFML/lib/libcsfml-window.so.2.5.0 inc/CSFML/lib/libcsfml-system.so.2.5.0 inc/btree/lib/libbt.a -I inc/CSFML/include
echo "Compiled"
fi
LD_LIBRARY_PATH="$SFML_LIB:$CSFML_LIB" ./window
