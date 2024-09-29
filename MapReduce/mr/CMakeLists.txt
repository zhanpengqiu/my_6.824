
cmake_minimum_required(VERSION 3.5)

# MapReduce
project(MapReduce C CXX)

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

# Set paths and sources for .proto files
get_filename_component(worker_proto "./protos/worker.proto" ABSOLUTE)
get_filename_component(worker_proto_path "${worker_proto}" PATH)

get_filename_component(coordinator_proto "./protos/coordinator.proto" ABSOLUTE)
get_filename_component(coordinator_proto_path "${coordinator_proto}" PATH)

message(STATUS "Using protobuf ${worker_proto} ${worker_proto_path}")
set(worker_pb_hdrs "${CMAKE_CURRENT_BINARY_DIR}/worker.pb.h")
set(worker_pb_srcs "${CMAKE_CURRENT_BINARY_DIR}/worker.pb.cc")
set(worker_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/worker.grpc.pb.h")
set(worker_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/worker.grpc.pb.cc")

set(coordinator_pb_hdrs "${CMAKE_CURRENT_BINARY_DIR}/coordinator.pb.h")
set(coordinator_pb_srcs "${CMAKE_CURRENT_BINARY_DIR}/coordinator.pb.cc")
set(coordinator_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/coordinator.grpc.pb.h")
set(coordinator_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/coordinator.grpc.pb.cc")

# Generate .pb.h and .pb.cc files using protoc
add_custom_command(
    OUTPUT "${worker_pb_hdrs}" "${worker_pb_srcs}" "${worker_grpc_hdrs}" "${worker_grpc_srcs}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${coordinator_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${coordinator_proto}"
    DEPENDS "${coordinator_proto}")

add_custom_command(
    OUTPUT "${coordinator_pb_hdrs}" "${coordinator_pb_srcs}" "${coordinator_grpc_hdrs}" "${coordinator_grpc_srcs}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${worker_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${worker_proto}"
    DEPENDS "${worker_proto}")

# Add the generated library
add_library(mrmodule STATIC
    ${worker_pb_hdrs}
    ${worker_pb_srcs}
    ${worker_grpc_hdrs}
    ${worker_grpc_srcs}
    ${coordinator_pb_hdrs}
    ${coordinator_pb_srcs}
    ${coordinator_grpc_hdrs}
    ${coordinator_grpc_srcs}
    coordinator.cpp
    worker.cpp
    )

target_include_directories(mrmodule PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include)

target_link_libraries(mrmodule
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})


# 安装规则
install(TARGETS mrmodule
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)