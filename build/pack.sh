#!/bin/sh

# Make a zip for the Windows people
zip sgui.zip -r bin
zip sgui.zip -r font
zip sgui.zip -r include
zip sgui.zip -r lib

cd ..
zip build/sgui.zip -r doc
zip build/sgui.zip LICENSE
zip build/sgui.zip README
cd build

# And a .tar.gz for the rest of the world
tar czf sgui.tar.gz bin font include lib ../LICENSE ../README ../doc

