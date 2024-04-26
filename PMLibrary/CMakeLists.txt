cmake_minimum_required(VERSION 3.27.4)
project(PMLibrary)


find_package(Boost 1.84.0 REQUIRED)

file(GLOB SOURCES src/* src/*/* src/*/*/*)
add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${Boost_INCLUDE_DIRS}
        /usr/local/include/cryptopp/
    PRIVATE
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${Boost_LIBRARIES}
        libmariadbcpp.so
        /usr/local/lib/libcryptopp.a
        /usr/lib/x86_64-linux-gnu/libssl.a
        /usr/lib/x86_64-linux-gnu/libcrypto.a
)