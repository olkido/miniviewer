
cmake_minimum_required(VERSION 3.2)
project(miniviewer)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_LINK_FLAGS "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")

set(LIBIGL_DIR /Users/$ENV{USER}/Dropbox/Work/code/other/libigl/)
set(LIBIGL_INCLUDE_DIR ${LIBIGL_DIR}/include)
set(LIBIGL_LIB_DIR ${LIBIGL_DIR}/lib)
set(LIBIGL_INCLUDE_DIRS ${LIBIGL_INCLUDE_DIR})
include_directories(${LIBIGL_INCLUDE_DIRS})
set(LIBIGL_LIBRARIES
${LIBIGL_LIB_DIR}/libigl_core.a
${LIBIGL_LIB_DIR}/libigl_opengl_glfw.a
${LIBIGL_LIB_DIR}/libigl_opengl_glfw_imgui.a
${LIBIGL_LIB_DIR}/libigl_opengl.a
)

add_definitions(-DIGL_STATIC_LIBRARY)
list(APPEND SHARED_LIBRARIES ${LIBIGL_LIBRARIES})

### GLFW
include_directories(${LIBIGL_DIR}/external/glfw/include)
list(APPEND SHARED_LIBRARIES ${LIBIGL_LIB_DIR}/libglfw3.a)

### EIGEN
set(EIGEN_INCLUDE_DIRS  ${LIBIGL_DIR}/external/eigen)
include_directories(${EIGEN_INCLUDE_DIRS})

### GLAD
include_directories(${LIBIGL_DIR}/external/glad/include)
list(APPEND SHARED_LIBRARIES ${LIBIGL_DIR}/external/glad/build/libglad.a)

### IMGUI
set (IMGUI_DIR   ${LIBIGL_DIR}/external/imgui)
set(IMGUI_INCLUDE_DIRS ${IMGUI_DIR} ${IMGUI_DIR}/imgui )
include_directories(${IMGUI_INCLUDE_DIRS})
add_library(imgui
${IMGUI_DIR}/imgui/imgui.cpp
${IMGUI_DIR}/imgui/imgui_demo.cpp
${IMGUI_DIR}/imgui/imgui_draw.cpp
${IMGUI_DIR}/imgui_impl_glfw_gl3.cpp
)
list(APPEND SHARED_LIBRARIES imgui)


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
