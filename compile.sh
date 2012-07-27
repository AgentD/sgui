#!/bin/sh

CFLAGS="-ansi -pedantic -W -Wall -Wextra -Wshadow
        -Wwrite-strings -I./include"

MINGW="i586-mingw32msvc-gcc"

# Common source code files
SOURCE_COMMON="test.c src/widget.c src/progress_bar.c src/font_manager.c
                      src/widget_manager.c src/static_text.c"

# Platform specific source code files
SOURCE_X11="src/X11/window.c"
SOURCE_WIN="src/WIN32/window.c"

# Platform specific libraries
INCLUDE_X11="-I/usr/include -I/usr/include/freetype2"
INCLUDE_WIN="-Ibuild/win32_dep/include"

LIBS_X11="-lX11 -lfreetype"
LIBS_WIN="-Lbuild/win32_dep/lib -llibfreetype -lgdi32"

# Do the compilation
if [ ! -f ./compile.sh ]; then
    echo "Error: script must be run from it's directory!"
else
    gcc $SOURCE_COMMON $SOURCE_X11 $CFLAGS $INCLUDE_X11 $LIBS_X11\
        -o build/a.out

    $MINGW $SOURCE_COMMON $SOURCE_WIN $CFLAGS $INCLUDE_WIN $LIBS_WIN\
           -o build/a.exe

    cp build/win32_dep/bin/libfreetype-6.dll build/
fi

