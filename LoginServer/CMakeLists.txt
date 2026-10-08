project(LoginServer LANGUAGES CXX)

add_executable(LoginServer
    LoginConnection.cpp
    LoginServer.cpp
    ORMManager.cpp
    TcpClient.cpp
    main.cpp
)   


target_link_libraries(LoginServer
    PRIVATE
        mysqlclient
        cloud_net
)

target_include_directories(LoginServer
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)
