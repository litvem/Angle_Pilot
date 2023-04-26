#ifndef SPACETIME_HPP
#define SPACETIME_HPP

#include <cstdint>
#include <string>

/*
 * Namespace containing structs for space and time.
 * These structs are used to test cluon's shared
 * memory feature.
 */
namespace st {

    // Memory space names

    const std::string SPACE = "space";
    const std::string TIME = "time";

    struct space_t {
        int32_t x;
        int32_t y;
    };

    struct time_t {
        int32_t seconds;
        int32_t micros;
    };

    struct spacetime_t {
        space_t space;
        time_t time;
    };
} // !namespace st

#endif // !SPACETIME_HPP