cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(watchdog CXX)

file(READ "version.txt" ver)
string(REGEX MATCH "VERSION_MAJOR ([0-9]*)" _ ${ver})
set(VERSION_MAJOR ${CMAKE_MATCH_1})
string(REGEX MATCH "VERSION_MINOR ([0-9]*)" _ ${ver})
set(VERSION_MINOR ${CMAKE_MATCH_1})

set (CPP_VERISON 17)
set (CMAKE_CXX_STANDARD ${CPP_VERISON})

find_package(Python COMPONENTS Interpreter Development)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message("Supported conf dir.: cmake -DCONFIG_DIR=[/etc/watchdog]")
message("Supported distrib.: cmake -DDISTRIBUTION=[debug|release]")
message("Supported arch.: cmake -DARCHITECTURE=[x86|x86_64|armv7l]")
message("Disable XML: cmake -DDISABLE_XML=[on|off]")
message("Disable JSON: cmake -DDISABLE_JSON=[on|off]")

set(CONFIG_DIR "/etc/watchdog" CACHE STRING "Supported conf dir.")
set(DISTRIBUTION "debug" CACHE STRING "Supported distrib.")
set(ARCHITECTURE "${CMAKE_SYSTEM_PROCESSOR}" CACHE STRING "Supported arch.")
set(DISABLE_XML OFF CACHE BOOL "Disable XML support")
set(DISABLE_JSON OFF CACHE BOOL "Disable JSON support")

if("${DISTRIBUTION}" MATCHES "release")
  set(CMAKE_CXX_FLAGS "-std=c++${CPP_VERISON} -fstack-protector-all -D_FORTIFY_SOURCE=2 -ffunction-sections -Wall -Werror")
  set(DISTRIBUTION "release")
else()
  set(DISTRIBUTION "debug")
  set(CMAKE_CXX_FLAGS "-std=c++${CPP_VERISON} -Wall -O0 -g -fPIC -fprofile-arcs -ftest-coverage --coverage  -fprofile-abs-path")
  set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
  add_definitions(-DGCOV)
  add_definitions(-DDEBUG)
endif()
set(CMAKE_BUILD_TYPE ${DISTRIBUTION})

if(CONFIG_DIR STREQUAL "")
  set(CONFIG_DIR "/etc/watchdog")
endif()

# update directories
set(ADDITIONAL_LIBS "")
set(APP_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin/${DISTRIBUTION})
set(APP_SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(APP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src)
file(MAKE_DIRECTORY ${APP_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${APP_BINARY_DIR})
include_directories(${APP_INCLUDE_DIR})
set(OBJECT_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/${CMAKE_PROJECT_NAME}.dir)
message("-- Object files will be output to: ${OBJECT_DIR}")

# configure
include(${CMAKE_SOURCE_DIR}/cmake/config.cmake)
configure_file(${CMAKE_SOURCE_DIR}/cmake/config-cmake.hpp.in ${APP_SRC_DIR}/wd/BuildConfig.hpp)


set(ARCH "${ARCHITECTURE}")
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
message("-- Version: ${VERSION_MAJOR}.${VERSION_MINOR}")
message("-- Distrib.: ${DISTRIBUTION}")
message("-- Arch.: ${ARCH}")
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


# list of sources.
set(common_sources
	${APP_SRC_DIR}/wd/run/Run.cpp
	${APP_SRC_DIR}/wd/utils/Log.cpp
	${APP_SRC_DIR}/wd/Watchdog.cpp
)
set(common_includes
	${APP_SRC_DIR}/wd/BuildConfig.hpp
	${APP_SRC_DIR}/wd/config/IConfigable.hpp
	${APP_SRC_DIR}/wd/run/Run.hpp
	${APP_SRC_DIR}/wd/run/Process.hpp
	${APP_SRC_DIR}/wd/utils/Helper.hpp
	${APP_SRC_DIR}/wd/utils/Log.hpp
)
if(HAVE_XML_H)
  set(config_sources ${config_sources} ${APP_SRC_DIR}/wd/config/ConfigXML.cpp)
  set(config_includes ${config_includes} ${APP_SRC_DIR}/wd/config/ConfigXML.hpp)
endif()
if(HAVE_JSON_H)
  set(config_sources ${config_sources} ${APP_SRC_DIR}/wd/config/ConfigJSON.cpp)
  set(config_includes ${config_includes} ${APP_SRC_DIR}/wd/config/ConfigJSON.hpp)
endif()

set(bin_src
	${common_sources}
  ${config_sources}
	${APP_SRC_DIR}/main.cpp
)
set(bin_inc ${common_includes} ${config_includes})

add_custom_target(
  update_sonar_version ALL
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/update_sonar_version.py --root ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Target update_sonar_version"
)

# Executable configuration
add_executable(${CMAKE_PROJECT_NAME} ${bin_src} ${bin_inc})
target_link_libraries(${CMAKE_PROJECT_NAME} rt ${ADDITIONAL_LIBS})


if("${DISTRIBUTION}" MATCHES "debug")
  # Create the gcov target. Run coverage tests with 'make gcov'
  add_custom_target(gcov
    COMMAND mkdir -p coverage
    COMMAND ${CMAKE_MAKE_PROGRAM} test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )

  add_custom_command(TARGET gcov
    COMMAND gcov -b ${CMAKE_SOURCE_DIR}/src/wd/run/Run.cpp -o ${OBJECT_DIR}/src/wd/run/
    | grep -A 5 "Run.cpp" > CoverageSummary.tmp
    COMMAND gcov -b ${CMAKE_SOURCE_DIR}/src/wd/utils/Log.cpp -o ${OBJECT_DIR}/src/wd/utils/
    | grep -A 5 "Log.cpp" >> CoverageSummary.tmp
    COMMAND gcov -b ${CMAKE_SOURCE_DIR}/src/wd/Watchdog.cpp -o ${OBJECT_DIR}/src/wd/
    | grep -A 5 "Watchdog.cpp" >> CoverageSummary.tmp
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coverage
  )
  if(HAVE_XML_H)
    add_custom_command(TARGET gcov
      COMMAND gcov -b ${CMAKE_SOURCE_DIR}/src/wd/config/ConfigXML.cpp -o ${OBJECT_DIR}/src/wd/config/
      | grep -A 5 "ConfigXML.cpp" >> CoverageSummary.tmp
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coverage
  )
  endif()
  if(HAVE_JSON_H)
    add_custom_command(TARGET gcov
      COMMAND gcov -b ${CMAKE_SOURCE_DIR}/src/wd/config/ConfigJSON.cpp -o ${OBJECT_DIR}/src/wd/config/
      | grep -A 5 "ConfigJSON.cpp" >> CoverageSummary.tmp
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coverage
    )
  endif()

  add_custom_command(TARGET gcov
  COMMAND echo "=================== GCOV ===================="
    COMMAND cat CoverageSummary.tmp
    COMMAND echo "-- Coverage files have been output to ${CMAKE_BINARY_DIR}/coverage"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coverage
  )

  add_custom_command(TARGET gcov POST_BUILD
    COMMAND lcov --directory ${OBJECT_DIR} -c -o ${CMAKE_BINARY_DIR}/coverage/rapport.info
    COMMAND lcov --remove ${CMAKE_BINARY_DIR}/coverage/rapport.info -o ${CMAKE_BINARY_DIR}/coverage/rapport_filtered.info '${CMAKE_SOURCE_DIR}/nlohmann-json/*'
    COMMAND genhtml -o . -t "Test cover" rapport_filtered.info
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coverage
  )
  
  add_dependencies(gcov ${CMAKE_PROJECT_NAME})
  # Make sure to clean up the coverage folder
  set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES coverage)

  # Create the gcov-clean target. This cleans the build as well as generated 
  # .gcda and .gcno files.
  add_custom_target(scrub
    COMMAND ${CMAKE_MAKE_PROGRAM} clean
    COMMAND find ${OBJECT_DIR} -type f -name "*.gcno" | xargs rm 2>/dev/null
    COMMAND find ${OBJECT_DIR} -type f -name "*.gcda" | xargs rm 2>/dev/null
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )

  include(${CMAKE_SOURCE_DIR}/cmake/tests.cmake)
  
endif()