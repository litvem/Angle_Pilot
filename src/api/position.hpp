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

#ifndef DIT639_2023_GROUP_13_POSITION_HPP
#define DIT639_2023_GROUP_13_POSITION_HPP

// Include the standard int types of C
#include <cstdint>

// Include cluon to create a wrapper for the shared memory
#include "../cluon-complete-v0.0.127.hpp"

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
 * - APIException: a collection of enumerations that
 *                 represent exceptions related to
 *                 the API
 * 
 * - create:       a function that initiates the API
 * 
 * - attach:       a function that attaches to an
 *                 existing API
 * 
 * - clear:        a function that cleans up the API
 * 
 * - put:          puts data into the shared memory
 * 
 * - get:          reads data from the shared memory
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

    // Constant used when no cone position is available
    const cone_t NO_CONE_POS{
        0,
        0
    };

    /**
     * A struct representing a timestamp.
     * The timestamp consists of the UNIX
     * timestamp (seconds) and the microseconds
     * elapsed since the UNIX timestamp (micros).
     * 
     * @param micros the UNIX timestamp in microseconds
     */
    struct timestamp_t {
        const int64_t micros;
    };

    /**
     * The struct used for transferring data
     * regarding cone positions, between
     * microservices. It consists of multiple
     * cones and timestamps.
     * 
     * @param bClose the closest blue cone
     * @param bFar the second closest blue cone
     * @param yClose the closest yellow cone
     * @param yFar the second closest yellow cone
     * @param now the current UNIX timestamp
     * @param vidTimestamp the timestamp used in
     * @param gsr the original ground steering request
     * the .rec file
     */
    struct data_t {
        const cone_t bClose;
        const cone_t bFar;
        const cone_t yClose;
        const cone_t yFar;
        const timestamp_t now;
        const timestamp_t vidTimestamp;
        const _Float32 gsr;
    };

    /*
     * Exception enumerations tied to the API
     */
    enum APIException {
        /*
         * An exception that is thrown when trying to
         * create/attach when the API has already been
         * instantiated
         */
        CREATED,

        /*
         * An exception that is thrown when a consumer
         * tries to put data into the shared memory
         */
        IS_CONSUMER,

        /*
         * An exception that is thrown when trying to
         * put/get when no API has been instantiated
         */
        EMPTY
    };

    /**
     * Instantiates a shared memory region to act as
     * an API for communication regarding cone position
     * 
     * Only 1 producer can be created at any given time
     * 
     * @throws APIException::CREATED if an API has already
     * been instansiated
     */
    void create();

    /**
     * Attaches to a shared memory region for communication
     * regarding cone position
     * 
     * @throws APIException::CREATED if an API has already
     * been instansiated
     * @throws APIException::EMPTY if the is no API to
     * attach to
     */
    void attach();

    /**
     * Cleans up after the API by destroying the shared
     * memory and freeing the memory used for the shared
     * memory
     */
    void clear();

    /**
     * Writes data to the shared memory for consumers to read
     * 
     * @param data the data to write into the shared memory
     */
    void put(data_t data);

    /**
     * Reads data from the shared memory that a producer has
     * written
     * 
     * @returns the cone data from a producer
     */
    data_t get();

    /**
     * Checks if two cones are equal in terms of position
     * 
     * @param c1 the first cone to compare
     * @param c2 the second cone to compare
     * @returns a bool for whether the two cones are equal
     * or not
     */
    bool isEqual(const cone_t c1, const cone_t c2);
} // !namespace pos_api

#endif // !DIT639_2023_GROUP_13_POSITION_HPP