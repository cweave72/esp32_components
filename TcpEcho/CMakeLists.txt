idf_component_register(
    SRCS "src/TcpEcho.c"
    INCLUDE_DIRS "include"
    REQUIRES
        LogPrint
        TcpServer
        )
