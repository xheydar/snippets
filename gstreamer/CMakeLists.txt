cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)

project(main VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2")

set(PACKAGES_LIBS "")

find_package( OpenCV REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${OpenCV_LIBS})

find_package(PkgConfig)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
include_directories(${GSTREAMER_INCLUDE_DIRS})
link_directories(${GSTREAMER_LIBRARY_DIRS})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${GSTREAMER_LIBRARIES})

pkg_check_modules(GST_VIDEO REQUIRED gstreamer-video-1.0)
include_directories(${GST_VIDEO_INCLUDE_DIRS})
link_directories(${GST_VIDEO_LIBRARY_DIRS})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${GST_VIDEO_LIBRARIES})

pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
include_directories(${GST_APP_INCLUDE_DIRS})
link_directories(${GST_APP_LIBRARY_DIRS})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${GST_APP_LIBRARIES})

pkg_check_modules(GST_BASE REQUIRED gstreamer-plugins-base-1.0)
include_directories(${GST_BASE_INCLUDE_DIRS})
link_directories(${GST_BASE_LIBRARY_DIRS})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${GST_BASE_LIBRARIES})

pkg_check_modules(GST_INSERT REQUIRED gstreamer-insertbin-1.0)
include_directories(${GST_INSERT_INCLUDE_DIRS})
link_directories(${GST_INSERT_LIBRARY_DIRS})
set(PACKAGES_LIBS ${PACKAGES_LIBS} ${GST_INSERT_LIBRARIES})

message(STATUS ${GSTREAMER_LIBRARIES})
message(STATUS ${GST_VIDEO_LIBRARIES})
message(STATUS ${GST_APP_LIBRARIES})

add_executable(code0.out code0.cpp)
target_link_libraries(code0.out ${PACKAGES_LIBS})

add_executable(code1.out code1.cpp)
target_link_libraries(code1.out ${PACKAGES_LIBS})

add_executable(code2.out code2.cpp)
target_link_libraries(code2.out ${PACKAGES_LIBS})

add_executable(webcam.out webcam.cpp)
target_link_libraries(webcam.out ${PACKAGES_LIBS})

add_executable(string_pipeline.out string_pipeline.cpp)
target_link_libraries(string_pipeline.out ${PACKAGES_LIBS})

add_executable(string_pipeline_simple.out string_pipeline_simple.cpp)
target_link_libraries(string_pipeline_simple.out ${PACKAGES_LIBS})

add_executable(string_bin.out string_bin.cpp)
target_link_libraries(string_bin.out ${PACKAGES_LIBS})
