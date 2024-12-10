#include <iostream>

#include "test.h"

void foo()
{
    std::cout << "foo running...";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "foo done.";
}

void test_pool_1()
{
    thread_pool pool;

    pool.do_job(foo, priority::high);
    pool.do_job(foo, priority::highest);
    pool.do_job(foo, priority::lowest);
    pool.do_job(foo, priority::normal);
    pool.do_job(foo, priority::low);
    pool.do_job(foo, priority::highest);
    pool.do_job(foo, priority::normal);
    pool.do_job(foo, priority::lowest);
}