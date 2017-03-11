#--------------------------------------------------------------------
# This file is part of the sgui buildsystem. I, David Oberhollenzer,
# author of this file hereby place the contents of this file into
# the public domain.
#--------------------------------------------------------------------

if( UNIX AND NOT APPLE )
  set( CORE_PLATFORM_SRC
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/canvas.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/xrender/canvas.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/xrender/pixmap.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/xlib/canvas.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/xlib/pixmap.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/font.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/keycode_translate.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/opengl.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/platform.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/X11/window.c )
elseif( WIN32 )
  set( CORE_PLATFORM_SRC
       ${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/font.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/opengl.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/platform.c
       ${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/direct3d9.cxx
       ${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/direct3d11.cxx
       ${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/window.c )
endif( )



set( CORE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/canvas.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/event.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/font_cache.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_cache.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/offscreen/canvas.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/offscreen/pixmap.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/model.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/pixmap.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/rect.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/skin.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/utf8.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/widget.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/window.c
              ${CMAKE_CURRENT_SOURCE_DIR}/src/skins/default.c

              ${CORE_PLATFORM_SRC}
     PARENT_SCOPE )

