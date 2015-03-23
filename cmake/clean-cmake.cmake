#clean-all.cmake
# some defines
set(ROOT ${CMAKE_BINARY_DIR})

##############################

#main list
set(cmake_generated ${ROOT}/CMakeCache.txt
                    ${ROOT}/cmake_install.cmake  
                    ${ROOT}/Makefile
                    ${ROOT}/CMakeFiles
)

# Root loop
foreach(file ${cmake_generated})
  if (EXISTS ${file})
     file(REMOVE_RECURSE ${file})
  else (NOT EXISTS ${file})
     message("File ${file} not found")
  endif()
endforeach(file)

# remove temp files
execute_process(
  COMMAND find . -type f -name "*~" -exec rm {} \;
) 
