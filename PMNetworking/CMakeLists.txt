cmake_minimum_required(VERSION 3.28)
project(PMNetworking)

set(CMAKE_CXX_STANDARD 20)
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")

FIND_LIBRARY(CRYPTOPP crypto++ C:/Users/jeise/source/cryptopp890)
find_package(Boost 1.84.0 REQUIRED)
file(GLOB SOURCES src/* src/*/* src/*/*/*)
add_library(${PROJECT_NAME} ${SOURCES})

IF ( CRYPTOPP )
    TARGET_LINK_LIBRARIES(${PROGRAM_NAME} cryptopp )
ENDIF( CRYPTOPP )

if(MINGW)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif ()

target_include_directories(${PROJECT_NAME}
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            ${Boost_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
            ${Boost_LIBRARIES}
)