#!/bin/sh

CFLAGS="-ansi -W -Wall -Wextra -Wshadow -Wwrite-strings -I./include -ggdb"

MINGW="i586-mingw32msvc-gcc"
MINGW64="amd64-mingw32msvc-gcc"


$MINGW >/dev/null 2>&1
if [ "$?" == "127" ]; then
    MINGW="i486-mingw32-gcc"
fi

$MINGW64 >/dev/null 2>&1
if [ "$?" == "127" ]; then
    MINGW64="x86_64-w64-mingw32-gcc"
fi

# Common source code files
SOURCE_WIDGETS="src/widgets/progress_bar.c src/widgets/static_text.c
                src/widgets/button.c src/widgets/image.c
                src/widgets/edit_box.c src/widgets/frame.c
                src/widgets/scroll_bar.c src/widgets/group_box.c
                src/widgets/tab.c"

SOURCE_COMMON="test.c src/widget.c src/font.c src/rect.c src/widget_manager.c
                      src/skin.c src/link.c src/filesystem.c src/canvas.c
                      $SOURCE_WIDGETS"

# Platform specific source code files
SOURCE_X11="src/X11/window.c src/X11/keycode_translate.c src/X11/canvas.c"
SOURCE_WIN="src/WIN32/window.c src/WIN32/canvas.c"

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
        -Isrc/ -o build/a.out

    $MINGW $SOURCE_COMMON $SOURCE_WIN $CFLAGS $INCLUDE_WIN $LIBS_WIN32\
           -Isrc/ -o build/a.exe

    $MINGW64 $SOURCE_COMMON $SOURCE_WIN $CFLAGS $INCLUDE_WIN $LIBS_WIN64\
             -Isrc/ -o build/a64.exe
fi

