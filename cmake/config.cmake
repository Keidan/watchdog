#check for config

if(NOT ${DISABLE_XML} MATCHES "on")
  if(EXISTS /usr/include/libxml2)
    find_package(LibXml2)
    if(LIBXML2_FOUND)
      include_directories(${LIBXML2_INCLUDE_DIR})
      list(APPEND ADDITIONAL_LIBS ${LIBXML2_LIBRARIES})
      set(HAVE_XML_H 1)
      message("libxml2 includes: ${LIBXML2_INCLUDE_DIR}")
      message("libxml2 libraries: ${LIBXML2_LIBRARIES}")
    endif()
  else()
    message("/usr/include/libxml2 not found")
  endif()
endif()


if(NOT ${DISABLE_JSON} MATCHES "on")
  include_directories(${CMAKE_SOURCE_DIR}/nlohmann-json/single_include)
  set(HAVE_JSON_H 1)
endif()