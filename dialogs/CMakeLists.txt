#--------------------------------------------------------------------
# This file is part of the sgui buildsystem. I, David Oberhollenzer,
# author of this file hereby place the contents of this file into
# the public domain.
#--------------------------------------------------------------------

set( DIALOGS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/messagebox.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/messagebox_w32.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/messagebox_x11.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/color_dialog.c
     PARENT_SCOPE )

