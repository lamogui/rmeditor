cmake_minimum_required(VERSION 3.1.3)

project(rmeditor-dependencies)

# define a macro that helps defining an option
macro(rmeditor_dependencies_set_option var default type docstring)
    if(NOT DEFINED ${var})
        set(${var} ${default})
    endif()
    set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()


# determine whether to create a debug or release build
rmeditor_dependencies_set_option(CMAKE_BUILD_TYPE "Release" STRING "Choose the type of build (Debug or Release)")
rmeditor_dependencies_set_option(RMEDITOR_DEPENCENCIES_LIB_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../lib" PATH "Output path of *.a")
if (MSVC)
	rmeditor_dependencies_set_option(MSVC_STATIC OFF BOOL "use /MT* instead of /MD*")
endif(MSVC)
message(STATUS "RMEDITOR_DEPENCENCIES_LIB_OUTPUT_PATH set to ${RMEDITOR_DEPENCENCIES_LIB_OUTPUT_PATH}")

set(CMAKE_CXX_STANDARD_REQUIRED 11)
set(CMAKE_CXX_STANDARD 11)



# enable project folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#set lib output path
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${RMEDITOR_DEPENCENCIES_LIB_OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${RMEDITOR_DEPENCENCIES_LIB_OUTPUT_PATH})
link_directories(${RMEDITOR_DEPENCENCIES_LIB_OUTPUT_PATH})


# add "d" for debug libraries
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
if (CMAKE_BUILD_TYPE MATCHES DEBUG)
  set(CMAKE_STATIC_LIBRARY_SUFFIX "-dbg${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set(CMAKE_SHARED_LIBRARY_SUFFIX "-dbg${CMAKE_SHARED_LIBRARY_SUFFIX}")

  message(STATUS "CMAKE_STATIC_LIBRARY_SUFFIX=${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

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

add_subdirectory(rtaudio)
add_subdirectory(tunefish4)
add_subdirectory(ogg)

set(OGG_LIBRARIES ogg)
set(OGG_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/ogg/include)
add_subdirectory(vorbis)



