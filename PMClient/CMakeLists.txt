cmake_minimum_required(VERSION 3.28)
project(PMClient)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} main.cpp
        ../PMNetworking/include/PMNetworking/Base.h
        ../PMNetworking/src/Base.cpp)

target_include_directories(${PROJECT_NAME}
        PUBLIC
            PMNetworking
)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
            PMNetworking
)