cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)

project(main VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2")

set(PACKAGES_LIBS "")

find_package(PkgConfig REQUIRED)
pkg_search_module(YAML_CPP REQUIRED yaml-cpp)
include_directories(${YAML_CPP_INCLUDE_DIRS})
link_directories(${YAML_CPP_LIBRARY_DIRS})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${YAML_CPP_LIBRARIES})

find_package(Boost COMPONENTS system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${Boost_SYSTEM_LIBRARY})

add_subdirectory(./twitch)
set(PACKAGES_LIBS twitch ${PACKAGES_LIBS})

add_executable(twitch.out twitch.cpp)
target_link_libraries(twitch.out ${PACKAGES_LIBS})

add_executable(basic.out basic.cpp)
target_link_libraries(basic.out ${PACKAGES_LIBS})
