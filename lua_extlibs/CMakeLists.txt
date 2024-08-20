set(srcs "src/lext_timerlib.c"
         "src/lext_utils.c"
)

idf_component_register(
    SRCS
    ${srcs}
    INCLUDE_DIRS "include"
    REQUIRES
        LogPrint
        SwTimer
        lua
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
