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

// Author: Bao Quan Lindgren (2023)

// The header to implement
#include "position.hpp"

// Name of the shared memory region
#define MEM_NAME "position"

// The shared memory "singleton" that is used
cluon::SharedMemory *mem;
// Boolean to keep track of whether the shared memory
// has been instantiated 
bool created = false;
// Boolean to keep track of whether the shared memory
// is a producer or not
bool producer = false;

void pos_api::create()
{
    // Throw exception if the API already has been created
    if (created)
    {
        throw pos_api::APIException::CREATED;
    }

    // Throw exception if the API has been created elsewhere
    mem = new cluon::SharedMemory(MEM_NAME);
    if (mem->valid())
    {
        throw pos_api::APIException::CREATED;
    }
    // Free the test memory
    free(mem);

    // Instantiate a shared memory for the API
    mem = new cluon::SharedMemory(MEM_NAME, sizeof (data_t));
    created = true;
    producer = true;
    std::clog << "Created shared memory " << mem->name() << " (" << mem->size() << " bytes)." << std::endl;
}

void pos_api::attach()
{
    // Throw exception if the API already has been created
    if (created)
    {
        throw pos_api::APIException::CREATED;
    }

    // Attach to an existing API if it exists
    mem = new cluon::SharedMemory(MEM_NAME);

    // Throw exception if unable to attach
    if (!mem->valid())
    {
        throw pos_api::APIException::EMPTY;
    }

    created = true;
    std::clog << "Attached to shared memory " << mem->name() << " (" << mem->size() << " bytes)." << std::endl;
}

void pos_api::clear()
{
    // Clear only if created
    if (created)
    {
        // Destruct if the application is the producer
        if (producer) {
            mem->~SharedMemory();
        }
        // Free the dynamically allocated memory
        free((void *) mem);
        created = false;
        producer = false;
    }
}

void pos_api::put(pos_api::data_t data)
{
    // Throw exception if there is no API to interact with
    if (!created)
    {
        throw pos_api::APIException::EMPTY;
    }

    // Throw exception if not producer
    // Only the producer is allowed to put data
    if (!producer)
    {
        throw pos_api::APIException::IS_CONSUMER;
    }

    // Get exclusive access to the shared memory
    mem->lock();
    {
        // Copy the data that was provided
        // into the shared memory
        pos_api::data_t *d = (pos_api::data_t *) mem->data();
        // memcpy because &pos_api::data_t::operator=
        // is a deleted function(???)
        memcpy(d, &data, sizeof (pos_api::data_t));
    }
    // Unlock the memory and notify all consumers
    mem->unlock();
    mem->notifyAll();
}

pos_api::data_t pos_api::get()
{
    // Throw exception if there is no API to interact with
    if (!created)
    {
        throw pos_api::APIException::EMPTY;
    }

    // The data read from the shared memory
    pos_api::data_t d{};

    // It's initialised like that because the
    // struct doesn't have a default constructor(???)

    // Wait for an update
    mem->wait();
    // Get exclusive access to the shared memory
    mem->lock();
    {
        // Copy the data from the shared memory
        // memcpy because &pos_api::data_t::operator=
        // is a deleted function(???)
        memcpy(&d, mem->data(), sizeof (pos_api::data_t));
    }
    // Unlock the shared memory
    mem->unlock();
    return d;
}

bool pos_api::isEqual(const pos_api::cone_t c1, const pos_api::cone_t c2)
{
    return c1.posX == c2.posX && c1.posY == c2.posY;
}
