cmake_minimum_required(VERSION 3.13)
project(Events)

add_executable(events
    "src/main.cpp"
)
target_include_directories(events PRIVATE "include")
target_compile_features(events PRIVATE cxx_std_20)
