#--------------------------------------------------------------------
# This file is part of the sgui buildsystem. I, David Oberhollenzer,
# author of this file hereby place the contents of this file into
# the public domain.
#--------------------------------------------------------------------

if( OPENGL )
  add_executable( gl0 gl0.c )
  add_executable( gl1 gl1.c )
  add_executable( gl2 gl2.c )
  add_executable( gl3 gl3.c )
  add_executable( texcanvas texcanvas.c )
  target_link_libraries( gl0 sgui )
  target_link_libraries( gl1 sgui )
  target_link_libraries( gl2 sgui )
  target_link_libraries( gl3 sgui )
  target_link_libraries( texcanvas sgui sguisubwm )

  if( UNIX )
    target_link_libraries( texcanvas m )
  endif( )
endif( )

if( WIN32 AND DIRECT3D9 )
  add_executable( d3d9_0 d3d9_0.c )
  add_executable( d3d9_1 d3d9_1.c )
  add_executable( d3d9_2 d3d9_2.c )
  add_executable( d3d9_3 d3d9_3.c )
  target_link_libraries( d3d9_0 sgui )
  target_link_libraries( d3d9_1 sgui )
  target_link_libraries( d3d9_2 sgui )
  target_link_libraries( d3d9_3 sgui )
endif( )

if( WIN32 AND DIRECT3D11 )
  add_executable( d3d11_0 d3d11_0.c )
  add_executable( d3d11_1 d3d11_1.c )
  add_executable( d3d11_2 d3d11_2.c )
  add_executable( d3d11_3 d3d11_3.c )
  target_link_libraries( d3d11_0 sgui d3dcompiler_43 )
  target_link_libraries( d3d11_1 sgui d3dcompiler_43 )
  target_link_libraries( d3d11_2 sgui d3dcompiler_43 )
  target_link_libraries( d3d11_3 sgui d3dcompiler_43 )
endif( )

add_executable( demo demo.c )
add_executable( events events.c )
add_executable( simple simple.c )
add_executable( clipboard clipboard.c )
add_executable( widget widget.c )
add_executable( dialogs dialogs.c )

target_link_libraries( demo sgui )
target_link_libraries( events sgui )
target_link_libraries( simple sgui )
target_link_libraries( clipboard sgui )
target_link_libraries( widget sgui )
target_link_libraries( dialogs sgui )

