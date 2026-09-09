project(RtmpServer LANGUAGES CXX)

add_executable(RtmpServer
    Amf.cpp
    RtmpChunk.cpp
    RtmpConnection.cpp
    RtmpHandshake.cpp
    RtmpServer.cpp
    RtmpSession.cpp
    main.cpp
)

target_link_libraries(RtmpServer
    PRIVATE
        cloud_net
)

target_include_directories(RtmpServer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(RtmpServer
    PRIVATE
        cxx_std_17
)