# get googletest project
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

# disable googletest installation
set(INSTALL_GTEST OFF CACHE BOOL "Disable installation of googletest" FORCE)

FetchContent_MakeAvailable(googletest)

# add headers from googletest and googlemock to this library
target_include_directories(${TEST_LIBRARY_NAME} PUBLIC ${gtest_SOURCE_DIR}/include)
target_include_directories(${TEST_LIBRARY_NAME} PUBLIC ${gmock_SOURCE_DIR}/include)

# get test source files
target_sources(${TEST_LIBRARY_NAME} PUBLIC
    thread_pool_test.cpp
    multilogger_test.cpp
)

# create test executable
add_executable(${TEST_EXECUTABLE_NAME} main.cpp)

# link to executable the following - test library, gtest, gmock
target_link_libraries(${TEST_EXECUTABLE_NAME} PUBLIC ${TEST_LIBRARY_NAME} gtest gmock)
