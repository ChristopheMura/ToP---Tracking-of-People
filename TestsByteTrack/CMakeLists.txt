cmake_minimum_required(VERSION 3.14)

project(TestsByteTrack)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# ==========================================================
# Emplacement de ByteTrack
# ==========================================================

set(BYTETRACK_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ByteTrack-cpp/include
)

set(BYTETRACK_LIBRARY_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ByteTrack-cpp/build
)


# ==========================================================
# Test 1
# ==========================================================

add_executable(Test1 Test1.cpp)

target_include_directories(Test1 PRIVATE
    ${BYTETRACK_INCLUDE_DIR}
    /usr/include/eigen3
)

target_link_directories(Test1 PRIVATE
    ${BYTETRACK_LIBRARY_DIR}
)

target_link_libraries(Test1 PRIVATE
    bytetrack
)

set_target_properties(Test1 PROPERTIES
    BUILD_RPATH "${BYTETRACK_LIBRARY_DIR}"
)


# ==========================================================
# Test 2
# ==========================================================

add_executable(Test2 2personnes.cpp)

target_include_directories(Test2 PRIVATE
    ${BYTETRACK_INCLUDE_DIR}
    /usr/include/eigen3
)

target_link_directories(Test2 PRIVATE
    ${BYTETRACK_LIBRARY_DIR}
)

target_link_libraries(Test2 PRIVATE
    bytetrack
)

set_target_properties(Test2 PROPERTIES
    BUILD_RPATH "${BYTETRACK_LIBRARY_DIR}"
)


# ==========================================================
# Test 3
# ==========================================================

add_executable(Test3 Occlusion.cpp)

target_include_directories(Test3 PRIVATE
    ${BYTETRACK_INCLUDE_DIR}
    /usr/include/eigen3
)

target_link_directories(Test3 PRIVATE
    ${BYTETRACK_LIBRARY_DIR}
)

target_link_libraries(Test3 PRIVATE
    bytetrack
)

set_target_properties(Test3 PROPERTIES
    BUILD_RPATH "${BYTETRACK_LIBRARY_DIR}"
)