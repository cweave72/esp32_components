idf_component_register(
    SRCS "src/UdpEcho.c"
    INCLUDE_DIRS "include"
    REQUIRES
        UdpServer
        UdpSocket
        LogPrint
        )
