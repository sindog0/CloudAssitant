project(TransferServer LANGUAGES CXX)

add_executable(TransferServer
    ClipboardService.cpp
    FileReceiver.cpp
    TransferCodec.cpp
    TransferConnection.cpp
    TransferServer.cpp
    main.cpp
)

target_link_libraries(TransferServer
    PRIVATE
        cloud_net
)

add_executable(TransferTests
    FileReceiver.cpp
    TransferCodec.cpp
    TransferTests.cpp
)
target_link_libraries(TransferTests PRIVATE cloud_net)
add_test(NAME TransferTests COMMAND TransferTests)
