
set(INCROOT ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRCROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)

# all source files
set(SRC  
  ${SRCROOT}/array.cpp
  ${INCROOT}/array.hpp
  ${SRCROOT}/datastream.cpp
  ${INCROOT}/datastream.hpp
  ${SRCROOT}/random.cpp
  ${INCROOT}/random.hpp
  ${SRCROOT}/runtime.cpp
  ${INCROOT}/runtime.hpp
  ${SRCROOT}/simd.cpp
  ${INCROOT}/simd.hpp
  ${INCROOT}/system.hpp
  ${SRCROOT}/tf4.cpp
  ${INCROOT}/tf4.hpp
  ${SRCROOT}/tf4fx.cpp
  ${INCROOT}/tf4fx.hpp
  ${INCROOT}/types.hpp
)

source_group("tunefish4" FILES ${SRC})

if(WIN32)
  set(TF4_DEFINITIONS ${TF4_DEFINITIONS} -DeWIN32)
else()
  set(TF4_DEFINITIONS ${TF4_DEFINITIONS} -DeLINUX)
endif()

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
if (CMAKE_BUILD_TYPE MATCHES DEBUG)
  set(TF4_DEFINITIONS ${TF4_DEFINITIONS} -DeDEBUG)
else()
  set(TF4_DEFINITIONS ${TF4_DEFINITIONS} -DeRELEASE)
endif()



add_library(tunefish4 STATIC ${SRC})
target_include_directories(tunefish4 PRIVATE ${INCROOT})
target_compile_definitions(tunefish4 PUBLIC ${TF4_DEFINITIONS})







