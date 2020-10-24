cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(watchdog CXX)
set (CMAKE_CXX_STANDARD 11)

message("Supported conf dir.: cmake -DCONFIG_DIR=[/etc/watchdog]")
message("Supported distrib.: cmake -DDISTRIBUTION=[debug|release]")
message("Supported arch.: cmake -DCMAKE_BUILD_TYPE=[x86|x86_64|armv7l]")
message("Disable XML: cmake -DDISABLE_XML=yes")
message("Disable JSON: cmake -DDISABLE_JSON=yes")

if("${DISTRIBUTION}" MATCHES "release")
  set(CMAKE_CXX_FLAGS "-std=c++11 -fstack-protector-all -D_FORTIFY_SOURCE=2 -ffunction-sections -Wall -Werror")
  set(DISTRIBUTION "release")
else()
  set(DISTRIBUTION "debug")
  set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -O0 -g -fPIC")
  add_definitions(-DDEBUG)
endif()

if(CONFIG_DIR STREQUAL "")
  set(CONFIG_DIR "/etc/watchdog")
endif()

# update directories
set(ADDITIONAL_LIBS "")
set(WATCHDOG_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(WATCHDOG_SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# configure
include(${CMAKE_SOURCE_DIR}/cmake/config.cmake)
configure_file(${CMAKE_SOURCE_DIR}/cmake/config-cmake.hpp.in ${WATCHDOG_SRC_DIR}/config.hpp)


set(ARCH "${CMAKE_BUILD_TYPE}")
if(ARCH STREQUAL "")
  set(ARCH "${CMAKE_SYSTEM_PROCESSOR}")
  if(ARCH STREQUAL "i686" OR ARCH STREQUAL "i386")
    set(ARCH "x86")
  endif()
endif()
if("${ARCH}" MATCHES "x86_64")
  # the name of the target operating system
  set(CMAKE_SYSTEM_NAME Linux)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
  set(ARCH "x86_64")
elseif("${ARCH}" MATCHES "armv7l")
  # the name of the target operating system
  set(CMAKE_SYSTEM_NAME Linux)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
  set(ARCH "arm7lv")
else()
  # the name of the target operating system
  set(CMAKE_SYSTEM_NAME Linux)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
  set(ARCH "x86")
endif()

# Information
message("-- Dristrib: ${DISTRIBUTION}")
message("-- Arch: ${ARCH}")
message("-- Config dir: ${CONFIG_DIR}")
if(HAVE_XML_H)
message("-- XML support: YES")
else()
message("-- XML support: NO")
endif()
if(HAVE_JSON_H)
message("-- JSON support: YES")
else()
message("-- JSON support: NO")
endif()


add_definitions(-DCONFIG_FILE_FOLDER="${CONFIG_DIR}")

# removes generated files with 'make clean-all'
add_custom_target(clean-cmake
   COMMAND ${CMAKE_COMMAND} -P cmake/clean-cmake.cmake
)
add_custom_target(clean-all
   COMMAND ${CMAKE_BUILD_TOOL} clean
   COMMAND ${CMAKE_BUILD_TOOL} clean-cmake
)

set(WATCHDOG_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin/${DISTRIBUTION})
file(MAKE_DIRECTORY ${WATCHDOG_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${WATCHDOG_BINARY_DIR})


# list of sources.
set(SOURCES
	${WATCHDOG_SRC_DIR}/WRun.cpp
	${WATCHDOG_SRC_DIR}/WUtils.cpp
	${WATCHDOG_SRC_DIR}/Log.cpp
)
set(INCLUDES
	${WATCHDOG_SRC_DIR}/config.hpp
	${WATCHDOG_SRC_DIR}/watchdog.hpp
	${WATCHDOG_SRC_DIR}/Log.hpp
)
if(HAVE_XML_H)
list(APPEND SOURCES ${WATCHDOG_SRC_DIR}/WXML.cpp)
endif()
if(HAVE_JSON_H)
list(APPEND SOURCES ${WATCHDOG_SRC_DIR}/WJSON.cpp)
endif()

set(bin_src
	${WATCHDOG_SRC_DIR}/watchdog.cpp
	${SOURCES}
)

# Executable configuration
add_executable(watchdog ${bin_src} ${INCLUDES})
target_link_libraries(watchdog rt ${ADDITIONAL_LIBS})

