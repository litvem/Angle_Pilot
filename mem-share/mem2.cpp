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
    if (!cmdargs.count(st::SPACE) ||
        !cmdargs.count(st::TIME))
    {
        std::cerr << argv[0] << " attaches to a shared memory area containing space coordinates." << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --" << st::SPACE << "=<name of shared memory> --" << st::TIME << "=<name of shared memory>" << std::endl;
        std::cerr << "         --" << st::SPACE << ":   the namespace of the shared memory to read space data from" << std::endl;
        std::cerr << "         --" << st::TIME << ":   the namespace of the shared memory to store time data in" << std::endl;
        std::cerr << "Example: " << argv[0] << " --" << st::SPACE << "=space --" << st::TIME << "=time" << std::endl;
        return 1;
    }

    // The name of the shared memory to create
    const std::string SPACE_MEM = cmdargs[st::SPACE];
    const std::string TIME_MEM = cmdargs[st::TIME];

    // The shared memory area for space data
    cluon::SharedMemory space{SPACE_MEM};
    // The shared memory area for time data
    cluon::SharedMemory time{TIME_MEM, sizeof (st::spacetime_t)};

    if (space.valid() && time.valid())
    {
        std::clog << argv[0] << ": Attached to shared memory " << space.name() << " (" << space.size() << " bytes)." << std::endl;
        std::clog << argv[0] << ": Created shared memory " << time.name() << " (" << time.size() << " bytes)." << std::endl;
    }

    // Endless loop. exit by destroying the shared memory of with ^C
    while (space.valid() && time.valid())
    {
        // The incoming space data
        st::space_t s;
        // The outgoing time data
        st::time_t t;

        // Wait for something to happen
        space.wait();

        // Lock the memory and read from it
        space.lock();
        {
            s = *reinterpret_cast<st::space_t *>(space.data());
        }
        // Unlock the shared memory once we're done
        space.unlock();

        // Print the data we received
        std::cout << "X coordinate: " << s.x << std::endl;
        std::cout << "Y coordinate: " << s.y << std::endl;
        std::cout << "----------------------------------" << std::endl;

        // Lock the second shared memory and write to it
        time.lock();
        {
            cluon::data::TimeStamp now = cluon::time::now();
            t = {now.seconds(), now.microseconds()};
            st::spacetime_t *st = reinterpret_cast<st::spacetime_t *>(time.data());
            *st = {s, t};
        }
        // Unlock the shared memory once we're done
        time.unlock();
        
        // Finally, notify any other processes that are sleeping.
        time.notifyAll();
    }

    return 0;
}