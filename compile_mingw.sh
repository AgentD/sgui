#!/bin/sh

# Flags for the C compiler
CFLAGS="-ansi -pedantic -Werror -Wall -Wextra -Wshadow -Wwrite-strings
        -Icore/include -Iwidgets/include -Ibuild/win_dep/include -mwindows
        -DWINVER=0x0410 $1"

# Compile a list of files. $1: prefix, $2: list of files,
#                          $3: object directory, $4: object prefix
#
# Only compiles files if the coresponding object file does not exist or is
# "older" (lower modification time stamp) than the source file
compile_files( )
{
    for f in $2
    do
        obj="build/obj/$3/$4$(basename "$f" .c).o"
        objtime="0"
        srctime=$(stat -c %Y $f)

        # If the object file exists, get the last modification timestamp
        if [ -e $obj ]; then
            objtime=$(stat -c %Y $obj)
        fi

        # compile if object file is older than source code file
        if [ $objtime -lt $srctime ]; then
            echo -e "\e[32m$f\e[0m"
            "$1"gcc $CFLAGS -c $f -o $obj

            # On error, exit
            if [ $? -ne 0 ]; then
                exit 1
            fi
        fi
    done
}

################################# find mingw #################################
PREFIX="i586-mingw32msvc-"
PREFIX64="amd64-mingw32msvc-"

"$PREFIX"gcc > /dev/null 2>&1
if [ "$?" == "127" ]; then
    PREFIX="i486-mingw32-"
fi

"$PREFIX64"gcc > /dev/null 2>&1
if [ "$?" == "127" ]; then
    PREFIX64="x86_64-w64-mingw32-"
fi

############################# source code files #############################
SOURCE_WIDGETS="widgets/src/progress_bar.c widgets/src/static_text.c
                widgets/src/button.c widgets/src/image.c
                widgets/src/edit_box.c widgets/src/frame.c
                widgets/src/scroll_bar.c widgets/src/group_box.c
                widgets/src/tab.c widgets/src/subview.c"

SOURCE_OPENGL="core/src/OpenGL/canvas_gl_tex.c"

SOURCE_COMMON="core/src/widget.c core/src/rect.c core/src/widget_manager.c
               core/src/skin.c core/src/canvas.c core/src/window.c
               core/src/utf8.c $SOURCE_WIDGETS $SOURCE_OPENGL"

SOURCE_PLATFORM="core/src/WIN32/window.c core/src/WIN32/canvas.c
                 core/src/WIN32/platform.c core/src/WIN32/opengl.c
                 core/src/WIN32/font.c core/src/WIN32/pixmap.c"

# Platform specific dependencies
LIBS32="-Lbuild/win_dep/x86 -llibfreetype -lgdi32 -lmsimg32 -lopengl32"
LIBS64="-Lbuild/win_dep/x64 -llibfreetype -lgdi32 -lmsimg32 -lopengl32"

############################# Do the compilation #############################

do_build( )
{
    "$3"gcc > /dev/null 2>&1

    # Only attempt build if we actually have the toolchain
    if [ "$?" != "127" ]; then
        mkdir -p "build/obj/$1/test"
        mkdir -p "build/bin/$1"
        mkdir -p "build/lib/$1"
        mkdir -p "build/include"

        echo -e "\e[0m * Compiling for $2, Windows operating system"
        compile_files "$3" "$SOURCE_COMMON"   "$1"
        compile_files "$3" "$SOURCE_PLATFORM" "$1" "w32_"

        echo -e "\e[31m***** creating dynamic library sgui.dll *****\e[0m"
        "$3"gcc $(ls build/obj/$1/*.o) $4 -shared \
        -Wl,--out-implib,build/lib/$1/libsgui.a \
        -o build/bin/$1/sgui.dll

        cp build/lib/$1/libsgui.a build/lib/$1/sgui.lib

        echo -e "\e[31m***** compiling test and demo programs *****\e[0m"

        FLAGS="-ansi -pedantic -Wall -Wextra -mwindows
               -Ibuild/include -Lbuild/lib/$1 -lsgui"

        "$3"gcc test/test.c $FLAGS -lopengl32 -o build/bin/$1/test.exe
        "$3"gcc test/test_gl.c $FLAGS -lopengl32 -o build/bin/$1/test_gl.exe
    fi
}

# Do it
if [ ! -f ./compile_mingw.sh ]; then
    echo -e "\e[01;31mERROR\e[0m script must be run from it's directory!"
else
    do_build "win32" "x86"    "$PREFIX"   "$LIBS32"
    do_build "win64" "x86_64" "$PREFIX64" "$LIBS64"

    cp -u build/win_dep/x86/libfreetype-6.dll build/bin/win32
    cp -u build/win_dep/x64/libfreetype-6.dll build/bin/win64

    cp -u core/include/*.h build/include/
    cp -u widgets/include/*.h build/include/
fi

