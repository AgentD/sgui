project( sgui C )

cmake_minimum_required( VERSION 2.6 )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin" )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin" )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/bin" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/bin" )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin" )

set( PROJECT_BRIEF "A small, simple gui library implemented in plain C89" )
set( PROJECT_VERSION "alpha" )

#----------------------------------------------------------------------
# Compile configuration
#----------------------------------------------------------------------

if( NOT WIN32 )
  option( MEMCANVAS "Compile memory canvas implementation" ON )

  if( NOT MEMCANVAS )
    set( SGUI_NO_MEM_CANVAS 1 )
  endif( )
endif( )

if( WIN32 )
  option( DIRECT3D9 "Compile with Direct3D(R) 9 support" ON )
  option( DIRECT3D11 "Compile with Direct3D(R) 11 support" ON )
else( )
  set( DIRECT3D9 false )
  set( DIRECT3D11 false )
endif( )

option( OPENGL "Compile with OpenGL(R) support" ON )
option( NOFLOAT "Disable all use of floatingpoint data types" OFF )
option( MESSAGEBOX "Compile with message box dialog" ON )
option( NATIVE_MB "Compile with native emergency message box function" ON )

if( UNIX )
  set( ICONCACHE true )
else( )
  option( ICONCACHE "Compile with icon and font cache implementations" ON )
endif( )

option( ICONVIEW "Compile with icon view widget" ON )
option( SUBVIEW "Compile with sub view widget" ON )
option( TABS "Compile with tab widget" ON )
option( FRAME "Compile with frame widget" ON )
option( GROUP "Compile with group box widget" ON )
option( COLPICKER "Compile with color picker widget" ON )
option( COLDIALOG "Compile with color selection dialog" ON )
option( MVC "Compile with model object for MVC widgets" ON )
option( NOPS "Replace disabled features with API compatible NOP functions" ON)

if( NOFLOAT )
  set( SGUI_NO_FLOAT 1 )
endif( )

if( NOT DIRECT3D9 )
  set( SGUI_NO_D3D9 1 )
endif( )

if( NOT DIRECT3D11 )
  set( SGUI_NO_D3D11 1 )
endif( )

if( NOT OPENGL )
  set( SGUI_NO_OPENGL 1 )
endif( )

if( NOT MESSAGEBOX )
  set( SGUI_NO_MESSAGEBOX 1 )
endif( )

if( NOT NATIVE_MB )
  set( SGUI_NO_NATIVE_MESSAGEBOX 1 )
endif( )

if( NOT ICONCACHE )
  set( SGUI_NO_ICON_CACHE 1 )
endif( )

if( NOT ICONVIEW )
  set( SGUI_NO_ICON_VIEW 1 )
endif( )

if( NOT SUBVIEW )
  set( SGUI_NO_SUBVIEW 1 )
endif( )

if( NOT TABS )
  set( SGUI_NO_TABS 1 )
endif( )

if( NOT FRAME )
  set( SGUI_NO_FRAME 1 )
endif( )

if( NOT GROUP )
  set( SGUI_NO_GROUP_BOX 1 )
endif( )

if( NOT COLPICKER )
  set( SGUI_NO_COLOR_PICKER 1 )
endif( )

if( NOT COLDIALOG )
  set( SGUI_NO_COLOR_DIALOG 1 )
endif( )

if( NOT MVC )
  set( SGUI_NO_MODEL 1 )
endif( )

if( NOPS )
  set( SGUI_NOP_IMPLEMENTATIONS 1 )
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

  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows -static-libgcc" )

  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -static-libgcc")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc")
endif( )

if( MSVC )
  message( STATUS "Compiling with MSVC" )

  add_definitions( /D_CRT_SECURE_NO_WARNINGS )

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
  set( SGUI_WINDOWS 1 )

  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    message( STATUS "Compiling for 64 bit Windows" )
  else( )
    message( STATUS "Compiling for 32 bit Windows" )
  endif( )
elseif( APPLE )
  message( ERROR "Mac OS X is not supported yet!" )
elseif( UNIX )
  set( SGUI_UNIX 1 )
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
                     ${CMAKE_SOURCE_DIR}/subwm/include/
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

  if( MSVC )
    set( SGUI_DEP ${SGUI_DEP} libfreetype.dll.lib )
  else( )
    set( SGUI_DEP ${SGUI_DEP} freetype )
  endif( )

  if( DIRECT3D11 )
    set( SGUI_DEP ${SGUI_DEP} dxguid )
  endif( )
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

configure_file( ${CMAKE_SOURCE_DIR}/sgui_config.in
                ${CMAKE_BINARY_DIR}/include/sgui_config.h )

include_directories( ${CMAKE_BINARY_DIR}/include )

add_subdirectory( core )
add_subdirectory( widgets )
add_subdirectory( dialogs )
add_subdirectory( subwm )

add_library( sgui SHARED ${CORE_SRC} ${WIDGETS_SRC} ${DIALOGS_SRC} )
add_library( sguisubwm STATIC ${SUBWM_SRC} )

target_link_libraries( sgui ${SGUI_DEP} )
target_link_libraries( sguisubwm sgui )

add_subdirectory( extras )

enable_testing( )
add_subdirectory( tests )

if( WIN32 )
  set_target_properties(sgui PROPERTIES PREFIX "")
endif( )

#----------------------------------------------------------------------
# generate Doxygen API reference
#----------------------------------------------------------------------
find_package( Doxygen )

if( DOXYGEN_FOUND )
  configure_file( ${CMAKE_SOURCE_DIR}/Doxyfile.in
                  ${CMAKE_BINARY_DIR}/Doxyfile @ONLY )

  add_custom_target( doxydoc
                     ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
                     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                     COMMENT "Generating API documentation with Doxygen"
                     VERBATIM
                   )
endif( )

#----------------------------------------------------------------------
# Configure packaging
#----------------------------------------------------------------------
if( WIN32 )
  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( W32_ARCH "x64" )
  else( )
    set( W32_ARCH "x86" )
  endif( )

  add_custom_target( pack 
                     COMMAND mkdir -p ./include
                     COMMAND mkdir -p ./doc
                     COMMAND cp -r ${CMAKE_SOURCE_DIR}/extras .
                     COMMAND cp -r ${CMAKE_SOURCE_DIR}/doc .
                     COMMAND cp ${CMAKE_SOURCE_DIR}/README .
                     COMMAND cp ${CMAKE_SOURCE_DIR}/LICENSE .
                     COMMAND cp ${CMAKE_SOURCE_DIR}/core/include/*.h ./include
                     COMMAND cp ${CMAKE_SOURCE_DIR}/widgets/include/*.h ./include
                     COMMAND cp ${CMAKE_SOURCE_DIR}/dialogs/include/*.h ./include
                     COMMAND cp ./win_dep/${W32_ARCH}/libfreetype.{a,la} ./lib/
                     COMMAND cp ./lib/libfreetype.a ./lib/libfreetype.lib
                     COMMAND cp ./lib/libsgui.dll.a ./lib/libsgui.dll.lib
                     COMMAND ${CMAKE_STRIP} --strip-all ./bin/*
                     COMMAND ${CMAKE_STRIP} --strip-all ./lib/*.lib
                     COMMAND ${CMAKE_STRIP} --strip-all ./lib/*.a
                     COMMAND cp ./win_dep/FTL.TXT ./lib
                     COMMAND cp ./win_dep/FTL.TXT ./bin
                     COMMAND zip sgui.zip -r -- ./bin ./font ./lib ./include ./doxydoc
                     COMMAND zip ./sgui.zip -r -- ./LICENSE ./README ./doc
                     COMMAND find extras -name "*.c" -print | zip ./sgui.zip -@
                     COMMAND mv sgui.zip ${CMAKE_SOURCE_DIR}/sgui_win32_${W32_ARCH}.zip
                     DEPENDS doxydoc
                     WORKING_DIRECTORY ${CMAKE_BINARY_DIR} )
endif( )

if( UNIX )
  install( DIRECTORY core/include DESTINATION include )
  install( DIRECTORY widgets/include DESTINATION include )
  install( DIRECTORY dialogs/include DESTINATION include )
  install( DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION include )

  install( TARGETS sgui
           RUNTIME DESTINATION bin
           LIBRARY DESTINATION lib
           ARCHIVE DESTINATION lib )
endif( )

