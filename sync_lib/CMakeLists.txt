set(SYNC_CPP_LIBRARY "Sync_Library")
create_interface_library(
    ${SYNC_CPP_LIBRARY}
    CXX
    20
    ""
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ====================================================================================
set(SYNC_CPP_FULL_TEST "Sync_CPP_FULL_Test")
create_executable(
    ${SYNC_CPP_FULL_TEST}
    ""
    "${SYNC_CPP_LIBRARY};gtest;gmock"
    test/main.cpp
    test/thread_pool_test.cpp
    test/task_context_test.cpp
    test/multilogger_test.cpp
)

create_ctest(SYNC_THREAD_POOL_Tests ${SYNC_CPP_FULL_TEST} --gtest_filter=SyncThreadPool_*)
create_ctest(SYNC_TASK_CONTEXT_Tests ${SYNC_CPP_FULL_TEST} --gtest_filter=SyncTaskContext_*)
create_ctest(SYNC_MULTILOGGER_Tests ${SYNC_CPP_FULL_TEST} --gtest_filter=SyncMultilogger_*)
