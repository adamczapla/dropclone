
add_executable(${PROJECT_NAME} 
  main.cpp
  drop_clone.cpp
  clone_manager.cpp
  clone_config.cpp
  parsers/nlohmann_json_parser.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE 
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/external
)

# target_precompile_headers(${PROJECT_NAME} PRIVATE
#   <nlohmann/json.hpp>
# )