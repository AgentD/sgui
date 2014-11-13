#--------------------------------------------------------------------
# This file is part of the sgui buildsystem. I, David Oberhollenzer,
# author of this file hereby place the contents of this file into
# the public domain.
#--------------------------------------------------------------------

set( SUBWM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/tex_canvas.c
               ${CMAKE_CURRENT_SOURCE_DIR}/src/gl_canvas.c
               ${CMAKE_CURRENT_SOURCE_DIR}/src/d3d9_canvas.c
     PARENT_SCOPE )

