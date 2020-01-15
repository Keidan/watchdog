cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project(watchdog CXX)
set (CMAKE_CXX_STANDARD 11)

# add some macros, functions
include(cmake/finder.cmake)

finder_include(libxml/xmlmemory.h "Please install the devel package of the XML2 library")
message("Supported conf dir.: cmake -DCONFIG_DIR=[/etc/watchdog]")
message("Supported distrib.: cmake -DDISTRIBUTION=[debug|release]")
message("Supported arch.: cmake -DCMAKE_BUILD_TYPE=[x86|x86_64|armv7l]")

if("${DISTRIBUTION}" MATCHES "release")
  set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Werror -O2 -fPIC")
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
set(WATCHDOG_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(WATCHDOG_SRC_DIR ${CMAKE_SOURCE_DIR}/src)
include_directories("/usr/include/libxml2")


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
message("-- Dristrib: ${DISTRIBUTION}")
message("-- Arch: ${ARCH}")
message("-- Config dir: ${CONFIG_DIR}")

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
file(GLOB_RECURSE bin_src "${WATCHDOG_SRC_DIR}/*.cpp")
file(GLOB_RECURSE bin_inc "${WATCHDOG_SRC_DIR}/*.hpp")

# Executable configuration
add_executable(watchdog ${bin_src} ${bin_inc})
target_link_libraries(watchdog xml2 rt)



