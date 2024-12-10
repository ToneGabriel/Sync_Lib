#include <iostream>

#include "test.h"

void foo()
{
    std::cout << "foo running...";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "foo done.";
}

void test_1()
{
    thread_pool pool(2);

    pool.do_job(foo, job_priority::high);
    pool.do_job(foo, job_priority::highest);
    pool.do_job(foo, job_priority::lowest);
    pool.do_job(foo, job_priority::normal);
    pool.do_job(foo, job_priority::low);
    pool.do_job(foo, job_priority::highest);
    pool.do_job(foo, job_priority::normal);
    pool.do_job(foo, job_priority::lowest);
}