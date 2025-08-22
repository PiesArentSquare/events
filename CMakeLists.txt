cmake_minimum_required(VERSION 3.24)
project(Events)

find_package(asio CONFIG REQUIRED)

add_executable(server
    "src/server.cpp"
)
target_include_directories(server PRIVATE "include")
target_compile_features(server PRIVATE cxx_std_20)
target_link_libraries(server PRIVATE asio::asio)

add_executable(client
    "src/client.cpp"
)
target_include_directories(client PRIVATE "include")
target_compile_features(client PRIVATE cxx_std_20)
target_link_libraries(client PRIVATE asio::asio)
