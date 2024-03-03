set(name "TcpSocket")
set(srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/TcpSocket.c")
         
idf_component_register(
    SRCS ${srcs} 
    INCLUDE_DIRS "include"
    REQUIRES 
        LogPrint
        CheckCond
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

