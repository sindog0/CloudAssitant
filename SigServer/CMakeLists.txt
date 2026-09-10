project(SigServer LANGUAGES CXX)

add_executable(SigServer
    SigServer.cpp
    SigConnection.cpp
    ConnectionManager.cpp
    main.cpp
)

target_link_libraries(SigServer
    PRIVATE
        cloud_net
)

