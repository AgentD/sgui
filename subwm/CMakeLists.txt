#--------------------------------------------------------------------
# This file is part of the sgui buildsystem. I, David Oberhollenzer,
# author of this file hereby place the contents of this file into
# the public domain.
#--------------------------------------------------------------------

if( WIN32 AND DIRECT3D9 )
  set( SUBWM_OS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/d3d9_canvas.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/d3d9_wm.c )
endif( )

if( WIN32 AND DIRECT3D11 )
  set( SUBWM_OS_SRC ${SUBWM_OS_SRC}
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/d3d11_canvas.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/d3d11_wm.c )
endif( )

set( SUBWM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/tex_canvas.c
               ${CMAKE_CURRENT_SOURCE_DIR}/src/gl_canvas.c
               ${CMAKE_CURRENT_SOURCE_DIR}/src/gl_wm.c
               ${CMAKE_CURRENT_SOURCE_DIR}/src/ctx_window.c
               ${CMAKE_CURRENT_SOURCE_DIR}/src/ctx_wm.c
               ${CMAKE_CURRENT_SOURCE_DIR}/src/skin.c
               ${CMAKE_CURRENT_SOURCE_DIR}/src/ctx_mesh.c
               ${SUBWM_OS_SRC}
     PARENT_SCOPE )

