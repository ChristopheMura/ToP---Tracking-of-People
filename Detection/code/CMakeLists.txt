
#	OpenCV 4:
#/opt/opencv4/lib/cmake/opencv4/OpenCVConfig.cmake

#	OpenCV 5:
#/opt/opencv5/lib/cmake/opencv5/OpenCVConfig.cmake

cmake_minimum_required(VERSION 3.16)
project(tracking_of_people CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================================
# Emplacement de ByteTrack
# ==========================================================

set(BYTETRACK_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ByteTrack-cpp/include
)

set(BYTETRACK_LIBRARY_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ByteTrack-cpp/build
)


#find_package(OpenCV 4 REQUIRED
#    PATHS /opt/opencv4/lib/cmake/opencv4
#    NO_DEFAULT_PATH
#)

find_package(OpenCV 5 REQUIRED CONFIG
)


message(STATUS "OpenCV ${OpenCV_VERSION} depuis ${OpenCV_DIR}")

add_executable(tracking_of_people src/main.cpp src/mylib.cpp)

target_include_directories(tracking_of_people PRIVATE
    ${BYTETRACK_INCLUDE_DIR}
    /usr/include/eigen3
)

target_link_directories(tracking_of_people PRIVATE
    ${BYTETRACK_LIBRARY_DIR}
)

target_compile_options(tracking_of_people PRIVATE 
    -Wall -Wextra)

target_link_libraries(tracking_of_people PRIVATE 
    ${OpenCV_LIBS}
    bytetrack)

set_target_properties(tracking_of_people PROPERTIES 
    INSTALL_RPATH_USE_LINK_PATH TRUE
    BUILD_RPATH "${BYTETRACK_LIBRARY_DIR}")