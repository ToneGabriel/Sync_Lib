#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <iostream>

#include "sync/multilogger.hpp"


class MultiloggerFixture : public ::testing::Test
{
protected:
    sync::multilogger _multilogger_instance;
};  // END MultiloggerFixture


TEST_F(MultiloggerFixture, test)
{
    // TODO
}
