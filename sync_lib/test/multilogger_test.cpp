#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <iostream>

#include "sync/multilogger.hpp"


class MultiloggerFixture : public ::testing::Test
{
protected:
    sync::multilogger _multilogger_instance;
};  // END MultiloggerFixture


TEST_F(MultiloggerFixture, empty)
{
    EXPECT_TRUE(this->_multilogger_instance.empty());
}


TEST_F(MultiloggerFixture, add)
{
    EXPECT_TRUE(this->_multilogger_instance.empty());

    this->_multilogger_instance.add(std::cout);

    EXPECT_FALSE(this->_multilogger_instance.empty());
}


TEST_F(MultiloggerFixture, clear)
{
    this->_multilogger_instance.add(std::cout);

    EXPECT_FALSE(this->_multilogger_instance.empty());

    this->_multilogger_instance.clear();

    EXPECT_TRUE(this->_multilogger_instance.empty());
}


TEST_F(MultiloggerFixture, write_to_valid_ostreams)
{
    this->_multilogger_instance.add(std::cout);
    this->_multilogger_instance.add(std::cerr);

    EXPECT_FALSE(this->_multilogger_instance.empty());
    EXPECT_NO_THROW(_multilogger_instance.write(nullptr, 0));
}


TEST_F(MultiloggerFixture, write_to_invalid_ostreams)
{
    int a;
    this->_multilogger_instance.add(a);

    EXPECT_THROW(_multilogger_instance.write(nullptr, 0), std::system_error);
}
