cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(
    SyncLibrary
    VERSION 1.1.3
    LANGUAGES CXX
)

# Get googletest project
include(cmake/googletest.cmake)

# Add utils
include(cmake/internal_utils.cmake)

# Configure sync library
add_subdirectory(sync_lib)
