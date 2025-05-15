enable_testing()

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.8.1  
)

FetchContent_MakeAvailable(Catch2)

add_executable(dropclone_tests 
  clone_config_test.cpp
  clone_config_config_entry_test.cpp
  nlohmann_json_parser_test.cpp
)

target_link_libraries(dropclone_tests PRIVATE dropclone_lib Catch2::Catch2WithMain)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

include(Catch)
catch_discover_tests(dropclone_tests)

option(RUN_TESTS "Run tests automatically after build" ON)

if(RUN_TESTS)
  add_custom_target(run_tests ALL
    # COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -V
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS dropclone_tests
  )
endif()