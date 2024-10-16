
cmake_minimum_required(VERSION 3.5)

# kvsrv
project(KVSrv C CXX)

# Find modules
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED CONFIG)
find_package(gRPC CONFIG REQUIRED)

# set protobuf exec
find_program(_PROTOBUF_PROTOC protoc)
find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)

# Set module variables
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
set(_GRPC_GRPCPP gRPC::grpc++)

add_subdirectory(raft)

# Set paths and sources for .proto files
get_filename_component(kvsrv_proto "./include/protos/kvsrv.proto" ABSOLUTE)
get_filename_component(kvsrv_proto_path "${kvsrv_proto}" PATH)

message(STATUS "Using protobuf ${kvsrv_proto} ${kvsrv_proto_path}")
set(kvsrv_pb_hdrs "${CMAKE_CURRENT_BINARY_DIR}/kvsrv.pb.h")
set(kvsrv_pb_srcs "${CMAKE_CURRENT_BINARY_DIR}/kvsrv.pb.cc")
set(kvsrv_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/kvsrv.grpc.pb.h")
set(kvsrv_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/kvsrv.grpc.pb.cc")

message(STATUS "${kvsrv_pb_hdrs}${CMAKE_CURRENT_BINARY_DIR}")
# Generate .pb.h and .pb.cc files using protoc
add_custom_command(
    OUTPUT "${kvsrv_pb_hdrs}" "${kvsrv_pb_srcs}" "${kvsrv_grpc_hdrs}" "${kvsrv_grpc_srcs}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${kvsrv_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${kvsrv_proto}"
    DEPENDS "${kvsrv_proto}")

# Add the generated library
add_library(kvsrvmodule STATIC
    ${kvsrv_pb_hdrs}
    ${kvsrv_pb_srcs}
    ${kvsrv_grpc_hdrs}
    ${kvsrv_grpc_srcs}
    client.cpp
    server.cpp
    )

target_include_directories(kvsrvmodule PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include)

target_link_libraries(kvsrvmodule
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
  raftmodule)

# 添加可执行文件
add_executable(server_test server_test.cpp)
add_executable(client_test client_test.cpp)

target_link_libraries(server_test kvsrvmodule)
target_link_libraries(client_test kvsrvmodule)

# 链接库文件
if(TARGET raftmodule)
  target_link_libraries(server_test raftmodule)
  target_link_libraries(client_test raftmodule)
endif()



# 安装规则
# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

install(TARGETS kvsrvmodule
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)
cmake_minimum_required(VERSION 3.10)
project(MapReduce VERSION 1.0 LANGUAGES CXX)


