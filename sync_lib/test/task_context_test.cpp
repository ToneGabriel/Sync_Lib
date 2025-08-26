#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "sync/task_context.hpp"


// Members tests
// ===========================================================
class TaskContextFixture : public ::testing::Test
{
protected:
    sync::task_context _task_context_instance;
};  // END TaskContextFixture


TEST_F(TaskContextFixture, stopped)
{
    EXPECT_FALSE(this->_task_context_instance.stopped());
}


TEST_F(TaskContextFixture, stop)
{
    EXPECT_FALSE(this->_task_context_instance.stopped());
    this->_task_context_instance.stop();
    EXPECT_TRUE(this->_task_context_instance.stopped());
}


TEST_F(TaskContextFixture, restart)
{
    EXPECT_FALSE(this->_task_context_instance.stopped());
    this->_task_context_instance.stop();
    EXPECT_TRUE(this->_task_context_instance.stopped());
    this->_task_context_instance.restart();
    EXPECT_FALSE(this->_task_context_instance.stopped());
}


// TEST_F(TaskContextFixture, post_and_run)
// {
//     std::vector<std::string> execution_order;
//     std::vector<std::string> expected_order = {"highest", "high", "medium", "low", "lowest"};

//     auto highest_task   = [&]() { execution_order.push_back("highest"); };
//     auto high_task      = [&]() { execution_order.push_back("high"); };
//     auto medium_task    = [&]() { execution_order.push_back("medium"); };
//     auto low_task       = [&]() { execution_order.push_back("low"); };
//     auto lowest_task    = [&]() { execution_order.push_back("lowest"); };

//     (void)sync::post(this->_task_context_instance, sync::priority::lowest, lowest_task);
//     (void)sync::post(this->_task_context_instance, sync::priority::medium, medium_task);
//     (void)sync::post(this->_task_context_instance, sync::priority::low, low_task);
//     (void)sync::post(this->_task_context_instance, sync::priority::highest, highest_task);
//     (void)sync::post(this->_task_context_instance, sync::priority::high, high_task);

//     this->_task_context_instance.run();

//     EXPECT_EQ(execution_order, expected_order);
// }
