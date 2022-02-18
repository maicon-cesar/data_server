cmake_minimum_required(VERSION 3.6)
 project(Data_Server)

 add_definitions("-std=c++17")

 include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
 conan_basic_setup()

 add_executable(data_server
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/configuration.cpp)
 
 target_include_directories(data_server
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/)
 
 target_link_libraries(data_server ${CONAN_LIBS})