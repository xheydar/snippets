cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)

project(main VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2")

find_package(PkgConfig)

set(PACKAGES_LIBS "")

#pkg_check_modules(ZeroMQ QUIET libzmq)
#include_directories(${ZeroMQ_INCLUDE_DIRS})
#link_directories(${ZeroMQ_LIBRARY_DIRS})
#set(PACKAGES_LIBS ${PACKAGES_LIBS} ${ZeroMQ_LIBRARIES})

find_package(cppzmq)
set(PACKAGES_LIBS ${PACKAGE_LIBS} cppzmq)

add_executable(client.out client.cpp)
target_link_libraries(client.out ${PACKAGES_LIBS})

add_executable(broker_proxy.out broker_proxy.cpp)
target_link_libraries(broker_proxy.out ${PACKAGES_LIBS})

add_executable(broker_load_balance.out broker_load_balance.cpp)
target_link_libraries(broker_load_balance.out ${PACKAGES_LIBS})

add_executable(worker.out worker.cpp)
target_link_libraries(worker.out ${PACKAGES_LIBS})