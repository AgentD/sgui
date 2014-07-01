#!/bin/sh

clean_build( )
{
    rm -r -- bin
    rm -r -- CMakeFiles
    rm -r -- core
    rm -r -- extras
    rm -r -- include
    rm -r -- doxydoc
    rm -r -- lib
    rm -r -- widgets
    rm -- CMakeCache.txt cmake_install.cmake Makefile
}

# make sure we are in the right directory
if [ ! -f ./package.sh ]; then
    echo "Error: script must be run from it's directory!"
    exit
fi

cd build

if [ ! "$?" -eq "0" ]; then
    echo "Error: script must be run from it's directory!"
    exit;
fi

# generate source package
clean_build
cd ..
tar czf sgui_src.tar.gz --exclude-vcs *
zip sgui_src.zip -r -x\*~ -x.git -x\*.zip -x\*.tar.gz *
cd build

# generate SDK package
clean_build
mkdir include
mkdir doxydoc
cp ../core/include/*.h include
cp ../widgets/include/*.h include
cp ../dialogs/include/*.h include

cd ..
doxygen Doxyfile
cd build

tar czf sgui_dev.tar.gz doxydoc include ../doc ../extras ../LICENSE ../README

zip sgui_dev.zip -r doxydoc
zip sgui_dev.zip -r include

cd ..
zip build/sgui_dev.zip -r extras
zip build/sgui_dev.zip -r doc
zip build/sgui_dev.zip LICENSE
zip build/sgui_dev.zip README
cd build

mv sgui_dev.zip ..
mv sgui_dev.tar.gz ..

# build for unix system
clean_build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
strip --strip-all ./bin/*

uname -m | grep x86_64 > /dev/null

tar czf sgui_unix.tar.gz $TARFLAGS bin font lib ../LICENSE ../README

if [ "$?" -eq "0" ]; then
  mv sgui_unix.tar.gz ../sgui_unix_x86_64.tar.gz
else
  mv sgui_unix.tar.gz ../sgui_unix_x86.tar.gz
fi

# build for WinDOS 32 system
clean_build
cmake .. -DCMAKE_TOOLCHAIN_FILE="../CMake/mingw.cmake"
make

zip sgui.zip -r bin
zip sgui.zip -r font
zip sgui.zip -r lib

cd ..
zip build/sgui.zip LICENSE
zip build/sgui.zip README
cd build

mv sgui.zip ../sgui_win32.zip

# build for WinDOS 64 system
clean_build
cmake .. -DCMAKE_TOOLCHAIN_FILE="../CMake/mingw64.cmake"
make

zip sgui.zip -r bin
zip sgui.zip -r font
zip sgui.zip -r lib

cd ..
zip build/sgui.zip LICENSE
zip build/sgui.zip README
cd build

mv sgui.zip ../sgui_win64.zip

