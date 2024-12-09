cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(
    ThreadPoolProject
    VERSION 1.0.0
    LANGUAGES C CXX)

# Languages
set(CMAKE_C_STANDARD            17)
set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# Names
set(LIBRARY_NAME                "ProjectLibrary")
set(EXECUTABLE_NAME             "ProjectExecutable")
set(CURRENT_DIRECTORY           "./")

# Create project library (link executable and other cpp files to this)
add_library(${LIBRARY_NAME} STATIC)

# CMakeLists subdirectory config
add_subdirectory(configuration)
add_subdirectory(code)
add_subdirectory(app)
