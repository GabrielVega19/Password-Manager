cmake_minimum_required(VERSION 3.28)
project(Password-Manager VERSION 0.1.0 LANGUAGES C CXX)

add_subdirectory(PMNetworking)

add_subdirectory(PMClient)
add_subdirectory(PMServer)