#
# cmake_minimum_required(VERSION 3.2)
# project(miniviewer)
#
# message ("cmake: provide the path to libigl via -DLIBIGLDIR=<path_to_libigl>")
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# #set(CMAKE_CXX_LINK_FLAGS "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
#
#
# # pass libigl directory as input to CMake via the variable LIBIGL_DIR
# #set(LIBIGL_DIR /Users/$ENV{USER}/Dropbox/Work/code/libigl/)
# #set(LIBIGL_DIR ${PROJECT_SOURCE_DIR}/../geom_preprocessing/external/TetWild/extern/libigl/)
# set(LIBIGL_INCLUDE_DIR ${LIBIGL_DIR}/include)
# #set(LIBIGL_LIB_DIR ${LIBIGL_DIR}/lib)
# set(LIBIGL_INCLUDE_DIRS ${LIBIGL_INCLUDE_DIR})
# include_directories(${LIBIGL_INCLUDE_DIRS})
# #set(LIBIGL_LIBRARIES
# #${LIBIGL_LIB_DIR}/libigl_core.a
# #${LIBIGL_LIB_DIR}/libigl_opengl_glfw.a
# #${LIBIGL_LIB_DIR}/libigl_opengl_glfw_imgui.a
# #${LIBIGL_LIB_DIR}/libigl_opengl.a
# #)
#
# #add_definitions(-DIGL_STATIC_LIBRARY)
# #list(APPEND SHARED_LIBRARIES ${LIBIGL_LIBRARIES})
# add_definitions(-DLIBIGL_WITH_NANOGUI)
# add_definitions(-DIGL_VIEWER_WITH_NANOGUI)
#
#
# ### EIGEN
# set(EIGEN_INCLUDE_DIRS  ${LIBIGL_DIR}/external/eigen)
# include_directories(${EIGEN_INCLUDE_DIRS})
#
#
# ### OPENGL
# find_package(OpenGL REQUIRED)
# include_directories(${OPENGL_INCLUDE_DIR})
# list(APPEND SHARED_LIBRARIES ${OPENGL_gl_LIBRARY})
#
#
# ### GLAD
# set (GLAD_DIR   ${LIBIGL_DIR}/external/glad)
# include_directories(${GLAD_DIR}/include)
# add_library(glad ${GLAD_DIR}/src/glad.c)
# list(APPEND SHARED_LIBRARIES glad)
#
#
# ### GLFW
# set (GLFW_DIR   ${LIBIGL_DIR}/external/glfw)
# include_directories(${GLFW_DIR}/include)
# set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
# set(GLFW_BUILD_TESTS OFF CACHE BOOL " " FORCE)
# set(GLFW_BUILD_DOCS OFF CACHE BOOL " " FORCE)
# set(GLFW_INSTALL OFF CACHE BOOL " " FORCE)
# add_subdirectory(${GLFW_DIR}/ glfw)
# list(APPEND SHARED_LIBRARIES glfw)
#
# ### IMGUI
# set (IMGUI_DIR   ${LIBIGL_DIR}/external/imgui)
# set(IMGUI_INCLUDE_DIRS ${IMGUI_DIR} ${IMGUI_DIR}/imgui )
# include_directories(${IMGUI_INCLUDE_DIRS})
# add_library(imgui
# ${IMGUI_DIR}/imgui/imgui.cpp
# ${IMGUI_DIR}/imgui/imgui_demo.cpp
# ${IMGUI_DIR}/imgui/imgui_draw.cpp
# ${IMGUI_DIR}/imgui_impl_glfw_gl3.cpp
# )
# list(APPEND SHARED_LIBRARIES imgui)
#
#
# ### XML
# set (TINYXML2_DIR   ${LIBIGL_DIR}/external/tinyxml2)
# set(TINYXML_INCLUDE_DIRS ${TINYXML2_DIR} )
# include_directories(${TINYXML_INCLUDE_DIRS})
# add_library(tinyxml2 STATIC ${TINYXML2_DIR}/tinyxml2.cpp ${TINYXML2_DIR}/tinyxml2.h)
# set_target_properties(tinyxml2 PROPERTIES
# COMPILE_DEFINITIONS "TINYXML2_EXPORT"
# VERSION "3.0.0"
# SOVERSION "3")
# list(APPEND SHARED_LIBRARIES tinyxml2)
#
# ### PNG
# set(STB_IMAGE_DIR "${LIBIGL_DIR}/external/stb_image")
# include_directories(${STB_IMAGE_DIR})
# include_directories(${CMAKE_CURRENT_BINARY_DIR})
# add_library(igl_stb_image
# ${STB_IMAGE_DIR}/igl_stb_image.cpp
# ${STB_IMAGE_DIR}/stb_image.h
# ${STB_IMAGE_DIR}/stb_image_write.h
# )
# include(GenerateExportHeader)
# generate_export_header(igl_stb_image)
# list(APPEND SHARED_LIBRARIES igl_stb_image)
#
#
# # This is necessary to work around the static linking order dependencies on linux
# if(UNIX AND NOT APPLE)
#   set(SHARED_LIBRARIES ${SHARED_LIBRARIES} ${SHARED_LIBRARIES})
# endif(UNIX AND NOT APPLE)
#
# file(GLOB SOURCES
# "${PROJECT_SOURCE_DIR}/*.h"
# "${PROJECT_SOURCE_DIR}/*.hpp"
# "${PROJECT_SOURCE_DIR}/*.cpp"
# )
#
# add_executable(${PROJECT_NAME} ${SOURCES} ${SHARED_SOURCES})
# target_link_libraries(${PROJECT_NAME} ${SHARED_LIBRARIES})


cmake_minimum_required(VERSION 3.1)
project(miniviewer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# libigl
option(LIBIGL_WITH_OPENGL            "Use OpenGL"         ON)
option(LIBIGL_WITH_OPENGL_GLFW       "Use GLFW"           ON)
option(LIBIGL_WITH_OPENGL_GLFW_IMGUI "Use ImGui"          ON)
option(LIBIGL_WITH_XML               "Use XML"            ON)
option(LIBIGL_WITH_PNG               "Use PNG"            ON)

find_package(LIBIGL REQUIRED QUIET)

# Add your project files
file(GLOB SRCFILES *.cpp)
add_executable(${PROJECT_NAME} ${SRCFILES})
target_link_libraries(${PROJECT_NAME} igl::core igl::opengl_glfw igl::opengl_glfw igl::opengl igl::opengl_glfw_imgui igl::xml igl::png)
