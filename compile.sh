#!/bin/sh

# Flags for the C compiler
CFLAGS="-ansi -W -Wall -Wextra -Wshadow -Wwrite-strings -I./include -ggdb"

# Compile a list of files. $1: compiler, $2: list of files,
#                          $3: object directory, $4: object prefix
#
# Only compiles files if the coresponding object file does not exist or is
# "older" (lower modification time stamp) than the source file
#
compile_files( )
{
    for f in $2
    do
        obj="$3/$4$(basename "$f" .c).o"
        objtime="0"
        srctime=$(stat -c %Y $f)

        # If the object file exists, get the last modification timestamp
        if [ -e $obj ]; then
            objtime=$(stat -c %Y $obj)
        fi

        # compile if object file is older than source code file
        if [ $objtime -lt $srctime ]; then
            $1 $CFLAGS -c $f -o $obj
        fi
    done
}

# Create a static library from object files. $1: object dir, $2: library name,
#                                            $3: ar binary, $4: ranlib bin
#
# Simply glues all object files (*.o) within a source directory together to a
# single static library.
create_library( )
{
    cur=$(pwd)              # store working directory
    cd $1                   # go to object file directory
    $3 rcs $2 $(ls *.o)     # "ar" all *.o files to an archive
    $4 $2                   # run "ranlib" on the resulting archive
    cd $cur                 # go back to stored working directory
}

################################# find mingw #################################
MINGW="i586-mingw32msvc-gcc"
MINGW_AR="i586-mingw32msvc-ar"
MINGW_RANLIB="i586-mingw32msvc-ranlib"

MINGW64="amd64-mingw32msvc-gcc"
MINGW64_AR="amd64-mingw32msvc-ar"
MINGW64_RANLIB="amd64-mingw32msvc-ranlib"

$MINGW >/dev/null 2>&1
if [ "$?" == "127" ]; then
    MINGW="i486-mingw32-gcc"
    MINGW_AR="i486-mingw32-ar"
    MINGW_RANLIB="i486-mingw32-ranlib"
fi

$MINGW64 >/dev/null 2>&1
if [ "$?" == "127" ]; then
    MINGW64="x86_64-w64-mingw32-gcc"
    MINGW64_AR="x86_64-w64-mingw32-ar"
    MINGW64_RANLIB="x86_64-w64-mingw32-ranlib"
fi

############################# source code files #############################

# Common source code files
SOURCE_WIDGETS="src/widgets/progress_bar.c src/widgets/static_text.c
                src/widgets/button.c src/widgets/image.c
                src/widgets/edit_box.c src/widgets/frame.c
                src/widgets/scroll_bar.c src/widgets/group_box.c
                src/widgets/tab.c"

SOURCE_COMMON="src/widget.c src/font.c src/rect.c src/widget_manager.c
               src/skin.c src/filesystem.c src/canvas.c src/window.c
               $SOURCE_WIDGETS"

# Platform specific source code files
SOURCE_X11="src/X11/window.c src/X11/keycode_translate.c src/X11/canvas.c
            src/X11/platform.c"
SOURCE_WIN="src/WIN32/window.c src/WIN32/canvas.c src/WIN32/platform.c"

####################### Platform specific dependencies #######################
INCLUDE_X11="-I/usr/include -I/usr/include/freetype2"
INCLUDE_WIN="-Ibuild/win_dep/include"

LIBS_X11="-lX11 -lfreetype"
LIBS_WIN32="-Lbuild/win_dep/x86 -llibfreetype -lgdi32"
LIBS_WIN64="-Lbuild/win_dep/x64 -llibfreetype -lgdi32"

########################## Test application sources ##########################
SOURCE_TEST="test/test.c"

############################# Do the compilation #############################

# compile test apps. $1: compiler, $2: obj dir, $3: dependencies,
#                    $4: app postfix
compile_tests( )
{
    compile_files "$1" "$SOURCE_TEST" "$2/test"

    list=$(ls $2/test/*.o)

    for f in $list
    do
        name=$(basename $f ".o")
        $1 $f -L$2 -lsgui $3 -o build/$name$4
    done
}

# Do it
if [ ! -f ./compile.sh ]; then
    echo "Error: script must be run from it's directory!"
else
    mkdir -p "build/obj/unix/test"
    mkdir -p "build/obj/win32/test"
    mkdir -p "build/obj/win64/test"

    ########## unix/X11 ##########
    compile_files "gcc $INCLUDE_X11" "$SOURCE_COMMON" "build/obj/unix"

    compile_files "gcc $INCLUDE_X11" "$SOURCE_X11" "build/obj/unix" "x11_"

    create_library "build/obj/unix" "libsgui.a" "ar" "ranlib"

    compile_tests "gcc" "build/obj/unix" "$LIBS_X11" "_unix"

    ########### win 32 ###########
    compile_files "$MINGW $INCLUDE_WIN" "$SOURCE_COMMON" "build/obj/win32"

    compile_files "$MINGW $INCLUDE_WIN" "$SOURCE_WIN" "build/obj/win32" "w32_"

    create_library "build/obj/win32" "libsgui.a" "$MINGW_AR" "$MINGW_RANLIB"

    compile_tests "$MINGW" "build/obj/win32" "$LIBS_WIN32" "_w32.exe"

    ########### win 64 ###########
    compile_files "$MINGW64 $INCLUDE_WIN" "$SOURCE_COMMON" "build/obj/win64"

    compile_files "$MINGW64 $INCLUDE_WIN" "$SOURCE_WIN" "build/obj/win64"\
                  "w32_"

    create_library "build/obj/win64" "libsgui.a" "$MINGW64_AR"\
                   "$MINGW64_RANLIB"

    compile_tests "$MINGW64" "build/obj/win64" "$LIBS_WIN64" "_w64.exe"
fi

