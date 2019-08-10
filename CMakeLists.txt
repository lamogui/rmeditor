cmake_minimum_required(VERSION 3.2.2) #travis 
project(rmeditor)

#macros 
macro(rmeditor_set_option var default type docstring)
    if(NOT DEFINED ${var})
        set(${var} ${default})
    endif()
    set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()

#options
rmeditor_set_option(CMAKE_BUILD_TYPE "Release" STRING "Choose the type of build (Debug or Release)")
rmeditor_set_option(RMEDITOR_USE_QT_STATIC OFF BOOL "Have you successfully generated a static build of qt ?")
rmeditor_set_option(RMEDITOR_BIN_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin" PATH "Output path of *.exe")
if (MSVC)
	rmeditor_set_option(MSVC_STATIC OFF BOOL "use /MT* instead of /MD*")
endif(MSVC)

#c++ 11
set(CMAKE_CXX_STANDARD_REQUIRED 11)
set(CMAKE_CXX_STANDARD 11)

#set bin output path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RMEDITOR_BIN_OUTPUT_PATH})

#dependencies 
set(BUILD_TESTING OFF) #force RtAudio not test
add_subdirectory(dependencies)

#qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)

### rmeditor
#headers
set(RMEDITOR_INCLUDE_DIR include)
set(RMEDITOR_H
#  ${RMEDITOR_INCLUDE_DIR}/4klangmusic.hpp
  ${RMEDITOR_INCLUDE_DIR}/call_once.h
  ${RMEDITOR_INCLUDE_DIR}/camera.hpp
  ${RMEDITOR_INCLUDE_DIR}/classmanager.hpp
  ${RMEDITOR_INCLUDE_DIR}/demotimeline.hpp
  ${RMEDITOR_INCLUDE_DIR}/fast2dquad.hpp
#  ${RMEDITOR_INCLUDE_DIR}/ffmpegencoder.hpp
  ${RMEDITOR_INCLUDE_DIR}/highlighter.hpp
  ${RMEDITOR_INCLUDE_DIR}/jassert.hpp
  ${RMEDITOR_INCLUDE_DIR}/keyframe.hpp
  ${RMEDITOR_INCLUDE_DIR}/logdockwidget.hpp
  ${RMEDITOR_INCLUDE_DIR}/logmanager.hpp
  ${RMEDITOR_INCLUDE_DIR}/logwidget.hpp
  ${RMEDITOR_INCLUDE_DIR}/mainwindow.hpp
  ${RMEDITOR_INCLUDE_DIR}/mediafile.hpp
  ${RMEDITOR_INCLUDE_DIR}/mediafileseditorwidget.hpp
  ${RMEDITOR_INCLUDE_DIR}/music.hpp
  ${RMEDITOR_INCLUDE_DIR}/oggvorbismusic.hpp
  ${RMEDITOR_INCLUDE_DIR}/project.hpp
  ${RMEDITOR_INCLUDE_DIR}/quadfragmentshader.hpp
  ${RMEDITOR_INCLUDE_DIR}/render.hpp
  ${RMEDITOR_INCLUDE_DIR}/renderer.hpp
  ${RMEDITOR_INCLUDE_DIR}/renderfunctionscache.hpp
  ${RMEDITOR_INCLUDE_DIR}/renderwidget.hpp
  ${RMEDITOR_INCLUDE_DIR}/scene.hpp
  ${RMEDITOR_INCLUDE_DIR}/sequence.hpp
  ${RMEDITOR_INCLUDE_DIR}/shadercode.hpp
  ${RMEDITOR_INCLUDE_DIR}/shaderminifier.hpp
  ${RMEDITOR_INCLUDE_DIR}/singleton.h
  ${RMEDITOR_INCLUDE_DIR}/texteditable.hpp
  ${RMEDITOR_INCLUDE_DIR}/texteditor.hpp
  ${RMEDITOR_INCLUDE_DIR}/timeline.hpp
  ${RMEDITOR_INCLUDE_DIR}/timelinedockwidget.hpp
  ${RMEDITOR_INCLUDE_DIR}/timelinetrack.hpp
  ${RMEDITOR_INCLUDE_DIR}/timelinewidget.hpp
#  ${RMEDITOR_INCLUDE_DIR}/tunefish4music.hpp
  ${RMEDITOR_INCLUDE_DIR}/undocommands.hpp
  ${RMEDITOR_INCLUDE_DIR}/xmlsavedobject.hpp
  ${RMEDITOR_INCLUDE_DIR}/tf4.hpp
  ${RMEDITOR_INCLUDE_DIR}/tf4fx.hpp
  ${RMEDITOR_INCLUDE_DIR}/array.hpp
  ${RMEDITOR_INCLUDE_DIR}/datastream.hpp
  ${RMEDITOR_INCLUDE_DIR}/runtime.hpp
  ${RMEDITOR_INCLUDE_DIR}/simd.hpp
  ${RMEDITOR_INCLUDE_DIR}/random.hpp
  ${RMEDITOR_INCLUDE_DIR}/system.hpp
)

#sources
set(RMEDITOR_SRC_DIR src)
set(RMEDITOR_SRC 
#  ${RMEDITOR_SRC_DIR}/4klangmusic.cpp
  ${RMEDITOR_SRC_DIR}/camera.cpp
  ${RMEDITOR_SRC_DIR}/classmanager.cpp
  ${RMEDITOR_SRC_DIR}/demotimeline.cpp
  ${RMEDITOR_SRC_DIR}/fast2dquad.cpp
#  ${RMEDITOR_SRC_DIR}/ffmpegencoder.cpp
  ${RMEDITOR_SRC_DIR}/highlighter.cpp
  ${RMEDITOR_SRC_DIR}/keyframe.cpp
  ${RMEDITOR_SRC_DIR}/logdockwidget.cpp
  ${RMEDITOR_SRC_DIR}/logmanager.cpp
  ${RMEDITOR_SRC_DIR}/logwidget.cpp
  ${RMEDITOR_SRC_DIR}/main.cpp
  ${RMEDITOR_SRC_DIR}/mainwindow.cpp
  ${RMEDITOR_SRC_DIR}/mediafile.cpp
  ${RMEDITOR_SRC_DIR}/mediafileseditorwidget.cpp
  ${RMEDITOR_SRC_DIR}/music.cpp
  ${RMEDITOR_SRC_DIR}/oggvorbismusic.cpp
  ${RMEDITOR_SRC_DIR}/project.cpp
  ${RMEDITOR_SRC_DIR}/quadfragmentshader.cpp
  ${RMEDITOR_SRC_DIR}/render.cpp
  ${RMEDITOR_SRC_DIR}/renderer.cpp
  ${RMEDITOR_SRC_DIR}/renderfunctionscache.cpp
  ${RMEDITOR_SRC_DIR}/renderwidget.cpp
  ${RMEDITOR_SRC_DIR}/scene.cpp
  ${RMEDITOR_SRC_DIR}/sequence.cpp
  ${RMEDITOR_SRC_DIR}/shadercode.cpp
  ${RMEDITOR_SRC_DIR}/shaderminifier.cpp
  ${RMEDITOR_SRC_DIR}/texteditable.cpp
  ${RMEDITOR_SRC_DIR}/texteditor.cpp
  ${RMEDITOR_SRC_DIR}/timeline.cpp
  ${RMEDITOR_SRC_DIR}/timelinedockwidget.cpp
  ${RMEDITOR_SRC_DIR}/timelinetrack.cpp
  ${RMEDITOR_SRC_DIR}/timelinewidget.cpp
#  ${RMEDITOR_SRC_DIR}/tunefish4music.cpp
  ${RMEDITOR_SRC_DIR}/undocommands.cpp
  ${RMEDITOR_SRC_DIR}/xmlsavedobject.cpp
  ${RMEDITOR_SRC_DIR}/tf4.cpp
  ${RMEDITOR_SRC_DIR}/tf4fx.cpp
  ${RMEDITOR_SRC_DIR}/array.cpp
  ${RMEDITOR_SRC_DIR}/datastream.cpp
  ${RMEDITOR_SRC_DIR}/runtime.cpp
  ${RMEDITOR_SRC_DIR}/simd.cpp
  ${RMEDITOR_SRC_DIR}/random.cpp
)

#ui
set(RMEDITOR_FORMS_DIR src)
set(RMEDITOR_FORMS  
    ${RMEDITOR_FORMS_DIR}/logdockwidget.ui 
    ${RMEDITOR_FORMS_DIR}/mainwindow.ui 
    ${RMEDITOR_FORMS_DIR}/mediafileseditorwidget.ui
    ${RMEDITOR_FORMS_DIR}/timelinedockwidget.ui
)

#ressources
set(RMEDITOR_RESOURCES_DIR resources)
qt5_add_resources(RMEDITOR_RESOURCES
   ${RMEDITOR_RESOURCES_DIR}/res.qrc 
   ${RMEDITOR_RESOURCES_DIR}/qdarkstyle/style.qrc
)

if (WIN32)
    set(RMEDITOR_RC_DIR resources)
    set(RMEDITOR_RC ${RMEDITOR_RC_DIR}/ressources.rc)
endif(WIN32)


#libraries
set(RMEDITOR_LIBS 
  Qt5::Network 
  Qt5::Xml 
  Qt5::OpenGL 
  Qt5::Widgets 
  Qt5::Core
  rtaudio_static
  tunefish4
  ogg
  vorbis
  vorbisfile
  )

if(UNIX)
    set(RMEDITOR_LIBS ${RMEDITOR_LIBS} 
        GL #FIXME : error adding symbols: DSO missing from command line
        -lpulse -lpulse-simple -lpthread # FIXME : RtAudio dependencies for pthread not correctly set
        )  
endif(UNIX)

if (MSVC)
	set (RMEDITOR_LIBS ${RMEDITOR_LIBS} OpenGL32 )
    if (RMEDITOR_USE_QT_STATIC)
	    set(RMEDITOR_LIBS ${RMEDITOR_LIBS} 
          #Qt static dependencies
          general User32 
          general Ws2_32
          general Iphlpapi 
          general UxTheme 
          general Winmm
          general Dwmapi #Qt5PlatformSupport
          general Imm32 #Qt5PlatformSupport

          debug ${CMAKE_PREFIX_PATH}/lib/qtpcred.lib
          debug ${CMAKE_PREFIX_PATH}/lib/qtharfbuzzd.lib
          debug ${CMAKE_PREFIX_PATH}/lib/qtlibpngd.lib
          debug ${CMAKE_PREFIX_PATH}/lib/Qt5FontDatabaseSupportd.lib #Qt5PlatformSupport
          debug ${CMAKE_PREFIX_PATH}/lib/Qt5EventDispatcherSupportd.lib #Qt5PlatformSupport
          debug ${CMAKE_PREFIX_PATH}/lib/Qt5ThemeSupportd.lib #Qt5PlatformSupport
          debug ${CMAKE_PREFIX_PATH}/lib/Qt5AccessibilitySupportd.lib #Qt5PlatformSupport
          debug ${CMAKE_PREFIX_PATH}/lib/qtfreetyped.lib #Qt5PlatformSupport
          debug ${CMAKE_PREFIX_PATH}/plugins/platforms/qwindowsd.lib

          optimized ${CMAKE_PREFIX_PATH}/lib/qtpcre.lib
          optimized ${CMAKE_PREFIX_PATH}/lib/qtharfbuzz.lib
          optimized ${CMAKE_PREFIX_PATH}/lib/qtlibpng.lib
          optimized ${CMAKE_PREFIX_PATH}/lib/Qt5FontDatabaseSupport.lib #Qt5PlatformSupport
          optimized ${CMAKE_PREFIX_PATH}/lib/Qt5EventDispatcherSupport.lib #Qt5PlatformSupport
          optimized ${CMAKE_PREFIX_PATH}/lib/Qt5ThemeSupport.lib #Qt5PlatformSupport
          optimized ${CMAKE_PREFIX_PATH}/lib/Qt5AccessibilitySupport.lib #Qt5PlatformSupport
          optimized ${CMAKE_PREFIX_PATH}/lib/qtfreetype.lib #Qt5PlatformSupport
          optimized ${CMAKE_PREFIX_PATH}/plugins/platforms/qwindows.lib
        )
	 endif(RMEDITOR_USE_QT_STATIC)
endif(MSVC)

#definitions 
if (MSVC_STATIC)
  set(RMEDITOR_DEFINITIONS ${RMEDITOR_DEFINITIONS} PUBLIC "MSVC_STATIC")
endif(MSVC_STATIC)

#executable 
add_executable(rmeditor ${RMEDITOR_SRC} ${RMEDITOR_H} ${RMEDITOR_FORMS} ${RMEDITOR_RESOURCES})
target_link_libraries(rmeditor general ${RMEDITOR_LIBS})
target_include_directories(rmeditor PRIVATE
    ${RMEDITOR_INCLUDE_DIR} 
    dependencies #FIXME : RtAudio
    dependencies/tunefish4/include #FIXME : tunefish4
    dependencies/ogg/include 
    dependencies/vorbis/include
    ${CMAKE_CURRENT_BINARY_DIR})

if (MSVC_STATIC) # temp until there is at least one definition 
  target_compile_definitions(rmeditor ${RMEDITOR_DEFINITIONS})
endif (MSVC_STATIC)

#dependencies
if(MSVC)
	if (RMEDITOR_USE_QT_STATIC)
    
  else (RMEDITOR_USE_QT_STATIC)
		add_custom_target(
			QtCopyDll
			# debug version
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5Cored.dll       ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5Guid.dll        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5Networkd.dll    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5OpenGLd.dll     ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5Widgetsd.dll    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5Xmld.dll        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/

      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5Cored.pdb       ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5Guid.pdb        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5Networkd.pdb    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5OpenGLd.pdb     ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5Widgetsd.pdb    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5Xmld.pdb        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/
			
			# release version
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5Core.dll       ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5Gui.dll        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/ 
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5Network.dll    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5OpenGL.dll     ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5Widgets.dll    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5Core_DIR}/../../../bin/Qt5Xml.dll        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/
			# Output Message
			COMMENT "Copying Qt binaries from '${Qt5Core_DIR}/../../bin/' to '${CMAKE_RUNTIME_OUTPUT_DIRECTORY}'" VERBATIM
		)
		
		add_dependencies(rmeditor QtCopyDll)
  
	endif (RMEDITOR_USE_QT_STATIC)
endif(MSVC)

#MSVC static link
if (MSVC)
  if (MSVC_STATIC)
	foreach(flag
          CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
          CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		if(${flag} MATCHES "/MD")
		string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
		endif()
	endforeach()
  else (MSVC_STATIC)
	foreach(flag
          CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
          CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		if(${flag} MATCHES "/MT")
		string(REGEX REPLACE "/MT" "/MD" ${flag} "${${flag}}")
		endif()
	endforeach()
  endif(MSVC_STATIC)
endif()



