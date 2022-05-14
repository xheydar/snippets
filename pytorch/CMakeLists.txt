cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)

project(main VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

set(PACKAGES_LIBS "")

find_package(PkgConfig REQUIRED)

find_package(Python3 REQUIRED COMPONENTS Development)

pkg_check_modules(YAML_CPP REQUIRED yaml-cpp)
include_directories(${YAML_CPP_INCLUDE_DIRS})
link_directories(${YAML_CPP_LIBRARY_DIRS})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${YAML_CPP_LIBRARIES})

find_package(Torch REQUIRED)
include_directories(${TORCH_INCLUDE_DIRS})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${TORCH_LIBRARIES})

find_package(TorchVision REQUIRED)
set(PACKAGES_LIBS ${PACKAGES_LIBS} TorchVision::TorchVision)

find_package( OpenCV REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${OpenCV_LIBS})

add_subdirectory(./dnn)
set(PACKAGES_LIBS dnn ${PACKAGES_LIBS})

add_executable(main.out main.cpp)
target_link_libraries(main.out ${PACKAGES_LIBS})
