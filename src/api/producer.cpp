// Include the header with structs to test the shared memory on
#include "position.hpp"

// We will use chrono and thread to sleep for a while.
#include <chrono>
#include <thread>

#include <iostream>

// Include the standard int types of C
#include <cstdint>

void handleExit(int sig);

// Main entry point
int32_t main(int32_t argc, char **argv)
{
    signal(SIGINT, handleExit);

    try
    {
        pos_api::create();
    }
    catch(const pos_api::APIException& e)
    {
        switch (e)
        {
            case pos_api::APIException::CREATED:
                std::cerr << "Shared memory already exists" << std::endl;
                break;
            default:
                std::cerr << "Oops! Something went wrong" << std::endl;
        }

        handleExit(0);
    }
    
    // Endless loop, exit with ^C
    while (true)
    {
        cluon::data::TimeStamp t = cluon::time::now();

        pos_api::data_t d{
            {rand(), rand()},
            {rand(), rand()},
            {rand(), rand()},
            {rand(), rand()},
            {t.seconds(), t.microseconds()},
            {t.seconds(), t.microseconds()}
        };

        pos_api::put(d);

        // Let's wait for 1s before producing the next data.
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(500ms);
    }

    return 0;
}

void handleExit(int sig)
{
    std::clog << std::endl << "Cleaning up..." << std::endl;
    pos_api::clear();
    std::clog << "Exiting programme..." << std::endl;
    exit(0);
}
