cmake_minimum_required(VERSION 3.10)
project(rs)

find_package(Threads)

#set(CMAKE_CXX_FLAGS "-Wall -Werror -pedantic -g -O3")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
		${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
	)
endif()

find_program(
	CLANG_TIDY_EXE
	NAMES "clang-tidy"
	DOC "Path to clang-tidy executable"
)

find_program(
	CLANG_FORMAT_EXE
	NAMES "clang-format"
	DOC "Path to clang-format executable"
)

if(NOT CLANG_TIDY_EXE)
	message(STATUS "clang-tidy not found.")
else()
	message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
	set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.*,-fuchsia*")
endif()

if(NOT CLANG_FORMAT_EXE)
	message(STATUS "clang-format not found.")
else()
	message(STATUS "clang-format found: ${CLANG_FORMAT_EXE}")
	set(DO_CLANG_FORMAT "${CLANG_FORMAT_EXE}" "-istyle=Google -i ${ALL_SOURCE_FILES}")
endif()

set(rs-server_SOURCES
	src/server.cc
)

set(rs-client_SOURCES
	src/client.cc
)

add_subdirectory(third_party/grpc EXCLUDE_FROM_ALL)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
set(_PROTOBUF_LIBPROTOBUF libprotobuf)

get_filename_component(hw_proto "${CMAKE_CURRENT_SOURCE_DIR}/proto/helloworld.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.grpc.pb.h")
add_custom_command(
	OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
	COMMAND ${_PROTOBUF_PROTOC}
	ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
	     --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
	     -I "${hw_proto_path}"
	     --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
	     "${hw_proto}"
	     DEPENDS "${hw_proto}")

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

foreach(_target
	rs-server rs-client)
add_executable(${_target} ${${_target}_SOURCES}
	${hw_proto_srcs}
	${hw_grpc_srcs})
target_link_libraries(${_target}
	grpc++
	${_PROTOBUF_LIBPROTOBUF} ${CMAKE_THREAD_LIBS_INIT})

	set_target_properties(
		${_target} PROPERTIES
		CXX_STANDARD 14
		CXX_STANDARD_REQUIRED ON
		COMPILE_FLAGS "-D_GNU_SOURCE -DGNU_SOURCE -Wall -Werror -pedantic -O3 -rdynamic -g"
	)
	if(CLANG_TIDY_EXE AND CLANG_FORMAT_EXE)
		set_target_properties(
			${_target} PROPERTIES
			CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
			CXX_CLANG_FORMAT "${DO_CLANG_FORMAT}")
	endif()
endforeach()
