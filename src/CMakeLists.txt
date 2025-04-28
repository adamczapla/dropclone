
add_library(dropclone_lib STATIC 
  run_main.cpp
  drop_clone.cpp
  clone_manager.cpp
  clone_config.cpp
  nlohmann_json_parser.cpp
  path_snapshot.cpp
)

target_include_directories(dropclone_lib PUBLIC 
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/external
)

add_executable(dropclone bootstrap.cpp)

target_link_libraries(dropclone PRIVATE dropclone_lib)