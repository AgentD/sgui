#!/bin/sh

clean_build( )
{
    rm -r -- bin CMakeFiles core extras dialogs lib widgets tests
    rm -- CMakeCache.txt cmake_install.cmake Makefile CTestTestfile.cmake
}

windows_pack( )
{
    cp ./win_dep/FTL.TXT ./lib
    cp ./win_dep/FTL.TXT ./bin
    cp ./win_dep/$2/libfreetype.{a,la} ./lib/
    cp ./lib/libfreetype.a ./lib/libfreetype.lib
    cp ./lib/libsgui.dll.a ./lib/libsgui.dll.lib
    cp ./lib/sgui_static.a ./lib/sgui_static.lib

    zip sgui.zip -r -- ./bin ./font ./lib ./include ./doxydoc

    cd ..
    zip build/sgui.zip -r -- ./LICENSE ./README ./doc ./extras
    cd build

    mv sgui.zip ../sgui_$1.zip
}

# make sure we are in the right directory
if [ ! -f ./package.sh ]; then
    echo "Error: script must be run from it's directory!"
    exit -1
fi

cd build

if [ ! "$?" -eq "0" ]; then
    echo "Error: script must be run from it's directory!"
    exit -1
fi

# process arguments
build_all=false
build_src=false
build_local=false
build_win32=false
build_win64=false
build_src=false
print_help=true

while test $# -gt 0
do
    case "$1" in
        --all)
            build_all=true
            print_help=false
            ;;
        --src)
            build_src=true
            print_help=false
            ;;
        --local)
            build_local=true
            print_help=false
            ;;
        --win32)
            build_win32=true
            print_help=false
            ;;
        --win64)
            build_win64=true
            print_help=false
            ;;
        --src)
            build_src=true
            print_help=false
            ;;
        --help)
            ;;
        *)
            echo "unknown argument $1"
            ;;
    esac
    shift
done

if [ "$print_help" = true ]; then
    echo "supported flags:"
    echo "  --all    build all targets"
    echo "  --src    build source package"
    echo "  --local  build package for the local UNIX(R) system"
    echo "  --win32  build package for 32 bit Windows(R)"
    echo "  --win64  build package for 64 bit Windows(R)"
    exit 0
fi

# generate source package
if [ "$build_all" = true ] || [ "$build_src" = true ]; then
    clean_build
    rm -r -- include doxydoc
    cd ..
    tar czf sgui_src.tar.gz --exclude-vcs *
    zip sgui_src.zip -r -x\*~ -x.git -x\*.zip -x\*.tar.gz *
    cd build
fi

# generate header directory and doxygen documentation
clean_build
rm -r -- include doxydoc
mkdir include
mkdir doxydoc
cp ../core/include/*.h include
cp ../widgets/include/*.h include
cp ../dialogs/include/*.h include

cd ..
doxygen Doxyfile
cd build

cd doxydoc/latex
make
mv refman.pdf ../
cd ../
rm -r -- latex/
cd ../

# build for unix system
if [ "$build_all" = true ] || [ "$build_local" = true ]; then
    clean_build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make
    strip --strip-all ./bin/*

    uname -m | grep x86_64 > /dev/null

    tar czf sgui_unix.tar.gz $TARFLAGS bin font lib include doxydoc\
        ../extras ../doc ../LICENSE ../README

    if [ "$?" -eq "0" ]; then
        mv sgui_unix.tar.gz ../sgui_unix_x86_64.tar.gz
    else
        mv sgui_unix.tar.gz ../sgui_unix_x86.tar.gz
    fi
fi

# build for WinDOS 32 system
if [ "$build_all" = true ] || [ "$build_win32" = true ]; then
    clean_build
    cmake .. -DCMAKE_BUILD_TYPE=Release\
             -DCMAKE_TOOLCHAIN_FILE="../CMake/mingw.cmake"
    make
    i686-w64-mingw32-strip --strip-all ./bin/*
    windows_pack "win32" "x86"
fi

# build for WinDOS 64 system
if [ "$build_all" = true ] || [ "$build_win64" = true ]; then
    clean_build
    cmake .. -DCMAKE_BUILD_TYPE=Release\
             -DCMAKE_TOOLCHAIN_FILE="../CMake/mingw64.cmake"
    make
    x86_64-w64-mingw32-strip --strip-all ./bin/*
    windows_pack "win64" "x64"
fi

