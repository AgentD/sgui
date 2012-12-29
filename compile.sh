#!/bin/sh

# Flags for the C compiler
CFLAGS="-ansi -pedantic -Werror -Wall -Wextra -Wshadow -Wwrite-strings
        -Icore/include -I/usr/include/freetype2 $1"

# Compile a list of files. $1: additional compiler flags, $2: list of files,
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
            gcc $CFLAGS $1 -c $f -o $obj

            # On error, exit
            if [ $? -ne 0 ]; then
                exit 1
            fi
        fi
    done
}

############################# source code files #############################

# Common source code files
SOURCE_WIDGETS="core/src/widgets/progress_bar.c core/src/widgets/static_text.c
                core/src/widgets/button.c core/src/widgets/image.c
                core/src/widgets/edit_box.c core/src/widgets/frame.c
                core/src/widgets/scroll_bar.c core/src/widgets/group_box.c
                core/src/widgets/tab.c"

SOURCE_OPENGL="core/src/OpenGL/canvas_gl_tex.c"

SOURCE_COMMON="core/src/widget.c core/src/font.c core/src/rect.c
               core/src/widget_manager.c core/src/skin.c core/src/filesystem.c
               core/src/canvas.c core/src/window.c
               $SOURCE_WIDGETS $SOURCE_OPENGL"

# Platform specific stuff
SOURCE_X11="core/src/X11/window.c core/src/X11/keycode_translate.c
            core/src/X11/canvas.c core/src/X11/platform.c
            core/src/X11/opengl.c"

LIBS_X11="-lX11 -lGL -lfreetype"

############################# Do the compilation #############################

# Do it
if [ ! -f ./compile.sh ]; then
    echo -e "\e[01;31mERROR\e[0m script must be run from it's directory!"
else
    mkdir -p "build/obj/unix/test"
    mkdir -p "build/bin/unix"

    echo -e "\e[0m * Compiling for UNIX like system"
    compile_files "-fPIC" "$SOURCE_COMMON" "unix"
    compile_files "-fPIC" "$SOURCE_X11"    "unix" "x11_"

    echo -e "\e[31m***** creating shared library libsgui.so *****\e[0m"
    gcc $LIBS_X11 $1 -shared -Wl,-soname,libsgui.so $(ls build/obj/unix/*.o) \
        -o build/bin/unix/libsgui.so

    echo -e "\e[31m***** compiling test and demo programs *****\e[0m"

    FLAGS="$CFLAGS -Wl,-rpath,. -Lbuild/bin/unix -lsgui"

    gcc $FLAGS test/test.c -o build/bin/unix/test
    gcc $FLAGS -lGL test/test_gl.c -o build/bin/unix/test_gl
fi

