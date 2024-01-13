
idf_component_register(
    SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/UdpRpcServer.c"
    INCLUDE_DIRS "include"
    REQUIRES
        LogPrint
        UdpSocket
        RtosUtils
        )
