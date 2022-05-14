cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)

project(main VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2")

set(PACKAGES_LIBS "")

#set(B64_CPP_INCLUDE_DIR "/opt/homebrew/Cellar/libb64/1.2.1/include")
#set(B64_CPP_LIBRARIES "b64")
#set(B64_LIBRARY_DIRS "/opt/homebrew/Cellar/libb64/1.2.1/lib")
#include_directories(${B64_CPP_INCLUDE_DIR})
#link_directories(${B64_LIBRARY_DIRS})
#set(PACKAGES_LIBS ${PACKAGES_LIBS} ${B64_CPP_LIBRARIES})

add_subdirectory(./base64)
set(PACKAGES_LIBS base64 ${PACKAGES_LIBS})

add_executable(main.out main.cpp)
target_link_libraries(main.out ${PACKAGES_LIBS})