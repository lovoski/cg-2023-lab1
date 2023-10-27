cmake_minimum_required(VERSION 3.10)

# set compiler argument
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# program name
project(HelloGL C CXX)

# Use ide configuration instead
# if(WIN32)
#  set(CMAKE_TOOLCHAIN_FILE "C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake"
#    CACHE STRING "Vcpkg toolchain file")
#  message(STATUS "use vcpkg")
# endif(WIN32)

# message(STATUS ${CMAKE_PREFIX_PATH})

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -g")
endif()

# create compile file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add package
# find_package(OpenCL REQUIRED)
find_package(Threads)

# set(HelloGL_libs ${HelloGL_libs} ${CMAKE_THREAD_LIBS_INIT} OpenCL::OpenCL)
set(HelloGL_libs ${HelloGL_libs} ${CMAKE_THREAD_LIBS_INIT})

option(OPENMP_ENABLE "Use OpenMP" ON)

if(USE_GCC)

else()
  if(OPENMP_ENABLE)
    find_package(OpenMP REQUIRED)

    if(OpenMP_FOUND)
      add_definitions(-DDYM_USE_OPENMP)
      set(HelloGL_libs ${HelloGL_libs} OpenMP::OpenMP_CXX)
      message(STATUS "-> OpenMP included at ${OpenMP_CXX_INCLUDE_DIRS} ${OpenMP_C_INCLUDE_DIRS}")
    endif(OpenMP_FOUND)
  endif(OPENMP_ENABLE)
endif(USE_GCC)



# !!! link opengl (copy from the source of LearnOpenGL)
# Begin:
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
link_directories(${PROJECT_SOURCE_DIR}/lib)

# find the required packages
find_package(GLM REQUIRED)
message(STATUS "-> GLM included at ${GLM_INCLUDE_DIR}")
if(WIN32)
  find_package(glfw3 CONFIG REQUIRED)
  find_package(assimp CONFIG REQUIRED)
  find_package(Freetype REQUIRED)
else()
  find_package(GLFW3 REQUIRED)
  find_package(ASSIMP REQUIRED)
endif(WIN32)

message(STATUS "-> GLFW3 included at ${GLFW3_INCLUDE_DIR}")
#find_package(ASSIMP REQUIRED)
#message(STATUS "-> ASSIMP included at ${ASSIMP_INCLUDE_DIR}")

# find_package(SOIL REQUIRED)
# message(STATUS "Found SOIL in ${SOIL_INCLUDE_DIR}")
# find_package(GLEW REQUIRED)
# message(STATUS "Found GLEW in ${GLEW_INCLUDE_DIR}")
if(POLICY CMP0072)
  set(OpenGL_GL_PREFERENCE LEGACY)
endif()

if(WIN32)
  find_package(Freetype REQUIRED)
  set(LIBS glfw opengl32 Freetype::Freetype assimp::assimp)
elseif(UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
  find_package(OpenGL REQUIRED)
  add_definitions(${OPENGL_DEFINITIONS})
  message(STATUS "-> OpenGL included at ${OPENGL_INCLUDE_DIR}")
  find_package(X11 REQUIRED)
  message(STATUS "-> X11.X11 included at ${X11_X11_INCLUDE_PATH}")

  # note that the order is important for setting the libs
  # use pkg-config --libs $(pkg-config --print-requires --print-requires-private glfw3) in a terminal to confirm
  set(LIBS ${GLFW3_LIBRARY} X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL dl pthread freetype ${ASSIMP_LIBRARY})
  set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
elseif(APPLE)
  INCLUDE_DIRECTORIES(/System/Library/Frameworks)
  FIND_LIBRARY(COCOA_LIBRARY Cocoa)
  FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
  FIND_LIBRARY(IOKit_LIBRARY IOKit)
  FIND_LIBRARY(CoreVideo_LIBRARY CoreVideo)
  MARK_AS_ADVANCED(COCOA_LIBRARY OpenGL_LIBRARY)
  SET(APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY})
  SET(APPLE_LIBS ${APPLE_LIBS} ${GLFW3_LIBRARY} ${ASSIMP_LIBRARY})
  set(LIBS ${LIBS} ${APPLE_LIBS})
else()
  set(LIBS)
endif(WIN32)

message(STATUS "-> [Fin] Gernate LIBS")

# configure_file(configuration/root_directory.h.in configuration/root_directory.h)
# include_directories(${CMAKE_BINARY_DIR}/configuration)

# Use my own configuration instead
# Setting up the source directory for executable
set(SOURCE_DIR ${CMAKE_SOURCE_DIR})

configure_file(${CMAKE_SOURCE_DIR}/configuration/config.h.in ${CMAKE_BINARY_DIR}/config.h)

include_directories(${CMAKE_BINARY_DIR})

message(STATUS "-> [Fin] Gernate configuration")


set(LIBS ${LIBS} GLAD)
add_library(GLAD "src/glad.c")

file(GLOB IMGUI_CXX_FILE
"includes/imgui/*.cpp"
"includes/imgui/backends/imgui_impl_glfw.cpp"
"includes/imgui/backends/imgui_impl_opengl3.cpp"
)


message(STATUS "-> [Fin] Add library glad")
set(HelloGL_libs ${HelloGL_libs} ${HelloGLCore} ${LIBS} )

set(HelloGL_include_dirs ${HelloGL_include_dirs}
  ${PROJECT_SOURCE_DIR}/includes
  ${PROJECT_SOURCE_DIR}/core
  # include imgui
  ${PROJECT_SOURCE_DIR}/includes/imgui/
  ${PROJECT_SOURCE_DIR}/includes/imgui/backends
  ${GLM_INCLUDE_DIR}
  ${GLFW3_INCLUDE_DIR}
  ${ASSIMP_INCLUDE_DIR}
  ${OpenMP_CXX_INCLUDE_DIRS}
  ${OpenMP_C_INCLUDE_DIRS}
)

message(STATUS "-> [Fin] Add libs and include_dirs")

# set output path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# read main.cpp
file(GLOB MAIN
  "main.cpp"
)

message(STATUS "-> [Fin] Find file: *_main.cpp, *_main.cu")

# read other file
file(GLOB SOURCE
  "src/*.cpp"
  ${IMGUI_CXX_FILE}
)

message(STATUS "-> [Fin] Find source file.")
message(STATUS "->       " ${SOURCE})

message(STATUS "-> [Run] Targetting file... it wil take a few minutes.")

# target
# file(GLOB_RECURSE SRC_FILES "*.c" "*.cpp" "*.cu")
# file(GLOB SRC_FILES "*.c" "*.cpp" "*.cu")

message(STATUS "-> [Fin] Target file.")

foreach(MAIN_PATH ${MAIN})
  string(REGEX REPLACE ".+/(.+)\\..*" "\\1" MAIN_NAME ${MAIN_PATH})
  string(REGEX REPLACE ".+\\.(.+)" "\\1" MAIN_TYPE ${MAIN_PATH})
  string(CONCAT MAIN_NAME ${MAIN_NAME} "")
  message(STATUS "==> build ${MAIN_PATH} (Type: ${MAIN_TYPE}) as")
  message(STATUS "    --> ${MAIN_NAME}")
  set(BUILD_FILE ${MAIN_PATH} ${SOURCE} ) 
  add_executable(${MAIN_NAME} ${BUILD_FILE})

  
  target_link_libraries(${MAIN_NAME} ${HelloGL_libs} )

endforeach(MAIN_PATH)

# include file
include_directories(${HelloGL_include_dirs})

option(BUILD_TESTS "Build tests." ON)

if(${BUILD_TESTS})
  add_subdirectory(tests)
endif(${BUILD_TESTS})
