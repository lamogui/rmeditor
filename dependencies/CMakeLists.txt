cmake_minimum_required(VERSION 3.1.3)

# define a macro that helps defining an option
macro(rmeditor_dependencies_set_option var default type docstring)
    if(NOT DEFINED ${var})
        set(${var} ${default})
    endif()
    set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()


# determine whether to create a debug or release build
rmeditor_dependencies_set_option(CMAKE_BUILD_TYPE Release STRING "Choose the type of build (Debug or Release)")
rmeditor_dependencies_set_option(LIB_PATH "${CMAKE_SOURCE_DIR}/../lib" PATH "Output path of *.a")
message(STATUS "LIB_PATH set to ${LIB_PATH}")

set(CMAKE_CXX_STANDARD_REQUIRED 11)
set(CMAKE_CXX_STANDARD 11)

project(rmeditor-dependencies)


#if (MSVC)
#  foreach(flag
#          CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
#          CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
#    if(${flag} MATCHES "/MD")
#      string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
#    endif()
#  endforeach()
#endif()

# enable project folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#set lib output path
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_PATH})
LINK_DIRECTORIES(${LIB_PATH})


string(TOUPPER ${CMAKE_BUILD_TYPE} DEP_BUILD_TYPE)
if (DEP_BUILD_TYPE MATCHES DEBUG)
  set(CMAKE_STATIC_LIBRARY_SUFFIX "d${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set(CMAKE_SHARED_LIBRARY_SUFFIX "d${CMAKE_SHARED_LIBRARY_SUFFIX}")

  message(INFO "CMAKE_STATIC_LIBRARY_SUFFIX=${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()



add_subdirectory(rtaudio)
add_subdirectory(tunefish4)


