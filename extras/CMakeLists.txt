#--------------------------------------------------------------------
# This file is part of the sgui buildsystem. I, David Oberhollenzer,
# author of this file hereby place the contents of this file into
# the public domain.
#--------------------------------------------------------------------

add_executable( demo demo.c )
add_executable( gl0 gl0.c )
add_executable( gl1 gl1.c )
add_executable( gl2 gl2.c )
add_executable( signalsandslots signalsandslots.c )
add_executable( simple simple.c )

target_link_libraries( demo sgui )
target_link_libraries( gl0 sgui )
target_link_libraries( gl1 sgui )
target_link_libraries( gl2 sgui )
target_link_libraries( signalsandslots sgui )
target_link_libraries( simple sgui )
