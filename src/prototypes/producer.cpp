/*
 * Copyright (C) 2023  Robert Einer, Emma Litvin, Ossian Ålund, Bao Quan Lindgren, Khaled Adel Saleh Mohammed Al-Baadani
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
    // Attach an exit handler to the ^C event
    signal(SIGINT, handleExit);

    try
    {
        pos_api::create();
    }
    catch (const pos_api::APIException& e)
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
