cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(
    SyncLibrary
    VERSION 1.0.0
    LANGUAGES C CXX)

# Languages
set(CMAKE_C_STANDARD            17)
set(CMAKE_C_STANDARD_REQUIRED   ON)
set(CMAKE_C_EXTENSIONS          OFF)

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# Library destinations
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Names
set(TEST_LIBRARY_NAME       "SyncTestLibrary")
set(TEST_EXECUTABLE_NAME    "SyncTest")

# Create project library
add_library(${TEST_LIBRARY_NAME} INTERFACE)
target_include_directories(${TEST_LIBRARY_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# create test executable
add_executable(${TEST_EXECUTABLE_NAME}
    test/main.cpp
    test/thread_pool_test.cpp
    test/multilogger_test.cpp
)

# get googletest project
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main
)

# disable googletest installation
set(INSTALL_GTEST OFF CACHE BOOL "Disable installation of googletest" FORCE)

FetchContent_MakeAvailable(googletest)

# link to executable the following - test library, gtest, gmock
target_link_libraries(${TEST_EXECUTABLE_NAME} PUBLIC ${TEST_LIBRARY_NAME} gtest gmock)


# CPack settings
# set(CPACK_GENERATOR "ZIP")
# set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
# set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
# include(CPack)

# install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include FILES_MATCHING PATTERN "*.h")