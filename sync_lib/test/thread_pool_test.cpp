#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "sync/thread_pool.hpp"


// Helpers
// ===========================================================
void _test_no_return_no_except()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

void _test_throw_std_out_of_range_exception()
{
    _test_no_return_no_except();
    throw std::out_of_range("Out of range exception");
}


// Constructor tests
// ===========================================================
TEST(ThreadPoolConstructor, default_constructor)
{
    sync::thread_pool tp;
    EXPECT_EQ(tp.thread_count(), std::thread::hardware_concurrency());
}


TEST(ThreadPoolConstructor, thread_count_constructor)
{
    sync::thread_pool tp(5);
    EXPECT_EQ(tp.thread_count(), 5);
}


// Members tests
// ===========================================================
class ThreadPoolFixture : public ::testing::Test
{
protected:
    sync::thread_pool _thread_pool_instance;

public:
    ThreadPoolFixture()
        : _thread_pool_instance(1) {}

};  // END ThreadPoolFixture


TEST_F(ThreadPoolFixture, post)
{
    auto no_except_result = sync::post(this->_thread_pool_instance, _test_no_return_no_except);
    auto exception_result = sync::post(this->_thread_pool_instance, _test_throw_std_out_of_range_exception);

    EXPECT_NO_THROW(no_except_result.get());
    EXPECT_THROW(exception_result.get(), std::out_of_range);
}


TEST_F(ThreadPoolFixture, jobs_done)
{
    (void)sync::post(this->_thread_pool_instance, _test_no_return_no_except);
    (void)sync::post(this->_thread_pool_instance, _test_no_return_no_except);

    this->_thread_pool_instance.join();

    EXPECT_EQ(this->_thread_pool_instance.jobs_done(), 2);
}


TEST_F(ThreadPoolFixture, join)
{
    auto no_except_result = sync::post(this->_thread_pool_instance, _test_no_return_no_except);
    this->_thread_pool_instance.join();
    auto exception_result = sync::post(this->_thread_pool_instance, _test_no_return_no_except);

    EXPECT_EQ(this->_thread_pool_instance.jobs_done(), 1);
}


TEST_F(ThreadPoolFixture, stop)
{
    (void)sync::post(this->_thread_pool_instance, _test_no_return_no_except);
    (void)sync::post(this->_thread_pool_instance, _test_no_return_no_except);

    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // wait for pool to start a task from queue (stop() directly might be too fast)
    this->_thread_pool_instance.stop();                         // after 1 task is started, stop the pool (second task is canceled)
    this->_thread_pool_instance.join();                         // wait for first task to finish

    EXPECT_EQ(this->_thread_pool_instance.jobs_done(), 1);
}
