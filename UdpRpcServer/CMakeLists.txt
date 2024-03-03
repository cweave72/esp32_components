idf_component_register(
    SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/UdpRpcServer.c"
    INCLUDE_DIRS "include"
    REQUIRES
        UdpServer
        UdpSocket
        LogPrint
        )

# Optionally set local log level for this component.
# LOCAL_DEBUG
# LOCAL_INFO
set(local_log_level "LOCAL_DEBUG")

target_compile_definitions(
    ${COMPONENT_LIB}
    PRIVATE
    "-D${local_log_level}"
    )
