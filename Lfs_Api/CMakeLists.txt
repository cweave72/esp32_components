set(srcs "src/Lfs_Api.c"
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "include"
    REQUIRES
        LogPrint
        Lfs_Part
        Fs_Api
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
