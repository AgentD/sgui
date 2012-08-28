#!/bin/sh

CFLAGS="-ansi -W -Wall -Wextra -Wshadow -Wwrite-strings -I./include -ggdb"

MINGW="i586-mingw32msvc-gcc"
MINGW64="amd64-mingw32msvc-gcc"

if [ ! command -v $MINGW >/dev/null 2>&1 ]; then
    MINGW="i486-mingw32-gcc"
fi

if [ ! command -v $MINGW64 >/dev/null 2>&1 ]; then
    MINGW64="x86_64-w64-mingw32-gcc"
fi

# Common source code files
SOURCE_COMMON="test.c src/widget.c src/progress_bar.c src/canvas.c
                      src/widget_manager.c src/static_text.c src/button.c
                      src/skin.c src/link.c src/image.c src/radio_menu.c
                      src/edit_box.c"

# Platform specific source code files
SOURCE_X11="src/X11/window.c src/X11/keycode_translate.c"
SOURCE_WIN="src/WIN32/window.c"

# Platform specific libraries
INCLUDE_X11="-I/usr/include -I/usr/include/freetype2"
INCLUDE_WIN="-Ibuild/win_dep/include"

LIBS_X11="-lX11 -lfreetype"
LIBS_WIN32="-Lbuild/win_dep/x86 -llibfreetype -lgdi32"
LIBS_WIN64="-Lbuild/win_dep/x64 -llibfreetype -lgdi32"

# Do the compilation
if [ ! -f ./compile.sh ]; then
    echo "Error: script must be run from it's directory!"
else
    gcc $SOURCE_COMMON $SOURCE_X11 $CFLAGS $INCLUDE_X11 $LIBS_X11\
        -o build/a.out

    $MINGW $SOURCE_COMMON $SOURCE_WIN $CFLAGS $INCLUDE_WIN $LIBS_WIN32\
           -o build/a.exe

    $MINGW64 $SOURCE_COMMON $SOURCE_WIN $CFLAGS $INCLUDE_WIN $LIBS_WIN64\
             -o build/a64.exe
fi

