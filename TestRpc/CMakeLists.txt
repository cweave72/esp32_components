set(srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/TestRpc.c"
         "${CMAKE_CURRENT_SOURCE_DIR}/src/TestRpc.pb.c")
         
idf_component_register(
    SRCS ${srcs} 
    INCLUDE_DIRS "include"
    REQUIRES ProtoRpc
    )

set(nanopb_path $ENV{ESP32_TOOLS}/nanopb)
set(proto_file "${CMAKE_CURRENT_SOURCE_DIR}/src/TestRpc.proto")
set(proto_inc_paths
    ${nanopb_path}/generator/proto
    )

# Build the C-bindings for proto_file.
include(${nanopb_path}/nanopb.cmake)
nanopb_build(
    ${proto_file}
    PROTO_PATHS ${proto_inc_paths}
    HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
