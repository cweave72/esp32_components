set(srcs "src/lfs.c"
         "src/lfs_util.c"
         "src/lfs_helpers.c"
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "include"
    REQUIRES
        LogPrint
)

target_compile_definitions(
    ${COMPONENT_LIB}
    PUBLIC
    -DLFS_THREADSAFE
)

