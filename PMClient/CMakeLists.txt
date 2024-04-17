cmake_minimum_required(VERSION 3.27.4)
project(PMClient)

include(CTest)
enable_testing()

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        PMLibrary
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC 
        PMLibrary
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)