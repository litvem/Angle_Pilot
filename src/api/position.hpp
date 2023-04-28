/*
 * MIT License
 * 
 * Copyright (c) 2023 Robert Einer, Emma Litvin, Ossian Ålund, Bao Quan Lindgren, Khaled Adel Saleh Mohammed Al-Baadani
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef DIT639_2023_GROUP_13_POSITION_HPP
#define DIT639_2023_GROUP_13_POSITION_HPP

// Include the standard int types of C
#include <cstdint>

/*
 * The API used to transfer cone data between
 * microservices in this project.
 * 
 * The API includes:
 * - cone_t:       a struct for holding positional
 *                 information of cones
 * 
 * - timestamp_t:  a struct representing a UNIX
 *                 timestamp, consisting of the 
 *                 seconds passed since epoch and
 *                 the amount of mictoseconds since
 *                 the last full second
 * 
 * - data_t:       a struct representing the full
 *                 package that is written to the
 *                 shared memory. It contains four
 *                 (4) cones and two (2) timestamps
 * 
 * Author: Bao Quan Lindgren (2023)
 */
namespace pos_api {

    /**
     * A struct holding positional information
     * of cones. It is only intended to be
     * instantiated by the cone detector
     * microservice.
     * 
     * @param posX the x coordinate of a cone
     * @param posY the y coordinate of a cone
     */
    struct cone_t {
        const uint16_t posX;
        const uint16_t posY;
    };

    /**
     * A struct representing a timestamp.
     * The timestamp consists of the UNIX
     * timestanp (seconds) and the microseconds
     * elapsed since the UNIX timestamp (micros).
     * 
     * @param seconds a UNIX timestamp
     * @param micros the amount of microseconds
     * elapsed since the UNIX timestamp
     */
    struct timestamp_t {
        const uint32_t seconds;
        const uint32_t micros;
    };

    /**
     * The struct used for transferring data
     * regarding cone positions, between
     * microservices. It consists of multiple
     * cones and timestamps.
     * 
     * @param lClose the closest left hand cone
     * @param lFar the second closest left hand cone
     * @param rClose the closest right hand cone
     * @param rFar the second closest right hand cone
     * @param now the current UNIX timestamp
     * @param vidTimestamp the timestamp used in
     * the .rec file
     */
    struct data_t {
        const cone_t lClose;
        const cone_t lFar;
        const cone_t rClose;
        const cone_t rFar;
        const timestamp_t now;
        const timestamp_t vidTimestamp;
    };
} // !pos_api

#endif // !DIT639_2023_GROUP_13_POSITION_HPP