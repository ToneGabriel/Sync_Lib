#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <sstream>
#include <thread>

#include "sync/multilogger.h"

// ======================================================================================================================
// Tests
// ======================================================================================================================
class MultiloggerFixture : public ::testing::Test
{
protected:
    std::ostringstream _osstream1;
    std::ostringstream _osstream2;
    sync::multilogger _multilogger_instance;

protected:
    void SetUp() override
    {
        _multilogger_instance.add(_osstream1);
        _multilogger_instance.add(_osstream2);
    }
};  // END MultiloggerFixture

TEST_F(MultiloggerFixture, add)
{
    EXPECT_FALSE(this->_multilogger_instance.empty());
}

TEST_F(MultiloggerFixture, clear)
{
    this->_multilogger_instance.clear();
    this->_multilogger_instance << "No Output";

    EXPECT_TRUE(this->_osstream1.str().empty());
    EXPECT_TRUE(this->_osstream2.str().empty());
}

TEST_F(MultiloggerFixture, single_thread_output)
{
    this->_multilogger_instance << "Hello, Logger!";

    EXPECT_EQ(this->_osstream1.str(), "Hello, Logger!");
    EXPECT_EQ(this->_osstream2.str(), "Hello, Logger!");
}

TEST_F(MultiloggerFixture, multi_thread_output)
{
    auto write_to_logger =  [this](const std::string& message)
                            {
                                this->_multilogger_instance << message;
                            };

    std::thread t1(write_to_logger, "Hello from Thread 1!\n");
    std::thread t2(write_to_logger, "Hello from Thread 2!\n");

    t1.join();
    t2.join();

    std::string output1 = this->_osstream1.str();
    std::string output2 = this->_osstream2.str();

    EXPECT_TRUE(output1.find("Hello from Thread 1!\n") != std::string::npos);
    EXPECT_TRUE(output1.find("Hello from Thread 2!\n") != std::string::npos);

    EXPECT_TRUE(output2.find("Hello from Thread 1!\n") != std::string::npos);
    EXPECT_TRUE(output2.find("Hello from Thread 2!\n") != std::string::npos);
}

TEST_F(MultiloggerFixture, skip_error_streams)
{
    this->_osstream1.setstate(std::ios::failbit);   // Simulate a failed stream
    this->_multilogger_instance << "Test Data";

    EXPECT_EQ(this->_osstream1.str(), "");          // Should not receive data
    EXPECT_EQ(this->_osstream2.str(), "Test Data"); // Should receive data
}

TEST_F(MultiloggerFixture, multiple_data_types)
{
    this->_multilogger_instance << 42 << " " << 3.14;

    EXPECT_EQ(this->_osstream1.str(), "42 3.14");
    EXPECT_EQ(this->_osstream2.str(), "42 3.14");
}
