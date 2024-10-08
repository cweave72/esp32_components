set(srcs "src/Lfs_Part.c"
         "src/Lfs_PartRpc.c"
         "src/Lfs_PartRpc.pb.c"
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "include"
    REQUIRES
        CList
        CheckCond
        LogPrint
        RtosUtils
        littlefs
        esp_partition
)

# Optionally set local log level for this component.
# LOCAL_DEBUG
# LOCAL_INFO
# LOCAL_VERBOSE
set(local_log_level "LOCAL_INFO")

target_compile_definitions(
    ${COMPONENT_LIB}
    PRIVATE
    "-D${local_log_level}"
    )

set(rpcname "Lfs_PartRpc")
set(nanopb_path $ENV{ESP32_TOOLS}/nanopb)
set(proto_file "${CMAKE_CURRENT_SOURCE_DIR}/src/${rpcname}.proto")
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

## Uncomment build handler template.
add_custom_command(
    COMMAND ${CMAKE_COMMAND} -E echo "[protorpc] Generating handler source template for ${rpcname}."
    COMMAND ${CMAKE_COMMAND} -E touch .protorpc
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/protorpc_out.sh
    VERBATIM
    OUTPUT .protorpc
    DEPENDS ${proto_file}
    )

add_custom_target(
    ${rpcname}_protorpc ALL
    DEPENDS .protorpc
    )
