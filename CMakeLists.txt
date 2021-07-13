cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 20)

project(quote-server)

# Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# grpc_generate_cpp(QUOTE_PROTO_SRCS QUOTE_PROTO_HDRS "api/quote.proto")

# Generate Quote GRPC bindings
get_filename_component(QUOTE_PROTO "api/quote.proto" ABSOLUTE)
get_filename_component(QUOTE_PROTO_PATH "${QUOTE_PROTO}" DIRECTORY)

set(QUOTE_GRPC_SRC "quote.pb.cc" "quote.pb.h" "quote.grpc.pb.cc" "quote.grpc.pb.h")
list(TRANSFORM QUOTE_GRPC_SRC PREPEND "${CMAKE_CURRENT_BINARY_DIR}/")

add_custom_command(OUTPUT ${QUOTE_GRPC_SRC}
    DEPENDS "${QUOTE_PROTO}"
    COMMAND "${CONAN_BIN_DIRS_PROTOBUF}/protoc"
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${QUOTE_PROTO_PATH}"
        --plugin=protoc-gen-grpc="${CONAN_BIN_DIRS_GRPC}/grpc_cpp_plugin"
        "${QUOTE_PROTO}"
    )

# Targets
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_library(quote_grpc STATIC ${QUOTE_GRPC_SRC})
target_link_libraries(quote_grpc PUBLIC CONAN_PKG::grpc CONAN_PKG::protobuf)

file(GLOB_RECURSE SERVER_SRC CONFIGURE_DEPENDS "server/*.cpp" "server/*.h")
file(GLOB_RECURSE SERVER_TEST_SRC CONFIGURE_DEPENDS "server/*.t.cpp")
list(REMOVE_ITEM SERVER_SRC ${SERVER_TEST_SRC})

list(TRANSFORM SERVER_TEST_SRC REPLACE "\.t\.cpp$" "\.cpp" OUTPUT_VARIABLE SERVER_TEST_DEP)
list(APPEND SERVER_TEST_SRC ${SERVER_TEST_DEP})
list(FILTER SERVER_TEST_SRC EXCLUDE REGEX "server/main\.cpp$")

add_executable(server ${SERVER_SRC})
target_link_libraries(server PRIVATE quote_grpc CONAN_PKG::grpc CONAN_PKG::boost)

add_executable(server_test ${SERVER_TEST_SRC})
target_link_libraries(server_test PRIVATE quote_grpc CONAN_PKG::grpc CONAN_PKG::boost CONAN_PKG::catch2)

install(TARGETS server RUNTIME DESTINATION bin)
