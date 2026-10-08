project(LoadBanlanceServer LANGUAGES CXX)

add_executable(LoadBanlanceServer
    LoadBanlanceConnection.cpp
    LoadBanlanceServer.cpp
    main.cpp
)

target_link_libraries(LoadBanlanceServer
    PRIVATE 
        cloud_net
)