#--------------------------------------------------------------------
# This file is part of the sgui buildsystem. I, David Oberhollenzer,
# author of this file hereby place the contents of this file into
# the public domain.
#--------------------------------------------------------------------

if( UNIX )
  set( CORE_PLATFORM_SRC
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/canvas.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/font.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/keycode_translate.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/opengl.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/pixmap.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/platform.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/window.c )
elseif( WIN32 )
  set( CORE_PLATFORM_SRC
       ${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/canvas.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/font.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/opengl.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/pixmap.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/platform.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/window.c )
endif( )



set( CORE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/canvas.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/font_cache.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/pixmap.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/rect.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/skin.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/skin_default.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/utf8.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/widget.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/window.c

              ${CMAKE_CURRENT_SOURCE_DIR}/src/OpenGL/canvas_gl.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/OpenGL/canvas_gl_compat.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/OpenGL/canvas_gl_core.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/OpenGL/pixmap_gl.c

              ${CORE_PLATFORM_SRC}
     PARENT_SCOPE )

