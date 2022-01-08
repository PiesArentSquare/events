cmake_minimum_required(VERSION 3.13)
project(Events)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE
    "libs/asio/asio/include"
)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
target_link_libraries(asio INTERFACE wsock32 ws2_32)

add_executable(server
    "src/server.cpp"
)
target_include_directories(server PRIVATE "include")
target_compile_features(server PRIVATE cxx_std_20)
target_link_libraries(server PRIVATE asio)

add_executable(client
    "src/client.cpp"
)
target_include_directories(client PRIVATE "include")
target_compile_features(client PRIVATE cxx_std_20)
target_link_libraries(client PRIVATE asio)
