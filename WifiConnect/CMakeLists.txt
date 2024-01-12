idf_component_register(
    SRCS "WifiConnect.c"
    INCLUDE_DIRS "include"
    REQUIRES 
        esp_wifi
        LogPrint
        RtosUtils
        )
