cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)

project(main VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2")

set(PACKAGES_LIBS "")

find_package(PkgConfig REQUIRED)

pkg_check_modules(JSON REQUIRED nlohmann_json)
include_directories(${JSON_INCLUDE_DIRS})
#find_package(nlohmann_json 3.2.0 REQUIRED)

add_executable(main.out main.cpp)
target_link_libraries(main.out ${PACKAGES_LIBS})
