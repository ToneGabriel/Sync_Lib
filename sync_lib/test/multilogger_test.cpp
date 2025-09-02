#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <iostream>
#include <sstream>
#include <string>

#include "sync/multilogger.hpp"


class MultiloggerFixture : public ::testing::Test
{
protected:
    sync::multilogger   _multilogger_instance;
    std::ostringstream  _osstream1;
    std::ostringstream  _osstream2;

protected:
    void SetUp() override
    {
        _multilogger_instance.add(_osstream1);
        _multilogger_instance.add(_osstream2);
    }

    void TearDown() override
    {
        _multilogger_instance.clear();
    }
};  // END MultiloggerFixture


TEST_F(MultiloggerFixture, empty)
{
    EXPECT_FALSE(this->_multilogger_instance.empty());
}


TEST_F(MultiloggerFixture, add)
{
    this->_multilogger_instance.clear();
    EXPECT_TRUE(this->_multilogger_instance.empty());

    this->_multilogger_instance.add(std::cout);
    EXPECT_FALSE(this->_multilogger_instance.empty());
}


TEST_F(MultiloggerFixture, clear)
{
    EXPECT_FALSE(this->_multilogger_instance.empty());

    const std::string str = "Hello, Logger!";
    this->_multilogger_instance.clear();

    EXPECT_TRUE(this->_multilogger_instance.empty());
    EXPECT_NO_THROW(this->_multilogger_instance.write(str.data(), str.size()));
    EXPECT_TRUE(this->_osstream1.str().empty());
    EXPECT_TRUE(this->_osstream2.str().empty());
}


TEST_F(MultiloggerFixture, write_to_valid_ostreams)
{
    const std::string str = "Hello, Logger!";

    EXPECT_NO_THROW(this->_multilogger_instance.write(str.data(), str.size()));
    EXPECT_EQ(this->_osstream1.str(), str);
    EXPECT_EQ(this->_osstream2.str(), str);
}


TEST_F(MultiloggerFixture, write_to_invalid_ostreams)
{
    const std::string str = "Hello, Logger!";

    int a;
    this->_multilogger_instance.add(a);

    EXPECT_THROW(this->_multilogger_instance.write(str.data(), str.size()), std::system_error);
}


TEST_F(MultiloggerFixture, skip_error_streams)
{
    this->_osstream1.setstate(std::ios::failbit);   // Simulate a failed stream
    const std::string str = "Hello, Logger!";
    EXPECT_NO_THROW(this->_multilogger_instance.write(str.data(), str.size()));

    EXPECT_EQ(this->_osstream1.str(), "");      // Should not receive data
    EXPECT_EQ(this->_osstream2.str(), str);     // Should receive data
}


TEST_F(MultiloggerFixture, multi_thread_output)
{
    auto write_to_logger =  [this](const std::string& message)
                            {
                                this->_multilogger_instance.write(message.data(), message.size());
                            };

    const std::string thread_message_1 = "Hello from Thread 1!\n";
    const std::string thread_message_2 = "Hello from Thread 2!\n";

    std::thread t1(write_to_logger, thread_message_1);
    std::thread t2(write_to_logger, thread_message_2);

    t1.join();
    t2.join();

    std::string output1 = this->_osstream1.str();
    std::string output2 = this->_osstream2.str();

    EXPECT_TRUE(output1.find(thread_message_1) != std::string::npos);
    EXPECT_TRUE(output1.find(thread_message_2) != std::string::npos);

    EXPECT_TRUE(output2.find(thread_message_1) != std::string::npos);
    EXPECT_TRUE(output2.find(thread_message_2) != std::string::npos);
}
