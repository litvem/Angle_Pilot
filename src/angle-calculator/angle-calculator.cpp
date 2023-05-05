// Include the header with structs to test the shared memory on
#include "../api/position.hpp"

#include <iostream>

// Include the standard int types of C
#include <cstdint>

// Used to get the limits of numeric types
#include <limits>

// Declares a set of function to compute common mathematical operations. 
#include <math.h>

// The maximum steering value for a left turn
#define MAX_LEFT_STEERING_VAL 0.290888f

// The maximum steering value for a right turn
#define MAX_RIGHT_STEERING_VAL -MAX_LEFT_STEERING_VAL

/**
 * Struct representing a linear mathematical functions.
 * (y = coefficient * x + constant)
 * 
 * @param coefficient the slope of the linear function.
 * float max if the slope inclination is infinite
 * @param constant the constant of the linear function.
 * x value if the slope inclination is infinite
 */
struct line_t {
    _Float32 coefficient;
    _Float32 constant;
};

/**
 * Struct representing a point in a graph with x and y
 * coordinates
 * 
 * @param x the x coordinate of the point
 * @param y the y coordinate of the point
 */
struct point_t {
    _Float32 x;
    _Float32 y;
};

// The coefficient for a slope with infinite inclination
const _Float32 INF_SLOPE = std::numeric_limits<_Float32>::max();

/**
 * Exit handler that cleans up after the process
 * if possible
 * 
 * @param sig the exit signal
 */
void handleExit(int sig);

/**
 * Applies the two-point-equation on two cone
 * positions and calculates a mathematical
 * linear function
 * 
 * @param close the closest cone of one side
 * @param far the second closest cone on the
 * same side
 * @returns the coefficient and constant for
 * a linear mathematical function unless it
 * has an infinite slope inclination, in which
 * case it returns float max and the x value
 * of the line
 */
line_t getLineFromCones(pos_api::cone_t close, pos_api::cone_t far);

/**
 * Calculates the intersection between 2 lines,
 * if there is one. Otherwise returns the origin
 * in terms of the car heading.
 * 
 * @param f one of the functions to check the
 * intersect of
 * @param g the other function to check the
 * intersect of
 * @returns the point of intersect if there is one,
 * otherwise the origin in terms of the car heading
 */
point_t getIntersect(line_t f, line_t g);

_Float32 getAngle(point_t origin, point_t p);

_Float32 calculateSteering(pos_api::data_t data);

// Main entry point
int32_t main(int32_t argc, char **argv)
{
    // Attach an exit handler to the ^C event
    signal(SIGINT, handleExit);
    // Attach the exit handler to the process termination event
    signal(SIGTERM, handleExit);
    // Attach the exit handler to the ^/ event
    signal(SIGQUIT, handleExit);
    // Attach the exit handler to the hangup signal (kill terminal)
    signal(SIGHUP, handleExit);

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

        handleExit(SIGTERM);
        return 1;
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
}

line_t getLineFromCones(pos_api::cone_t close, pos_api::cone_t far)
{
    // Catch division by 0 (infinite slope)
    if (far.posX == close.posX)
    {
        // Includes the x value in place of line_t.constant
        return {INF_SLOPE, (_Float32) close.posX};
    }

    // dy/dx
    _Float32 coeff = (far.posY - close.posY) / (far.posX - close.posX);
    // y1 - ax1
    _Float32 constant = far.posY - (coeff * far.posX);

    return {coeff, constant};
}

point_t getIntersect(line_t f, line_t g)
{
    // x coordinate of the intersect
    _Float32 x;
    // y coordinate of the intersect
    _Float32 y;

    if (f.coefficient == INF_SLOPE && g.coefficient == INF_SLOPE)
    {
        // TODO: return origin
    }
    else if (f.coefficient == INF_SLOPE)
    {
        x = f.constant;
        y = g.coefficient * x;
    }
    else if (g.coefficient == INF_SLOPE)
    {
        x = g.coefficient;
        y = f.coefficient * x;
    }
    else
    {
        // f(x) = g(x)
        // mf * x + bf = mg * x + bg
        // x * (mf - mg) = bg - bf
        // x = (bg - bf) / (mf - mg)
        x = (g.constant - f.constant) / (f.coefficient - g.coefficient);

        y = f.coefficient * x;
    }

    return {x, y};
}

_Float32 getAngle(point_t origin, point_t p) {
    // The angle in degrees from the line between the line
    // between origin and p, and the x-axis
    // Positive values -> counterclockwise rotation
    _Float32 angle = atan((origin.y - p.y) / (origin.x - p.x)) * (180 / M_PI);

    // Shift 0 degrees by 90 degrees clockwise and
    // Subtract the angle by 180, yielding angles between
    // -180 and 180 degrees.
    // 0 degrees will point straight up while +/-180
    // degrees will point straight down
    angle = fmod(angle + 90.0f, 360.0f) - 180.0f;
    return angle;
}
