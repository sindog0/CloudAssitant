cmake_minimum_required(VERSION 3.16)
project(CloudAssistant LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#可执行文件的输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
#开启Debug编译
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(Net)
add_subdirectory(RtmpServer)
add_subdirectory(SigServer)
add_subdirectory(LoginServer)
add_subdirectory(LoadBanlanceServer)
add_subdirectory(Transfer)
