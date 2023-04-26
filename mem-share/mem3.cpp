// Include the single header file for cluon
#include "cluon-complete-v0.0.127.hpp"

// Include the header with structs to test the shared memory on
#include "spacetime.hpp"

#include <iostream>

// Include the standard int types of C
#include <cstdint>

int32_t main(int32_t argc, char **argv)
{
    // Parse the command line parameters as we require the user to specify some mandatory information on startup.
    auto cmdargs = cluon::getCommandlineArguments(argc, argv);

    // Exit the programme if all necessary command line arguments weren't passed
    if (!cmdargs.count(st::TIME))
    {
        std::cerr << argv[0] << " attaches to a shared memory area containing space coordinates." << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --" << st::TIME << "=<name of shared memory>" << std::endl;
        std::cerr << "         --" << st::TIME << ":   the namespace of the shared memory to read time data from" << std::endl;
        std::cerr << "Example: " << argv[0] << " --" << st::TIME << "=time" << std::endl;
        return 1;
    }

    // The name of the shared memory to create
    const std::string TIME_MEM = cmdargs[st::TIME];

    // The shared memory area for time data
    cluon::SharedMemory time{TIME_MEM};

    if (time.valid())
    {
        std::clog << argv[0] << ": Attached to shared memory " << time.name() << " (" << time.size() << " bytes)." << std::endl;
    }

    // Endless loop. exit by destroying the shared memory of with ^C
    while (time.valid())
    {
        // The incoming time data
        st::spacetime_t st;

        // Wait for something to happen
        time.wait();

        // Lock the memory and read from it
        time.lock();
        {
            st = *reinterpret_cast<st::spacetime_t *>(time.data());
        }
        // Unlock the shared memory once we're done
        time.unlock();

        // Print the data we received
        std::cout << "X coordinate: " << st.space.x << std::endl;
        std::cout << "Y coordinate: " << st.space.y << std::endl;
        std::cout << "@ <t:" << st.time.seconds << "." << st.time.micros << ":>" << std::endl;
        std::cout << "----------------------------------" << std::endl;
    }

    return 0;
}