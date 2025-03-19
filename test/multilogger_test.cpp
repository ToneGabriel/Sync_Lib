#include <iostream>
#include <fstream>

#include <multilogger.h>

void test_8()
{
    sync::multilogger lg;
    lg.add(std::cout);

    {
        std::ofstream ofs("test.txt");
        lg.add(ofs);
    }

    lg << "Test ";
}