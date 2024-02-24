set(srcs "src/Cobs.c"
         "src/Cobs_frame.c"
         )

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "include"
    REQUIRES 
        LogPrint
        SwFifo
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
