cmake_minimum_required(VERSION 3.10)
project(MapReduce VERSION 1.0 LANGUAGES CXX)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 添加子目录
add_subdirectory(mr)
# add_subdirectory(mrapps)

# 添加可执行文件
add_executable(mrcoordinator mrcoordinator.cpp)
add_executable(mrworker mrworker.cpp)

# 链接库文件
if(TARGET mrmodule)
  target_link_libraries(mrcoordinator mrmodule)
  target_link_libraries(mrworker mrmodule)
endif()

# 设置编译选项
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)