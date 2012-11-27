#!/bin/sh

# Flags for the C compiler
CFLAGS="-ansi -pedantic -Werror -Wall -Wextra -Wshadow -Wwrite-strings
        -I./include -Ibuild/win_dep/include -mwindows -mwindows -ggdb"

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
    cur=$(pwd)              # store working directory
    cd build/obj/$1         # go to object file directory
    "$3"ar rcs $2 $(ls *.o) # "ar" all *.o files to an archive
    "$3"ranlib $2           # run "ranlib" on the resulting archive
    cd $cur                 # go back to stored working directory
}

################################# find mingw #################################
PREFIX="i586-mingw32msvc-"
PREFIX64="amd64-mingw32msvc-"

"$PREFIX"gcc >/dev/null 2>&1
if [ "$?" == "127" ]; then
    PREFIX="i486-mingw32-"
fi

"$PREFIX64"gcc >/dev/null 2>&1
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

####################### Platform specific dependencies #######################
LIBS32="-Lbuild/win_dep/x86 -llibfreetype -lgdi32 -lopengl32"
LIBS64="-Lbuild/win_dep/x64 -llibfreetype -lgdi32 -lopengl32"

########################## Test application sources ##########################
SOURCE_TEST="test/test.c test/test_gl.c"

############################# Do the compilation #############################

# compile test apps. $1: compiler prefix, $2: obj dir, $3: dependencies,
#                    $4: app postfix
compile_tests( )
{
    echo -e "\e[31m***** compiling test and demo programs *****\e[0m"

    compile_files "$1" "$SOURCE_TEST" "$2/test"

    list=$(ls build/obj/$2/test/*.o)

    for f in $list
    do
        name=$(basename $f ".o")

        echo -e "\e[31m***** linking $name$4 *****\e[0m"
        "$1"gcc $f -Lbuild/obj/$2 -lsgui $3 -o build/$name$4

        # On error, exit
        if [ $? -ne 0 ]; then
            exit 1
        fi
    done
}

# Do it
if [ ! -f ./compile_mingw.sh ]; then
    echo -e "\e[01;31mERROR\e[0m script must be run from it's directory!"
else
    mkdir -p "build/obj/win32/test"
    mkdir -p "build/obj/win64/test"

    ########### win 32 ###########
    echo -e "\e[0m * Compiling for x86, Windows operating system"

    compile_files "$PREFIX" "$SOURCE_COMMON"   "win32"
    compile_files "$PREFIX" "$SOURCE_PLATFORM" "win32" "w32_"

    create_library "win32" "libsgui.a" "$PREFIX"

    compile_tests "$PREFIX" "win32" "$LIBS32" "_w32.exe"

    ########### win 64 ###########
    echo -e "\e[0m * Compiling for x86_64, Windows operating system"

    compile_files "$PREFIX64" "$SOURCE_COMMON"   "win64"
    compile_files "$PREFIX64" "$SOURCE_PLATFORM" "win64" "w32_"

    create_library "win64" "libsgui.a" "$PREFIX64"

    compile_tests "$PREFIX64" "win64" "$LIBS64" "_w64.exe"
fi

