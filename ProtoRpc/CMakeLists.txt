set(srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/ProtoRpc.c"
         "${CMAKE_CURRENT_SOURCE_DIR}/src/ProtoRpc.pb.c")

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "include"
    REQUIRES
        PbGeneric
        LogPrint
    )

# Optionally set local log level for this component.
# LOCAL_DEBUG
# LOCAL_INFO
set(local_log_level "LOCAL_INFO")

target_compile_definitions(
    ${COMPONENT_LIB}
    PRIVATE
    "-D${local_log_level}"
    )

set(nanopb_path $ENV{ESP32_TOOLS}/nanopb)
set(proto_file "${CMAKE_CURRENT_SOURCE_DIR}/src/ProtoRpc.proto")
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
