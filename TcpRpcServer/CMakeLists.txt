idf_component_register(
    SRCS "src/TcpRpcServer.c"
    INCLUDE_DIRS "include"
    REQUIRES
        LogPrint
        ProtoRpc
        TcpServer
        Cobs
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

