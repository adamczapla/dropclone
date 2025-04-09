
add_executable(${PROJECT_NAME} 
  main.cpp
  drop_clone.cpp
  clone_manager.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE 
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/external
)