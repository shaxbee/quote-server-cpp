cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(FETCHCONTENT_QUIET OFF)

project(quote-server)

include(FetchContent)

# Boost
find_package(Boost 1.67.0 REQUIRED COMPONENTS "json")

# GRPC
FetchContent_Declare(
    gRPC
    GIT_REPOSITORY https://github.com/grpc/grpc
    GIT_TAG        v1.38.1
)

set(gRPC_BUILD_CSHARP_EXT OFF CACHE INTERNAL "")
set(gRPC_BUILD_GRPC_CSHARP_PLUGIN OFF CACHE INTERNAL "")
set(gRPC_BUILD_GRPC_NODE_PLUGIN OFF CACHE INTERNAL "")
set(gRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN OFF CACHE INTERNAL "")
set(gRPC_BUILD_GRPC_PHP_PLUGIN OFF CACHE INTERNAL "")
set(gRPC_BUILD_GRPC_PYTHON_PLUGIN OFF CACHE INTERNAL "")
set(gRPC_BUILD_GRPC_RUBY_PLUGIN OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(gRPC)

set(PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
set(GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)

# Catch2
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v2.13.1
)
FetchContent_MakeAvailable(Catch2)

# Generate Quote GRPC bindings
get_filename_component(QUOTE_PROTO "api/quote.proto" ABSOLUTE)
get_filename_component(QUOTE_PROTO_PATH "${QUOTE_PROTO}" DIRECTORY)

set(QUOTE_GRPC_SRC "quote.pb.cc" "quote.pb.h" "quote.grpc.pb.cc" "quote.grpc.pb.h")
list(TRANSFORM QUOTE_GRPC_SRC PREPEND "${CMAKE_CURRENT_BINARY_DIR}/")

add_custom_command(OUTPUT ${QUOTE_GRPC_SRC}
    DEPENDS "${QUOTE_PROTO}"
    COMMAND "${PROTOBUF_PROTOC}"
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${QUOTE_PROTO_PATH}"
        --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${QUOTE_PROTO}"
    )

# Targets
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories("include")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_library(quote_grpc ${QUOTE_GRPC_SRC})
target_link_libraries(quote_grpc PRIVATE grpc++ libprotobuf)

file(GLOB_RECURSE SERVER_SRC CONFIGURE_DEPENDS "server/*.h" "server/*.cpp")

add_executable(server ${SERVER_SRC})
target_link_libraries(server PRIVATE quote_grpc grpc++_reflection Boost::json)

