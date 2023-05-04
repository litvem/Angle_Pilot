// Include the header with structs to test the shared memory on
#include "../api/position.hpp"

// We will use chrono and thread to sleep for a while.
#include <chrono>
#include <thread>

#include <iostream>

// Include the standard int types of C
#include <cstdint>

/**
 * Struct representing a linear mathematical functions.
 * (y = coefficient * x + constant)
 * 
 * @param coefficient the slope of the linear function
 * @param constant the constant of the linear function
 */
struct line_t {
    float_t coefficient;
    float_t constant;
};

/**
 * Struct representing a point in a graph with x and y
 * coordinates
 * 
 * @param x the x coordinate of the point
 * @param y the y coordinate of the point
 */
struct point_t {
    float_t x;
    float_t y;
};

void handleExit(int sig);

/**
 * Applies the two-point-equation on two cone
 * positions and calculates a mathematical
 * linear function
 * 
 * @param close the closest cone of one side
 * @param far the second closest cone on the
 * same side
 */
line_t getLineFromCones(pos_api::cone_t close, pos_api::cone_t far);

point_t getIntersect(line_t f, line_t g);

float_t getAngle(point_t origin, point_t p);

float_t calculateSteering(pos_api::data_t data);

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

        std::cout << "@ <t:" << d.now.seconds << "." << d.now.micros << ">" << std::endl;
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

line_t getLineFromCones(pos_api::cone_t close, pos_api::cone_t far)
{
    // dy/dx
    float_t coeff = (far.posY - close.posY) / (far.posX - close.posX);
    // y1 - ax1
    float_t constant = far.posY - (coeff * far.posX);

    return {coeff, constant};
}
