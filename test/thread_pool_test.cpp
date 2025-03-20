#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <unordered_map>
#include "thread_pool.h"


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

class FunctorClassTest
{
public:

    void operator()()
    {
        _test_no_return_no_except();
    }

    void test_member_function(int)
    {
        _test_no_return_no_except();
    }
};  // END FunctorClassTest

enum class functor_type
{
    e_default_no_return_no_except,
    e_default_throw_std_out_of_range_exception,
    e_callable_object,
    e_member_function
};

// ======================================================================================================================
// Tests
// ======================================================================================================================

// TEST(ThreadPoolConstructor, default_constructor)
// {
//     sync::thread_pool tp;

//     EXPECT_EQ(tp.thread_count(), std::thread::hardware_concurrency());
// }

// TEST(ThreadPoolConstructor, thread_count_constructor)
// {
//     sync::thread_pool tp(5);

//     EXPECT_EQ(tp.thread_count(), 5);
// }


// ======================================================================================================================
// Job queue init tests
// ======================================================================================================================
class ThreadPoolQueueInitFixture : public ::testing::Test
{
protected:
    sync::thread_pool _thread_pool_instance;
    std::unordered_map< functor_type,
                        std::future<void>> _result_list;

public:

    ThreadPoolQueueInitFixture()
        : _thread_pool_instance(2) { /*Empty*/ }

protected:

    void SetUp() override
    {
        this->_result_list[functor_type::e_default_no_return_no_except] = 
                            this->_thread_pool_instance.do_job(_test_no_return_no_except);

        this->_result_list[functor_type::e_default_throw_std_out_of_range_exception] = 
                            this->_thread_pool_instance.do_job(_test_throw_std_out_of_range_exception);

        this->_result_list[functor_type::e_callable_object] = 
                            this->_thread_pool_instance.do_job(FunctorClassTest());

        this->_result_list[functor_type::e_member_function] = 
                            this->_thread_pool_instance.do_job(&FunctorClassTest::test_member_function, FunctorClassTest(), 0);
    }
};  // END ThreadPoolQueueInitFixture

TEST_F(ThreadPoolQueueInitFixture, restart)
{
    EXPECT_EQ(this->_thread_pool_instance.thread_count(), 2);

    this->_thread_pool_instance.restart(5);
    EXPECT_NE(this->_thread_pool_instance.pending_jobs(), 0);
    EXPECT_EQ(this->_thread_pool_instance.thread_count(), 5);
}

TEST_F(ThreadPoolQueueInitFixture, do_job)
{
    EXPECT_NE(this->_thread_pool_instance.pending_jobs(), 0);

    EXPECT_NO_THROW(this->_result_list[functor_type::e_default_no_return_no_except].get());
    EXPECT_THROW(this->_result_list[functor_type::e_default_throw_std_out_of_range_exception].get(), std::out_of_range);
    EXPECT_NO_THROW(this->_result_list[functor_type::e_callable_object].get());
    EXPECT_NO_THROW(this->_result_list[functor_type::e_member_function].get());

    EXPECT_EQ(this->_thread_pool_instance.pending_jobs(), 0);
}

TEST_F(ThreadPoolQueueInitFixture, pause_resume)
{
    this->_thread_pool_instance.pause();
    EXPECT_TRUE(this->_thread_pool_instance.is_paused());

    size_t jobs_done_after_pause = this->_thread_pool_instance.jobs_done();
    this->_thread_pool_instance.resume();
    EXPECT_EQ(this->_thread_pool_instance.jobs_done(), jobs_done_after_pause);
    EXPECT_FALSE(this->_thread_pool_instance.is_paused());
}

TEST_F(ThreadPoolQueueInitFixture, join_not_paused)
{
    EXPECT_NE(this->_thread_pool_instance.pending_jobs(), 0);

    this->_thread_pool_instance.join();
    EXPECT_TRUE(this->_thread_pool_instance.is_joined());
    EXPECT_EQ(this->_thread_pool_instance.thread_count(), 0);
    EXPECT_EQ(this->_thread_pool_instance.pending_jobs(), 0);
}

TEST_F(ThreadPoolQueueInitFixture, join_paused)
{
    this->_thread_pool_instance.pause();
    EXPECT_NE(this->_thread_pool_instance.pending_jobs(), 0);

    this->_thread_pool_instance.join();
    EXPECT_TRUE(this->_thread_pool_instance.is_joined());
    EXPECT_EQ(this->_thread_pool_instance.thread_count(), 0);
    EXPECT_EQ(this->_thread_pool_instance.pending_jobs(), 0);
}

TEST_F(ThreadPoolQueueInitFixture, force_join_not_paused)
{
    EXPECT_NE(this->_thread_pool_instance.pending_jobs(), 0);

    this->_thread_pool_instance.force_join();
    EXPECT_TRUE(this->_thread_pool_instance.is_joined());
    EXPECT_EQ(this->_thread_pool_instance.thread_count(), 0);
    EXPECT_NE(this->_thread_pool_instance.pending_jobs(), 0);
}

TEST_F(ThreadPoolQueueInitFixture, force_join_paused)
{
    this->_thread_pool_instance.pause();
    EXPECT_NE(this->_thread_pool_instance.pending_jobs(), 0);

    this->_thread_pool_instance.force_join();
    EXPECT_TRUE(this->_thread_pool_instance.is_joined());
    EXPECT_EQ(this->_thread_pool_instance.thread_count(), 0);
    EXPECT_NE(this->_thread_pool_instance.pending_jobs(), 0);
}

TEST_F(ThreadPoolQueueInitFixture, clear_pending_jobs)
{
    EXPECT_NE(this->_thread_pool_instance.pending_jobs(), 0);

    this->_thread_pool_instance.clear_pending_jobs();
    EXPECT_EQ(this->_thread_pool_instance.pending_jobs(), 0);
}

// ======================================================================================================================
// Job store init tests
// ======================================================================================================================
class ThreadPoolStoreInitFixture : public ::testing::Test
{
    protected:
    sync::thread_pool _thread_pool_instance;
    std::unordered_map< functor_type,
                        std::future<void>> _result_list;

public:

    ThreadPoolStoreInitFixture()
        : _thread_pool_instance(2) { /*Empty*/ }

protected:

    void SetUp() override
    {
        this->_result_list[functor_type::e_default_no_return_no_except] = 
                            this->_thread_pool_instance.store_job(_test_no_return_no_except);

        this->_result_list[functor_type::e_default_throw_std_out_of_range_exception] = 
                            this->_thread_pool_instance.store_job(_test_throw_std_out_of_range_exception);

        this->_result_list[functor_type::e_callable_object] = 
                            this->_thread_pool_instance.store_job(FunctorClassTest());

        this->_result_list[functor_type::e_member_function] = 
                            this->_thread_pool_instance.store_job(&FunctorClassTest::test_member_function, FunctorClassTest(), 0);
    }
};  // END ThreadPoolStoreInitFixture

TEST_F(ThreadPoolStoreInitFixture, store_job)
{
    EXPECT_EQ(this->_thread_pool_instance.pending_jobs(), 0);
    EXPECT_EQ(this->_thread_pool_instance.stored_jobs(), 4);
}

TEST_F(ThreadPoolStoreInitFixture, flush_job_storage)
{
    EXPECT_EQ(this->_thread_pool_instance.pending_jobs(), 0);
    EXPECT_EQ(this->_thread_pool_instance.stored_jobs(), 4);

    this->_thread_pool_instance.flush_job_storage();
    EXPECT_NE(this->_thread_pool_instance.pending_jobs(), 0);
    EXPECT_EQ(this->_thread_pool_instance.stored_jobs(), 0);
}

TEST_F(ThreadPoolStoreInitFixture, clear_stored_jobs)
{
    EXPECT_EQ(this->_thread_pool_instance.pending_jobs(), 0);
    EXPECT_EQ(this->_thread_pool_instance.stored_jobs(), 4);

    this->_thread_pool_instance.clear_stored_jobs();
    EXPECT_EQ(this->_thread_pool_instance.pending_jobs(), 0);
    EXPECT_EQ(this->_thread_pool_instance.stored_jobs(), 0);
}