
enable_testing()
set(binary ${APP_BINARY_DIR}/${CMAKE_PROJECT_NAME})
set(max_only --max-respawn 0)
set(max_and_dir ${max_only} --directory ${CMAKE_SOURCE_DIR}/samples)
set(standalone_no_name ${max_only} --path /bin --arg -a --arg -l --arg -s --arg /home --env wd=10)
set(standalone ${standalone_no_name} --name ls)

set(tname test_help)
add_test(NAME ${tname} 
  COMMAND ${binary} --help
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)

set(tname test_version)
add_test(NAME ${tname} 
  COMMAND ${binary} --version
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)

set(tname test_invalid_param)
add_test(NAME ${tname}
  COMMAND ${binary} --hey
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

set(tname test_new_cfg_xml)
add_test(NAME ${tname}
  COMMAND ${binary} -z --directory /tmp/cfg --config test.xml
)
if(NOT HAVE_XML_H)
  set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)
else()
  set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)
endif()

set(tname test_new_cfg_json)
add_test(NAME ${tname}
  COMMAND ${binary} -z --directory /tmp/cfg --config test.json
)
if(NOT HAVE_JSON_H)
  set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)
else()
  set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)
endif()

set(tname test_new_cfg_error1)
add_test(NAME ${tname}
  COMMAND ${binary} -z --config test.xml
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

set(tname test_new_cfg_error2)
add_test(NAME ${tname}
  COMMAND ${binary} -z --directory /tmp/cfg
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

set(tname test_default_dir)
add_test(NAME ${tname}
  COMMAND ${binary} ${max_only} --config watchdog.xml
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

set(tname test_invalid_type)
add_test(NAME ${tname}
  COMMAND ${binary} ${max_and_dir} -t csv
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

set(tname test_cfg_xml)
add_test(NAME ${tname}
  COMMAND ${binary} ${max_and_dir} --config watchdog.xml
)
if(NOT HAVE_XML_H)
  set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)
else()
  set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)
endif()

set(tname test_cfg_json)
add_test(NAME ${tname}
  COMMAND ${binary} ${max_and_dir} --config watchdog.json
)
if(NOT HAVE_JSON_H)
  set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)
else()
  set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)
endif()

set(tname test_type_xml)
add_test(NAME ${tname} 
  COMMAND ${binary} ${max_and_dir} -t xml
)
if(NOT HAVE_XML_H)
  set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)
else()
  set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)
endif()

set(tname test_type_json)
add_test(NAME ${tname} 
  COMMAND ${binary} ${max_and_dir} -t json
)
if(NOT HAVE_JSON_H)
  set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)
else()
  set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)
endif()

set(tname test_standalone)
add_test(NAME ${tname}
  COMMAND ${binary} ${standalone}
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)

set(tname test_standalone_error1)
add_test(NAME ${tname}
  COMMAND ${binary} ${max_only} --name lssss
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

set(tname test_standalone_error2)
add_test(NAME ${tname}
  COMMAND ${binary} ${standalone_no_name} --name lssss
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)

set(tname test_standalone_no_spam)
add_test(NAME ${tname}
  COMMAND ${binary} ${standalone} --disable-spam-detect 
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)

set(tname test_standalone_working)
add_test(NAME ${tname}
  COMMAND ${binary} ${standalone} --working ${CMAKE_SOURCE_DIR}
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)

set(tname test_standalone_working_notfound)
add_test(NAME ${tname}
  COMMAND ${binary} ${standalone} --working /notfounddir
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)

set(tname test_err_max_respawn)
add_test(NAME ${tname}
  COMMAND ${binary} ${standalone} --max-respawn error
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

set(tname test_err_5_max_respawn)
add_test(NAME ${tname}
  COMMAND ${binary} ${standalone} --max-respawn 5
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

set(tname test_standalone_pid)
add_test(NAME ${tname}
  COMMAND ${binary} ${standalone} --pidfile /tmp/wd.pid --pid
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)

set(tname test_standalone_pid_error)
add_test(NAME ${tname}
  COMMAND ${binary} ${standalone} --pid
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

set(tname test_min_respawn)
add_test(NAME ${tname}
  COMMAND ${binary} ${standalone} --min-respawn-delay 10
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)

set(tname test_err_min_respawn1)
add_test(NAME ${tname}
  COMMAND ${binary} ${standalone} --min-respawn-delay error
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

set(tname test_err_min_respawn2)
add_test(NAME ${tname}
  COMMAND ${binary} ${standalone} --min-respawn-delay -1
)
set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

# Tests configs

if(HAVE_XML_H)
  set(tname test_cfg_xml_no_file)
  add_test(NAME ${tname}
    COMMAND /bin/bash ${CMAKE_SOURCE_DIR}/cmake/test_configurations.sh ${binary} xml no_file
  )
  set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

  set(tname test_cfg_xml_empty)
  add_test(NAME ${tname}
    COMMAND /bin/bash ${CMAKE_SOURCE_DIR}/cmake/test_configurations.sh ${binary} xml empty
  )
  set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

  set(tname test_cfg_xml_no_process)
  add_test(NAME ${tname}
    COMMAND /bin/bash ${CMAKE_SOURCE_DIR}/cmake/test_configurations.sh ${binary} xml no_process
  )
  set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

  set(tname test_cfg_xml_empty_name)
  add_test(NAME ${tname}
    COMMAND /bin/bash ${CMAKE_SOURCE_DIR}/cmake/test_configurations.sh ${binary} xml empty_name
  )
  set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)
  
  set(tname test_cfg_xml_env)
  add_test(NAME ${tname}
    COMMAND /bin/bash ${CMAKE_SOURCE_DIR}/cmake/test_configurations.sh ${binary} xml env
  )
  set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)
endif()
if(HAVE_JSON_H)
  set(tname test_cfg_json_no_file)
  add_test(NAME ${tname}
    COMMAND /bin/bash ${CMAKE_SOURCE_DIR}/cmake/test_configurations.sh ${binary} json no_file
  )
  set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

  set(tname test_cfg_json_empty)
  add_test(NAME ${tname}
    COMMAND /bin/bash ${CMAKE_SOURCE_DIR}/cmake/test_configurations.sh ${binary} json empty
  )
  set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

  set(tname test_cfg_json_no_process)
  add_test(NAME ${tname}
    COMMAND /bin/bash ${CMAKE_SOURCE_DIR}/cmake/test_configurations.sh ${binary} json no_process
  )
  set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

  set(tname test_cfg_json_empty_name)
  add_test(NAME ${tname}
    COMMAND /bin/bash ${CMAKE_SOURCE_DIR}/cmake/test_configurations.sh ${binary} json empty_name
  )
  set_tests_properties(${tname} PROPERTIES WILL_FAIL TRUE)

  set(tname test_cfg_json_env)
  add_test(NAME ${tname}
    COMMAND /bin/bash ${CMAKE_SOURCE_DIR}/cmake/test_configurations.sh ${binary} json env
  )
  set_tests_properties(${tname} PROPERTIES WILL_FAIL FALSE)
endif()