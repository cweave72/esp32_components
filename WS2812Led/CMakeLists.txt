set(srcs "src/WS2812Led.c"
         "src/encoder.c"
    )

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "include"
    REQUIRES
        LogPrint
        SwTimer
        Random
        RtosUtils
        CList
        driver
        )
