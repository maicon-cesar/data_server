cmake_minimum_required(VERSION 3.6)
 project(Data_Client)

 add_definitions("-std=c++17")

 include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
 conan_basic_setup()

 add_executable(data_client 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data_client.cpp)

 target_include_directories(data_client
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/)

 target_link_libraries(data_client ${CONAN_LIBS})