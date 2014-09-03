#--------------------------------------------------------------------
# This file is part of the sgui buildsystem. I, David Oberhollenzer,
# author of this file hereby place the contents of this file into
# the public domain.
#--------------------------------------------------------------------

set( WIDGETS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/button.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/edit_box.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/frame.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/group_box.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_view.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/image.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/progress_bar.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/scroll_bar.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/label.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/subview.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/tab.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/slider.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/pass_box.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/numeric_edit.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/color_picker.c

     PARENT_SCOPE )

