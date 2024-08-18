set(srcs "src/lua_thread.c"
         "src/lua_thread_rpc.c"
         "src/lua_thread_rpc.pb.c"
         "demos/script_demo1.c"
         "demos/script_demo2.c"
         "custom/linit.c"
)

set(lua_src_base ${CMAKE_CURRENT_SOURCE_DIR}/lua)

# Gather all source files.
file(GLOB lua_srcs "${lua_src_base}/*.c")
# Remove source not included in the core.
list(FILTER lua_srcs EXCLUDE REGEX ".*/lua.c")
list(FILTER lua_srcs EXCLUDE REGEX ".*/onelua.c")
# Remote libraries not included (see custom/lualib.h)
list(FILTER lua_srcs EXCLUDE REGEX ".*/linit.c")
list(FILTER lua_srcs EXCLUDE REGEX ".*/lmathlib.c")
list(FILTER lua_srcs EXCLUDE REGEX ".*/loslib.c")
list(FILTER lua_srcs EXCLUDE REGEX ".*/lcorolib.c")
list(FILTER lua_srcs EXCLUDE REGEX ".*/lutf8lib.c")
list(FILTER lua_srcs EXCLUDE REGEX ".*/liolib.c")

set(INCLUDE_DIRS
    "include"
    "custom/inc"
    "custom"
)

idf_component_register(
    SRCS
        ${lua_srcs}
        ${srcs}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    REQUIRES
        LogPrint
        Fs_Api
        RtosUtils
)

# Optionally set local log level for this component.
# LOCAL_DEBUG
# LOCAL_INFO
# LOCAL_VERBOSE
set(local_log_level "LOCAL_DEBUG")

target_compile_definitions(
    ${COMPONENT_LIB}
    PRIVATE
    "-D${local_log_level}"
    )

set(rpcname "lua_thread_rpc")
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
