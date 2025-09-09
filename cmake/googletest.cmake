include(FetchContent)

# Declare googletest
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main
)

# Disable installation
set(INSTALL_GTEST OFF CACHE BOOL "Disable installation of googletest" FORCE)

# Check if already available
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    # Make googletest available
    FetchContent_MakeAvailable(googletest)
endif()
