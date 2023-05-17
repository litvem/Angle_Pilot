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
        pos_api::attach();
    }
    catch (const pos_api::APIException& e)
    {
        switch (e)
        {
            case pos_api::APIException::CREATED:
                std::cerr << "Shared memory already exists" << std::endl;
                break;
            case pos_api::APIException::EMPTY:
                std::cerr << "No shared memory to attach to" << std::endl;
                break;
            default:
                std::cerr << "Oops! Something went wrong" << std::endl;
        }

        handleExit(0);
    }
    
    // Endless loop, exit with ^C
    while (true)
    {
        pos_api::data_t d = pos_api::get();

        std::cout << "TIMESTAMP UNIX: " << d.now.seconds << std::endl;
        std::cout << "Blue Close X:" << d.bClose.posX << ", Blue Close Y:" << d.bClose.posY << std::endl;
        std::cout << "Blue Far X:" << d.bFar.posX << ", Blue Far Y:" << d.bFar.posY << std::endl;
        std::cout << "Yellow Close X:" << d.yClose.posX << ", Yellow Close Y:" << d.yClose.posY << std::endl;
        std::cout << "Yellow Far X:" << d.yFar.posX << ", Yellow Far Y:" << d.yFar.posY << std::endl;
        std::cout << "TIMESTAMP VIDEO-FRAME: " << d.vidTimestamp.seconds << std::endl;


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
