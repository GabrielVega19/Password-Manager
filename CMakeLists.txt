cmake_minimum_required(VERSION 3.0.0)
project(Password-Manager VERSION 0.1.0 LANGUAGES C CXX)

include(.env)
add_subdirectory(PMLibrary)
add_subdirectory(PMClient)
add_subdirectory(PMServer)
