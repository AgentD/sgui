#!/bin/sh

# Flags for the C compiler
CFLAGS="-ansi -pedantic -Werror -Wall -Wextra -Wshadow -Wwrite-strings
        -I./include -Ibuild/win_dep/include -mwindows -ggdb"

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

# Create a static library from object files. $1: object dir, $2: library name,
#                                            $3: prefix
#
# Simply glues all object files (*.o) within a source directory together to a
# single static library.
create_library( )
{
    echo -e "\e[31m***** creating static library $2 *****\e[0m"
    "$3"ar rcs build/lib/$1/$2 $(ls build/obj/$1/*.o)
    "$3"ranlib build/lib/$1/$2
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
SOURCE_WIDGETS="src/widgets/progress_bar.c src/widgets/static_text.c
                src/widgets/button.c src/widgets/image.c
                src/widgets/edit_box.c src/widgets/frame.c
                src/widgets/scroll_bar.c src/widgets/group_box.c
                src/widgets/tab.c"

SOURCE_OPENGL="src/OpenGL/canvas_gl_tex.c"

SOURCE_COMMON="src/widget.c src/font.c src/rect.c src/widget_manager.c
               src/skin.c src/filesystem.c src/canvas.c src/window.c
               src/screen.c $SOURCE_WIDGETS $SOURCE_OPENGL"

SOURCE_PLATFORM="src/WIN32/window.c src/WIN32/canvas.c src/WIN32/platform.c
                 src/WIN32/opengl.c"

# Test application sources
SOURCE_TEST="test/test.c test/test_gl.c"

# Platform specific dependencies
LIBS32="-Lbuild/win_dep/x86 -llibfreetype -lgdi32 -lopengl32"
LIBS64="-Lbuild/win_dep/x64 -llibfreetype -lgdi32 -lopengl32"

############################# Do the compilation #############################

# compile test apps. $1: compiler prefix, $2: obj dir, $3: dependencies,
compile_tests( )
{
    echo -e "\e[31m***** compiling test and demo programs *****\e[0m"

    compile_files "$1" "$SOURCE_TEST" "$2/test"

    list=$(ls build/obj/$2/test/*.o)

    for f in $list
    do
        name=$(basename $f ".o")

        echo -e "\e[31m***** linking $name$4 *****\e[0m"
        "$1"gcc $f -Lbuild/lib/$2 -lsgui $3 -o build/bin/$2/$name".exe"

        # On error, exit
        if [ $? -ne 0 ]; then
            exit 1
        fi
    done
}

do_build( )
{
    "$3"gcc > /dev/null 2>&1

    # Only attempt build if we actually have the toolchain
    if [ "$?" != "127" ]; then
        mkdir -p "build/obj/$1/test"
        mkdir -p "build/bin/$1"
        mkdir -p "build/lib/$1"

        echo -e "\e[0m * Compiling for $2, Windows operating system"

        compile_files "$3" "$SOURCE_COMMON"   "$1"
        compile_files "$3" "$SOURCE_PLATFORM" "$1" "w32_"

        create_library "$1" "libsgui.a" "$3"

        compile_tests "$3" "$1" "$4"
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
fi

