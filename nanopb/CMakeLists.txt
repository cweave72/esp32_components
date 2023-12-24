set(srcs "src/pb_common.c"
         "src/pb_encode.c"
         "src/pb_decode.c")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS include
                       )
