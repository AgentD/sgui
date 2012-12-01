#!/bin/sh

# Flags for the C compiler
CFLAGS="-ansi -pedantic -Werror -Wall -Wextra -Wshadow -Wwrite-strings
        -I./include -I/usr/include/freetype2 -lfreetype -ggdb"

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

# Create a static library from object files. $1: object dir, $2: library name
#
# Simply glues all object files (*.o) within a source directory together to a
# single static library.
create_library( )
{
    echo -e "\e[31m***** creating static library $2 *****\e[0m"
    ar rcs build/lib/$1/$2 $(ls build/obj/$1/*.o)
    ranlib build/lib/$1/$2
}

############################# source code files #############################

# Common source code files
SOURCE_WIDGETS="src/widgets/progress_bar.c src/widgets/static_text.c
                src/widgets/button.c src/widgets/image.c
                src/widgets/edit_box.c src/widgets/frame.c
                src/widgets/scroll_bar.c src/widgets/group_box.c
                src/widgets/tab.c"

SOURCE_OPENGL="src/OpenGL/canvas_gl_tex.c"

SOURCE_COMMON="src/widget.c src/font.c src/rect.c src/widget_manager.c
               src/skin.c src/filesystem.c src/canvas.c src/window.c
               src/screen.c $SOURCE_WIDGETS $SOURCE_OPENGL"

# Platform specific stuff
SOURCE_X11="src/X11/window.c src/X11/keycode_translate.c src/X11/canvas.c
            src/X11/platform.c src/X11/opengl.c"

LIBS_X11="-lX11 -lGL"

# Test application sources
SOURCE_TEST="test/test.c test/test_gl.c"

############################# Do the compilation #############################

# compile test apps. $1: additional flags, $2: obj dir, $3: app postfix
compile_tests( )
{
    echo -e "\e[31m***** compiling test and demo programs *****\e[0m"

    compile_files "$1" "$SOURCE_TEST" "$2/test"

    list=$(ls build/obj/$2/test/*.o)

    for f in $list
    do
        name=$(basename $f ".o")

        echo -e "\e[31m***** linking $name$3 *****\e[0m"
        gcc $CFLAGS $1 $f -Lbuild/lib/$2 -lsgui -o build/bin/$2/$name

        # On error, exit
        if [ $? -ne 0 ]; then
            exit 1
        fi
    done
}

do_build( )
{
    mkdir -p "build/obj/$1/test"
    mkdir -p "build/bin/$1"
    mkdir -p "build/lib/$1"

    echo -e "\e[0m * Compiling for $3, UNIX like system"

    compile_files "$2" "$SOURCE_COMMON" "$1"
    compile_files "$2" "$SOURCE_X11"    "$1" "x11_"

    create_library "$1" "libsgui.a"

    compile_tests "$2 $LIBS_X11" "$1"
}

# Do it
if [ ! -f ./compile.sh ]; then
    echo -e "\e[01;31mERROR\e[0m script must be run from it's directory!"
else
    if [ "$1" == "--local" ]; then
        uname -a | grep '64' > /dev/null 2>&1   # detect system type
        if [ "$?" == "0" ]; then                # '64' found in uname -a
            do_build "unix64" "-m64" "x86_64"
        else
            do_build "unix32" "-m32" "x86"
        fi
    else
        do_build "unix32" "-m32" "x86"
        do_build "unix64" "-m64" "x86_64"
    fi
fi

