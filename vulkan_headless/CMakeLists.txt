cmake_minimum_required(VERSION 3.10)

project(lve)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
add_definitions(-DDEBUG)

if( UNIX AND APPLE )
    add_definitions(-DUSE_WEBCAM)
endif()

set(PACKAGES_LIBS "")

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
set(PACKAGES_LIBS ${PACKAGES_LIBS} Vulkan::Vulkan)

find_package(PkgConfig REQUIRED)

pkg_check_modules(JSON REQUIRED nlohmann_json)
include_directories(${JSON_INCLUDE_DIRS})

pkg_check_modules(YAML_CPP REQUIRED yaml-cpp)
include_directories(${YAML_CPP_INCLUDE_DIRS})
link_directories(${YAML_CPP_LIBRARY_DIRS})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${YAML_CPP_LIBRARIES})

pkg_check_modules(GLFW3 REQUIRED glfw3)
include_directories(${GLFW3_INCLUDE_DIRS})
link_directories(${GLFW3_LIBRARY_DIRS})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${GLFW3_LIBRARIES})

pkg_check_modules(GLM REQUIRED glm)
include_directories(${GLM_INCLUDE_DIRS})
link_directories(${GLM_LIBRARY_DIRS})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${GLM_LIBRARIES})

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${OpenCV_LIBS})

add_subdirectory(./base64)
set(PACKAGES_LIBS ${PACKAGES_LIBS} base64)

add_subdirectory(vengine)
set(PACKAGES_LIBS ${PACKAGES_LIBS} vengine)

add_executable(main.out main.cpp)
target_link_libraries(main.out ${PACKAGES_LIBS})

add_executable(create_json_config.out create_json_config.cpp)
target_link_libraries(create_json_config.out ${PACKAGES_LIBS})
