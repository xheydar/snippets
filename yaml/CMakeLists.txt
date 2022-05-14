cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)

project(main VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2")

set(PACKAGES_LIBS "")

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML_CPP REQUIRED yaml-cpp)
include_directories(${YAML_CPP_INCLUDE_DIRS})
link_directories(${YAML_CPP_LIBRARY_DIRS})
message(STATUS ${YAML_CPP_LIBRARIES})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${YAML_CPP_LIBRARIES})

#pkg_check_modules(JSON REQUIRED nlohmann_json)
#include_directories(${JSON_INCLUDE_DIRS})
#find_package(nlohmann_json 3.2.0 REQUIRED)

#find_package(CURL REQUIRED)
#include_directories(${CURL_INCLUDE_DIR})
#set(PACKAGES_LIBS ${PACKAGES_LIBS} ${CURL_LIBRARIES})

#add_subdirectory(./urls)
#set(PACKAGES_LIBS urls ${PACKAGES_LIBS})

add_executable(main.out main.cpp)
target_link_libraries(main.out ${PACKAGES_LIBS})
