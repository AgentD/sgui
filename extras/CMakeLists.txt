#--------------------------------------------------------------------
# This file is part of the sgui buildsystem. I, David Oberhollenzer,
# author of this file hereby place the contents of this file into
# the public domain.
#--------------------------------------------------------------------

if( OPENGL )
  add_executable( gl0 gl0.c )
  add_executable( gl1 gl1.c )
  add_executable( gl2 gl2.c )
  target_link_libraries( gl0 sgui )
  target_link_libraries( gl1 sgui )
  target_link_libraries( gl2 sgui )
endif( )

if( WIN32 AND DIRECT3D9 )
  add_executable( d3d9_0 d3d9_0.c )
  add_executable( d3d9_1 d3d9_1.c )
  target_link_libraries( d3d9_0 sgui )
  target_link_libraries( d3d9_1 sgui )
endif( )

if( WIN32 AND DIRECT3D11 )
  add_executable( d3d11_0 d3d11_0.c )
  target_link_libraries( d3d11_0 sgui d3dcompiler_43 )
endif( )

add_executable( demo demo.c )
add_executable( signalsandslots signalsandslots.c )
add_executable( simple simple.c )
add_executable( clipboard clipboard.c )
add_executable( widget widget.c )
add_executable( dialogs dialogs.c )

target_link_libraries( demo sgui )
target_link_libraries( signalsandslots sgui )
target_link_libraries( simple sgui )
target_link_libraries( clipboard sgui )
target_link_libraries( widget sgui )
target_link_libraries( dialogs sgui )

