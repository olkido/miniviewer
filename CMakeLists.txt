
cmake_minimum_required(VERSION 3.2)
project(miniviewer)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_LINK_FLAGS "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")

#if(APPLE)
#  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Deployment target for OSX" FORCE)
#endif(APPLE)

set(LIBIGL_DIR /Users/olkido/Dropbox/Work/code/other/libigl/)
set(LIBIGL_INCLUDE_DIR ${LIBIGL_DIR}/include)
set(LIBIGL_LIB_DIR ${LIBIGL_DIR}/lib)
set(LIBIGL_INCLUDE_DIRS ${LIBIGL_INCLUDE_DIR}  ${LIBIGL_DIR}/external/nanogui/include ${LIBIGL_DIR}/external/nanogui/ext/glfw/include/ ${LIBIGL_DIR}external/nanogui/ext/nanovg/src)
include_directories(${LIBIGL_INCLUDE_DIRS})
set(LIBIGL_LIBRARIES
${LIBIGL_LIB_DIR}/libigl.a
${LIBIGL_LIB_DIR}/libigl_viewer.a
)

add_definitions(-DIGL_STATIC_LIBRARY)
list(APPEND SHARED_LIBRARIES ${LIBIGL_LIBRARIES})
add_definitions(-DLIBIGL_WITH_NANOGUI)
add_definitions(-DIGL_VIEWER_WITH_NANOGUI)


list(APPEND SHARED_LIBRARIES ${LIBIGL_LIB_DIR}/libglfw3.a)
list(APPEND SHARED_LIBRARIES ${LIBIGL_LIB_DIR}/libnanogui.a)

set(EIGEN_INCLUDE_DIRS  ${LIBIGL_DIR}/external/nanogui/ext/eigen)
include_directories(${EIGEN_INCLUDE_DIRS})

# This is necessary to work around the static linking order dependencies on linux
if(UNIX AND NOT APPLE)
  set(SHARED_LIBRARIES ${SHARED_LIBRARIES} ${SHARED_LIBRARIES})
endif(UNIX AND NOT APPLE)

file(GLOB SOURCES
"${PROJECT_SOURCE_DIR}/*.h"
"${PROJECT_SOURCE_DIR}/*.hpp"
"${PROJECT_SOURCE_DIR}/*.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCES} ${SHARED_SOURCES})
target_link_libraries(${PROJECT_NAME} ${SHARED_LIBRARIES})
