project( sgui C )

cmake_minimum_required( VERSION 2.6 )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

#----------------------------------------------------------------------
# Compile configuration
#----------------------------------------------------------------------
if( NOT WIN32 )
  option( MEMCANVAS "Compile memory canvas implementation" ON )

  if( NOT MEMCANVAS )
    add_definitions( -DSGUI_NO_MEM_CANVAS )
  endif( )
endif( )

option( OPENGL "Compile with OpenGL(R) support" ON )

if( NOT OPENGL )
  add_definitions( -DSGUI_NO_OPENGL )
endif( )

option( NOFLOAT "Disable all use of floatingpoint data types" OFF )

if( NOFLOAT )
  add_definitions( -DSGUI_NO_FLOAT )
endif( )

#----------------------------------------------------------------------
# Compiler detection and configuration
#----------------------------------------------------------------------
if( CMAKE_COMPILER_IS_GNUCC OR MINGW )
  message( STATUS "Compiling with gcc" )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic"          )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow -Wwrite-strings" )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror"    )

  set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb" )
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -Ofast" )

  add_definitions( -D_GNU_SOURCE )
endif( )

if( MINGW )
  message( STATUS "Compiling with MinGW" )
  add_definitions( -D_WIN32_WINNT=0x0400 )
  add_definitions( -DWINVER=0x0410 )

  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows" )
endif( )

if( MSVC )
  message( STATUS "Compiling with MSVC" )

  add_definitions( /D_CRT_SECURE_NO_WARNINGS )
  add_definitions( /DUNICODE )

  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4005 /wd4996" )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /nologo"         )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Oi"             )

  if( CMAKE_CL_64 )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj" )
  endif( )

  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /fp:fast /arch:SSE2" )
endif( )

#----------------------------------------------------------------------
# OS detection
#----------------------------------------------------------------------
if( WIN32 )
  add_definitions( -DMACHINE_OS_WINDOWS )

  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    message( STATUS "Compiling for 64 bit Windows" )
  else( )
    message( STATUS "Compiling for 32 bit Windows" )
  endif( )
elseif( APPLE )
  message( ERROR "Mac OS X is not supported yet!" )
elseif( UNIX )
  add_definitions( -DMACHINE_OS_UNIX )
  message( STATUS "Compiling for a generic UNIX like operating system" )
else( )
  message( ERROR "Unknown, possibly unsupported operating system!" )
endif( )

#----------------------------------------------------------------------
# Configure dependencies
#----------------------------------------------------------------------
include_directories( ${CMAKE_SOURCE_DIR}/core/include/
                     ${CMAKE_SOURCE_DIR}/widgets/include/
                     ${CMAKE_SOURCE_DIR}/dialogs/include/
                   )

if( WIN32 )
  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( FREETYPE_DIR ${CMAKE_BINARY_DIR}/win_dep/x64 )
  else( )
    set( FREETYPE_DIR ${CMAKE_BINARY_DIR}/win_dep/x86 )
  endif( )

  include_directories( ${CMAKE_BINARY_DIR}/win_dep/include )
  link_directories( ${FREETYPE_DIR} )
  file( COPY ${FREETYPE_DIR}/libfreetype-6.dll
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin )

  set( SGUI_DEP ${SGUI_DEP} msimg32 )
  set( SGUI_DEP ${SGUI_DEP} freetype )
else( )
  find_package( Threads REQUIRED )
  set( SGUI_DEP ${SGUI_DEP} ${CMAKE_THREAD_LIBS_INIT} )
endif( )

if( UNIX )
  find_package( Freetype REQUIRED )
  set( SGUI_DEP ${SGUI_DEP} ${FREETYPE_LIBRARIES} )
  include_directories( ${FREETYPE_INCLUDE_DIRS} )

  find_package( X11 REQUIRED )
  set( SGUI_DEP ${SGUI_DEP} ${X11_X11_LIB} ${X11_Xrender_LIB} )
  include_directories( ${X11_INCLUDE_DIR} )
endif( )

if( OPENGL )
  find_package( OpenGL REQUIRED )
  set( SGUI_DEP ${SGUI_DEP} ${OPENGL_gl_LIBRARY} )
  include_directories( ${OPENGL_INCLUDE_DIR} )
endif( )

#----------------------------------------------------------------------
# Configure targets
#----------------------------------------------------------------------
add_subdirectory( core )
add_subdirectory( widgets )
add_subdirectory( dialogs )

add_library( sgui SHARED ${CORE_SRC} ${WIDGETS_SRC} ${DIALOGS_SRC} )
add_library( sgui_static STATIC ${CORE_SRC} ${WIDGETS_SRC} ${DIALOGS_SRC} )

target_link_libraries( sgui ${SGUI_DEP} )
target_link_libraries( sgui_static ${SGUI_DEP} )

add_subdirectory( extras )

