
#	OpenCV 4:
#/opt/opencv4/lib/cmake/opencv4/OpenCVConfig.cmake

#	OpenCV 5:
#/opt/opencv5/lib/cmake/opencv5/OpenCVConfig.cmake

cmake_minimum_required(VERSION 3.16)
project(yolo_detext CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(OpenCV 4 REQUIRED
#    PATHS /opt/opencv4/lib/cmake/opencv4
#    NO_DEFAULT_PATH
#)

find_package(OpenCV 5 REQUIRED
    PATHS /opt/opencv5/lib/cmake/opencv5
    NO_DEFAULT_PATH
)

message(STATUS "OpenCV ${OpenCV_VERSION} depuis ${OpenCV_DIR}")

add_executable(yolo_detext src/main.cpp)
target_compile_options(yolo_detext PRIVATE -Wall -Wextra)
target_link_libraries(yolo_detext PRIVATE ${OpenCV_LIBS})
set_target_properties(yolo_detext PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)