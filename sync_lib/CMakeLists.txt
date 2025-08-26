
set(SYNC_CPP_LIBRARY "Sync_Library")
set(SYNC_CPP_FULL_TEST "Sync_CPP_FULL_Test")

create_library(${SYNC_CPP_LIBRARY} INTERFACE CXX 20 ${CMAKE_CURRENT_SOURCE_DIR}/include)

create_executable(${SYNC_CPP_FULL_TEST}
    "${SYNC_CPP_LIBRARY};gtest;gmock"
    test/main.cpp
    test/thread_pool_test.cpp
    test/task_context_test.cpp
)
