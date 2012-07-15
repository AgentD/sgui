#!/bin/sh

CFLAGS="-ansi -pedantic -W -Wall -Wextra -Wconversion -Wshadow
        -Wcast-qual -Wwrite-strings -I./include"

MINGW="i586-mingw32msvc-gcc"

# Common source code files
SOURCE_COMMON="test.c src/widget.c"

# Platform specific source code files
SOURCE_X11="src/X11/window.c"
SOURCE_WINDOSE="src/WIN32/window.c"

# Platform specific libraries
LIBS_X11="-lX11"
LIBS_WINDOSE="-lgdi32"

# Do the compilation
if [ ! -f ./compile.sh ]; then
    echo "Error: script must be run from it's directory!"
else
    gcc $SOURCE_COMMON $SOURCE_X11 $CFLAGS $LIBS_X11

    $MINGW $SOURCE_COMMON $SOURCE_WINDOSE $CFLAGS $LIBS_WINDOSE
fi

