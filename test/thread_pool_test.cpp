#include <iostream>
#include <fstream>

#include <thread_pool.h>


// Helpers
// ===========================================================
void highest_prio()
{
    std::cout << "highest_prio running...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "highest_prio done!\n";
}

void high_prio()
{
    std::cout << "high_prio running...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "high_prio done!\n";
}

void normal_prio()
{
    std::cout << "normal_prio running...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "normal_prio done!\n";
}

void low_prio()
{
    std::cout << "low_prio running...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "low_prio done!\n";
}

void lowest_prio()
{
    std::cout << "lowest_prio running...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "lowest_prio done!\n";
}

void foo_except_nok()
{
    std::cout << "foo_except_nok running...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));
    throw -3;
}

void foo_except_ok()
{
    std::cout << "foo_except_ok running...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));
    throw std::out_of_range("Out of range exception");
}

int foo_return()
{
    std::cout << "foo_return running...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 42;
}

// Tests
// ===========================================================
void test_1()
{
    sync::thread_pool pool(1);

    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::high, high_prio);
    pool.do_job(sync::priority::highest, highest_prio);
}

void test_2()
{
    sync::thread_pool pool;

    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::high, high_prio);
    pool.do_job(sync::priority::highest, highest_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::low, low_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::high, high_prio);
    pool.do_job(sync::priority::lowest, lowest_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::lowest, lowest_prio);
    pool.do_job(sync::priority::high, high_prio);
}

void test_3()
{
    sync::thread_pool pool(2);
    std::cout << "Pool has " << pool.thread_count() << " threads\n";

    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::high, high_prio);
    pool.do_job(sync::priority::highest, highest_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::low, low_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::high, high_prio);
    pool.do_job(sync::priority::lowest, lowest_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::lowest, lowest_prio);
    pool.do_job(sync::priority::high, high_prio);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    pool.restart(5);
    std::cout << "Pool has " << pool.thread_count() << " threads\n";
}

void test_4()
{
    sync::thread_pool pool(2);

    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::high, high_prio);
    pool.do_job(sync::priority::highest, highest_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::low, low_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    
    pool.pause();
    std::cout << "Pool paused\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));

    pool.do_job(sync::priority::high, high_prio);
    pool.do_job(sync::priority::lowest, lowest_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::lowest, lowest_prio);
    pool.do_job(sync::priority::high, high_prio);

    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "Pool resumed\n";
    pool.resume();

    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "Pool destroyed\n";
}

void test_5()
{
    sync::thread_pool pool(2);

    pool.do_job(sync::priority::normal, foo_except_nok);
    pool.do_job(sync::priority::high, foo_except_ok);
    pool.do_job(sync::priority::highest, foo_except_nok);
    pool.do_job(sync::priority::normal, foo_except_ok);
    pool.do_job(sync::priority::low, foo_except_nok);
    pool.do_job(sync::priority::normal, foo_except_ok);
    pool.do_job(sync::priority::normal, foo_except_nok);
    pool.do_job(sync::priority::high, foo_except_nok);
    pool.do_job(sync::priority::lowest, foo_except_ok);
    pool.do_job(sync::priority::normal, foo_except_nok);
    pool.do_job(sync::priority::lowest, foo_except_nok);
    pool.do_job(sync::priority::high, foo_except_ok);

    std::this_thread::sleep_for(std::chrono::seconds(4));
    pool.force_join();

    std::cout << "Pool joined with finished jobs " << pool.jobs_done() << '\n';

    pool.restart(5);
    std::cout << "Pool restarted\n";

    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::high, high_prio);
    pool.do_job(sync::priority::highest, highest_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::low, low_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::high, high_prio);
    pool.do_job(sync::priority::lowest, lowest_prio);
    pool.do_job(sync::priority::normal, normal_prio);
    pool.do_job(sync::priority::lowest, lowest_prio);
    pool.do_job(sync::priority::high, high_prio);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    pool.force_join();

    std::cout << "Pool joined with finished jobs " << pool.jobs_done() << '\n';

    std::cout << "Pool destroyed\n";
}

void test_6()
{
    sync::thread_pool pool(2);
    
    pool.store_job(sync::priority::normal, normal_prio);
    pool.store_job(sync::priority::high, high_prio);
    pool.store_job(sync::priority::highest, highest_prio);
    pool.store_job(sync::priority::normal, normal_prio);
    pool.store_job(sync::priority::low, low_prio);
    pool.store_job(sync::priority::normal, normal_prio);
    pool.store_job(sync::priority::normal, normal_prio);
    pool.store_job(sync::priority::high, high_prio);
    pool.store_job(sync::priority::lowest, lowest_prio);
    pool.store_job(sync::priority::normal, normal_prio);
    pool.store_job(sync::priority::lowest, lowest_prio);
    pool.store_job(sync::priority::high, high_prio);

    std::cout << "Pool stored some jobs " << pool.stored_jobs() << '\n';

    pool.flush_job_storage();

    std::cout << "Pool flushed stored jobs " << '\n';
}

void test_7()
{
    sync::thread_pool pool(2);
    
    auto ret1 = pool.do_job(sync::priority::high, foo_return);
    auto ret2 = pool.do_job(sync::priority::normal, foo_except_ok);

    try
    {
        int result = ret1.get();
        std::cout << "Result = " << result << '\n';

        ret2.get();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception message = " << e.what() << '\n';
    }
}
