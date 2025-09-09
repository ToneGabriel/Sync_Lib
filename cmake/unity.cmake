include(FetchContent)

# Declare Unity
FetchContent_Declare(
    Unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        master
)

FetchContent_GetProperties(Unity)

# Check if already available
if(NOT Unity_POPULATED)
    # Make unity available
    FetchContent_MakeAvailable(Unity)
endif()
