// Include the single header file for cluon
#include "cluon-complete-v0.0.127.hpp"

// Include the header with structs to test the shared memory on
#include "spacetime.hpp"

// We will use chrono and thread to sleep for a while.
#include <chrono>
#include <thread>

#include <iostream>

// Include the standard int types of C
#include <cstdint>

// Main entry point
int32_t main(int32_t argc, char **argv)
{
    // Parse the command line parameters as we require the user to specify some mandatory information on startup.
    auto cmdargs = cluon::getCommandlineArguments(argc, argv);

    // Exit the programme if all necessary command line arguments weren't passed
    if (!cmdargs.count(st::SPACE))
    {
        std::cerr << argv[0] << " attaches to a shared memory area containing space coordinates." << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --" << st::SPACE << "=<name of shared memory>" << std::endl;
        std::cerr << "         --" << st::SPACE << ":   the namespace of the shared memory to store space data in" << std::endl;
        std::cerr << "Example: " << argv[0] << " --" << st::SPACE << "=space" << std::endl;
        return 1;
    }

    // The name of the shared memory to create
    const std::string SPACE_MEM = cmdargs[st::SPACE];

    // The shared memory area.
    cluon::SharedMemory space{st::SPACE, sizeof (st::space_t)};

    if (space.valid())
    {
        std::clog << argv[0] << ": Created shared memory " << space.name() << " (" << space.size() << " bytes)." << std::endl;
    }

    // Endless loop, exit with ^C
    while (space.valid()) {
        // Lock the shared memory area and write some data.
        space.lock();
        {
            st::space_t *data = reinterpret_cast<st::space_t *>(space.data());
            st::space_t newSpace{rand() - RAND_MAX / 2, rand() - RAND_MAX / 2};
            *data           = newSpace;
        }
        // Unlock the shared memory once we're done
        space.unlock();
        
        // Finally, notify any other processes that are sleeping.
        space.notifyAll();

        // Let's wait for 1s before producing the next data.
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(500ms);
    }

    return 0;
}