
enable_testing()
set(binary ${APP_BINARY_DIR}/${CMAKE_PROJECT_NAME})
set(max_only --max-respawn 0)
set(max_and_dir ${max_only} --directory ${CMAKE_SOURCE_DIR}/samples)
set(standalone ${max_only} --path /bin --name ls --arg -a --arg -l --arg -s --arg /home --env wd=10)
 
add_test(NAME test_help 
  COMMAND ${binary} --help
)

add_test(NAME test_version 
  COMMAND ${binary} --version
)

add_test(NAME test_invalid_param
  COMMAND ${binary} --hey
)
set_tests_properties(test_invalid_param PROPERTIES WILL_FAIL TRUE)

add_test(NAME test_new_config_xml
  COMMAND ${binary} -z --directory /tmp/cfg --config test.xml
)
if(NOT HAVE_XML_H)
  set_tests_properties(test_new_config_xml PROPERTIES WILL_FAIL TRUE)
endif()

add_test(NAME test_new_config_json
  COMMAND ${binary} -z --directory /tmp/cfg --config test.json
)
if(NOT HAVE_JSON_H)
  set_tests_properties(test_new_config_json PROPERTIES WILL_FAIL TRUE)
endif()

add_test(NAME test_new_config_error
  COMMAND ${binary} -z --config test.xml
)
set_tests_properties(test_new_config_error PROPERTIES WILL_FAIL TRUE)

add_test(NAME test_default_dir
  COMMAND ${binary} ${max_only} --config watchdog.xml
)
set_tests_properties(test_default_dir PROPERTIES WILL_FAIL TRUE)


add_test(NAME test_invalid_type
  COMMAND ${binary} ${max_and_dir} -t csv
)
set_tests_properties(test_invalid_type PROPERTIES WILL_FAIL TRUE)

add_test(NAME test_cfg_file_xml
  COMMAND ${binary} ${max_and_dir} --config watchdog.xml
)
if(NOT HAVE_XML_H)
  set_tests_properties(test_cfg_file_xml PROPERTIES WILL_FAIL TRUE)
endif()

add_test(NAME test_cfg_file_json
  COMMAND ${binary} ${max_and_dir} --config watchdog.json
)
if(NOT HAVE_JSON_H)
  set_tests_properties(test_cfg_file_json PROPERTIES WILL_FAIL TRUE)
endif()

add_test(NAME test_xml 
  COMMAND ${binary} ${max_and_dir} -t xml
)
if(NOT HAVE_XML_H)
  set_tests_properties(test_xml PROPERTIES WILL_FAIL TRUE)
endif()

add_test(NAME test_json 
  COMMAND ${binary} ${max_and_dir} -t json
)
if(NOT HAVE_JSON_H)
  set_tests_properties(test_json PROPERTIES WILL_FAIL TRUE)
endif()

add_test(NAME test_standalone
  COMMAND ${binary} ${standalone}
)

add_test(NAME test_standalone_error
  COMMAND ${binary} ${max_only} --name lssss
)
set_tests_properties(test_standalone_error PROPERTIES WILL_FAIL TRUE)

add_test(NAME test_standalone_no_spam
  COMMAND ${binary} ${standalone} --disable-spam-detect 
)

add_test(NAME test_standalone_working
  COMMAND ${binary} ${standalone} --working ${CMAKE_SOURCE_DIR}
)

add_test(NAME test_err_max_respawn
  COMMAND ${binary} ${standalone} --max-respawn error
)
set_tests_properties(test_err_max_respawn PROPERTIES WILL_FAIL TRUE)

add_test(NAME test_err_5_max_respawn
  COMMAND ${binary} ${standalone} --max-respawn 5
)
set_tests_properties(test_err_5_max_respawn PROPERTIES WILL_FAIL TRUE)

add_test(NAME test_standalone_pid
  COMMAND ${binary} ${standalone} --pidfile /tmp/wd.pid --pid
)

add_test(NAME test_min_respawn
  COMMAND ${binary} ${standalone} --min-respawn-delay 10
)

add_test(NAME test_err_min_respawn1
  COMMAND ${binary} ${standalone} --min-respawn-delay error
)
set_tests_properties(test_err_min_respawn1 PROPERTIES WILL_FAIL TRUE)

add_test(NAME test_err_min_respawn2
  COMMAND ${binary} ${standalone} --min-respawn-delay -1
)
set_tests_properties(test_err_min_respawn2 PROPERTIES WILL_FAIL TRUE)
