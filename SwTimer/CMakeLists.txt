idf_component_register(
    SRCS "SwTimer.c"
    INCLUDE_DIRS "include"
    REQUIRES 
        esp_timer
        LogPrint
        )
