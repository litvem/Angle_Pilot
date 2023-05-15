// Include the header with structs to test the shared memory on
#include "../api/position.hpp"

#include <iostream>

// Include the standard int types of C
#include <cstdint>

// Declares a set of function to compute common mathematical operations. 
#include <math.h>

// The testing functions used for the angle calculator
#include "angle-validator.hpp"

// The maximum absolute steering value
#define MAX_ABS_STEERING_VAL 0.290888f

// Output value of no steering angle
#define NO_ANGLE 0.0f

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

// The width of the frame being processed
uint16_t width;

// The height of the frame being processed
uint16_t height;

// The threshold to pass for calculateSteering to output something non-zero
_Float32 zeroThreshold;
// The threshold to pass for calculateSteering to output MAX_ABS_STEERING_ANGLE
_Float32 maxThreshold;

// The fraction that the origin's y coordinate
// should be offset by
_Float32 originYOffset;

// The fraction that each default lines' top point should be placed
// from the sides of the frame. This MUST be a whole number!!
_Float32 defaultLineOffset;

// Degrees to shift when calculating the output,
// with positive going counterclockwise
_Float32 angleBias;

// The origin of the car heading
point_t origin;

// The default edge for the right side
// when no line can be drawn
line_t rightDefault;
// The default edge for the left side
// when no line can be drawn
line_t leftDefault;

// The coefficient for a slope with infinite inclination
const _Float32 INF_SLOPE = 0.0f;

// The slope for NO_CONE_POS
const line_t NO_CONE_LINE = {pos_api::NO_CONE_POS.posX, pos_api::NO_CONE_POS.posY};

bool test;
bool verbose;

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
line_t getLineFromCones(const pos_api::cone_t close, const pos_api::cone_t far);

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
point_t getIntersect(const line_t f, const line_t g);

/**
 * Gets the angle between a vertical line and the line
 * between two points.
 * The output value ranges from -180 deg to +180 deg
 * with negative values going clockwise
 * 
 * @param origin the starting point of the line
 * @param p the endpoint of the line
 * @return an angle between -180 and +180
 */
_Float32 getAngle(const point_t origin, const point_t p);

/**
 * Checks if two lines are equal.
 * Lines are equal if both of their coefficients and
 * constants are equal
 * 
 * @param f a line
 * @param g another line
 * @return a bool representing if the lines are equal
 * or not
 */
bool isEqual(const line_t f, const line_t g);

/**
 * Fills in edges if they don't exist by checking the
 * existing edge. If no edge exists, it assumes the default
 * lines
 * 
 * @param f a pointer to a line
 * @param g a pointer to another line
 */
void determineEdges(line_t *const f, line_t *const g);

/**
 * Calculates the steering angle value based on the
 * data passed
 * 
 * @param data the cone data to perform calculations on
 * @return a steering angle value between -MAX_ABS_STEERING_VAL
 * and MAX_ABS_STEERING_VAL
 */
_Float32 calculateSteering(pos_api::data_t data);

// Main entry point
int32_t main(int32_t argc, char **argv)
{
    auto cmdargs = cluon::getCommandlineArguments(argc, argv);
    if (!cmdargs.count("width") ||
        !cmdargs.count("height") ||
        !cmdargs.count("z") || // zeroThreshold
        !cmdargs.count("m") || // maxThreshold
        !cmdargs.count("y") || // originYOffset
        !cmdargs.count("l") || // defaultLineOffset
        !cmdargs.count("b"))   // angleBias
    {
        std::cerr << argv[0] << " calculates a steering value based on cone data received through shared memory." << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --width=<width of frame> --height=<height of frame>"
                  << "--z=<threshold for non-zero values> --m=<threshold for max value>"
                  << "--y=<origin y value offset> --l=<endpoint offset for default lines>"
                  << "--b=<angle calculation offset> [--test] [--verbose]" << std::endl;
        std::cerr << "         --width:  width of the frame (int)" << std::endl;
        std::cerr << "         --height: height of the frame (int)" << std::endl;
        std::cerr << "         --z: angle threshold for the algorithm to output non-zero values (float)" << std::endl;
        std::cerr << "         --m: angle threshold for the algorithm to output the maximum value (float)" << std::endl;
        std::cerr << "         --y: fraction to offset the origin's y value (float)" << std::endl;
        std::cerr << "         --l: number of partitions to create from the frame to offset the default lines' ending point to (int)" << std::endl;
        std::cerr << "         --b: angle to offset the angle calculation by (float)" << std::endl;
        std::cerr << "         --test: whether or not to perform an accuracy test and print the results unot exiting the programme" << std::endl;
        std::cerr << "         --verbose: whether or not to perform an accuracy test and for each frame" << std::endl;
        std::cerr << "Example: " << argv[0] << " --width=640 --height=480 --z=10 --m=70 --y=0.2 --l=3 --b=0" << std::endl;
        return 1;
    }

    {
        int32_t tmpWidth = stoi(cmdargs["width"]);
        int32_t tmpHeight = stoi(cmdargs["height"]);

        if (tmpWidth > UINT16_MAX || tmpHeight > UINT16_MAX ||
            tmpWidth < 0 || tmpHeight < 0)
        {
            std::cerr << "Width or height out of bounds" << std::endl;
            return 1;
        }
        width = tmpWidth;
        height = tmpHeight;
        zeroThreshold = std::stof(cmdargs["z"]);
        maxThreshold = std::stof(cmdargs["m"]);
        originYOffset = std::stof(cmdargs["y"]);
        defaultLineOffset = std::stof(cmdargs["l"]);
        angleBias = std::stof(cmdargs["b"]);
    }

    test = cmdargs.count("test");
    verbose = cmdargs.count("verbose");
    origin = {(_Float32) width / 2.0f, height * originYOffset};

    // Get the default right edge between the top center
    // and bottom right corner
    rightDefault = getLineFromCones(
        {(uint16_t) (width - 1), 0},
        {(uint16_t) ((width / defaultLineOffset) * (defaultLineOffset - 1.0f)), height}
    );

    // Get the default right edge between the top center
    // and bottom left corner
    leftDefault = getLineFromCones(
        {1, 0},
        {(uint16_t) (width / defaultLineOffset), height}
    );


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

    // Used to check the timestamp of the last iteration.
    // The wait function of SharedMemory seems to be inconsistent
    int64_t lastTs = INT64_MIN;
    // Endless loop, exit with ^C
    while (true)
    {
        pos_api::data_t d = pos_api::get();

        // Skip if the timestamp is a duplicate
        if (d.vidTimestamp.micros == lastTs) continue;

        lastTs = d.vidTimestamp.micros;

        _Float32 outputVal = calculateSteering(d);
        _Float32 gsrVal = d.gsr;

        if (test || verbose)
        {
            ang_vld::registerSteering(gsrVal, outputVal);
        }

        // std::clog << "bClose @ (" << d.bClose.posX << ", " << d.bClose.posY << ")" << std::endl;
        // std::clog << "bFar @ (" << d.bFar.posX << ", " << d.bFar.posY << ")" << std::endl;
        // std::clog << "yClose @ (" << d.yClose.posX << ", " << d.yClose.posY << ")" << std::endl;
        // std::clog << "yFar @ (" << d.yFar.posX << ", " << d.yFar.posY << ")" << std::endl;

        std::cout << "group_13;" << d.vidTimestamp.micros << ";" << outputVal << std::endl;

        if (verbose)
        {
            ang_vld::printResult();
        }
    }

    return 0;
}


/*
int32_t main(int32_t argc, char **argv)
{
    origin = {5, 0};
    while (true)
    {
        int16_t y1x, y1y, y2x, y2y, b1x, b1y, b2x, b2y;
        std::string input;
        std::cout << "yellow cone 1 x coordinate: ";
        std::cin >> input;
        y1x = stoi(input);
        std::cout << "yellow cone 1 y coordinate: ";
        std::cin >> input;
        y1y = stoi(input);
        std::cout << "yellow cone 2 x coordinate: ";
        std::cin >> input;
        y2x = stoi(input);
        std::cout << "yellow cone 2 y coordinate: ";
        std::cin >> input;
        y2y = stoi(input);
        std::cout << "blue cone 1 x coordinate: ";
        std::cin >> input;
        b1x = stoi(input);
        std::cout << "blue cone 1 y coordinate: ";
        std::cin >> input;
        b1y = stoi(input);
        std::cout << "blue cone 2 x coordinate: ";
        std::cin >> input;
        b2x = stoi(input);
        std::cout << "blue cone 2 y coordinate: ";
        std::cin >> input;
        b2y = stoi(input);

        _Float32 val = calculateSteering({
            {b1x, b1y},
            {b2x, b2y},
            {y1x, y1y},
            {y2x, y2y},
            {0, 0},
            {0, 0}
        });

        std::cout << val << std::endl;
    }
    while (true)
    {
        std::string in;
        std::cin >> in;
        std::cout << "arctan(" << in << ") = " << atan(stoi(in)) * (180 / M_PI) << " deg" << std::endl;
    }
}
 */

void handleExit(int sig)
{
    std::cout << std::endl;
    if (test && !verbose)
    {
        ang_vld::printResult();
    }
    std::cout << "Cleaning up..." << std::endl;
    pos_api::clear();
    std::cout << "Exiting programme..." << std::endl;
}

line_t getLineFromCones(const pos_api::cone_t close, const pos_api::cone_t far)
{
    // Check if there is a cone at all
    if (pos_api::isEqual(close, pos_api::NO_CONE_POS))
    {
        return NO_CONE_LINE;
    }
    // Catch division by 0 (infinite slope)
    if (far.posX == close.posX)
    {
        // Includes the x value in place of line_t.constant
        return {INF_SLOPE, (_Float32) close.posX};
    }

    // dy/dx
    _Float32 coeff = (_Float32) (far.posY - close.posY) / (_Float32) (far.posX - close.posX);
    // y1 - ax1
    _Float32 constant = far.posY - (coeff * far.posX);

    // std::cout << "y = " << coeff << "x + " << constant << std::endl;

    return {coeff, constant};
}

point_t getIntersect(const line_t f, const line_t g)
{
    // x coordinate of the intersect
    _Float32 x;
    // y coordinate of the intersect
    _Float32 y;

    // Return the point at the top of the frame
    // right in between the lines if they are vertical
    if (f.coefficient == INF_SLOPE && g.coefficient == INF_SLOPE)
    {
        return {f.constant - g.constant, 0.0f};
    }
    // If one of the lines is vertical,
    // take that into consideration
    else if (f.coefficient == INF_SLOPE)
    {
        // f.constant is the x value where the line is at
        x = f.constant;
        y = g.coefficient * x;
    }
    else if (g.coefficient == INF_SLOPE)
    {
        // g.constant is the x value where the line is at
        x = g.constant;
        y = f.coefficient * x;
    }
    // Otherwise, treat them as regular functions
    else
    {
        // f(x) = g(x)
        // mf * x + bf = mg * x + bg
        // x * (mf - mg) = bg - bf
        // x = (bg - bf) / (mf - mg)
        x = (g.constant - f.constant) / (f.coefficient - g.coefficient);

        y = f.coefficient * x + f.constant;
    }

    return {x, y};
}

_Float32 getAngle(const point_t origin, const point_t p) {
    // The angle in degrees from the line between the line
    // between origin and p, and the x-axis
    // Positive values -> counterclockwise rotation
    _Float32 angle = atan((origin.y - p.y) / (origin.x - p.x)) * (180 / M_PI);

    // Shift 0 degrees by 90 degrees clockwise and
    // Subtract the angle by 180, yielding angles between
    // -180 and 180 degrees.
    // 0 degrees will point straight up while +/-180
    // degrees will point straight down
    angle = fmod(angle + 180.0f, 180.0f) - 90.0f;

    // Return the angle with a bias, if there is one
    return angle + angleBias;
}

bool isEqual(const line_t f, const line_t g)
{
    return f.coefficient == g.coefficient && f.constant == g.constant;
}

void determineEdges(line_t *const f, line_t *const g)
{
    // The value of function f
    line_t _f = *f;
    // The value of function g
    line_t _g = *g;

    // Represents if f has cones or not
    bool fNoCone = isEqual(_f, NO_CONE_LINE);
    // Represents if g has cones or not
    bool gNoCone = isEqual(_g, NO_CONE_LINE);

    // If there are no cones, assume both lines
    if (fNoCone && gNoCone)
    {
        *f = leftDefault;
        *g = rightDefault;
    }
    // If f has no cones...
    else if (fNoCone && !gNoCone)
    {
        // Check if g is on the right or left side
        // and then assume f
        if (_g.coefficient < 0 ||
            _g.coefficient == INF_SLOPE && _g.constant > origin.x)
        {
            *f = leftDefault;
        }
        else
        {
            *f = rightDefault;
        }
    }
    // If g has no cones...
    else if (gNoCone && !fNoCone)
    {
        // Check if f is on the right or left side
        // and then assume g
        if (_f.coefficient < 0 ||
            _f.coefficient == INF_SLOPE && _f.constant > origin.x)
        {
            *g = leftDefault;
        }
        else
        {
            *g = rightDefault;
        }
    }
}

_Float32 calculateSteering(const pos_api::data_t data)
{
    // Start by getting the lines from the cones,
    // if there are any

    // The line between the blue cones
    line_t bLine = getLineFromCones(data.bClose, data.bFar);
    // The line between the yellow cones
    line_t yLine = getLineFromCones(data.yClose, data.yFar);

    // Determine which side the cones are on
    // and assume edges for non-existent edges
    determineEdges(&bLine, &yLine);

    // The intersect between the two lines, if there is one
    point_t intersect = getIntersect(bLine, yLine);
    // std::clog << "intersect @ (" << intersect.x << ", " << intersect.y << ")" << std::endl;

    // The angle between a horizontal line and
    // the line between origin and intersect
    _Float32 angle = getAngle(origin, intersect);

    // std::clog << "origin @ (" << origin.x << ", " << origin.y << ")" << std::endl;
    // std::clog << angle << std::endl;

    // A check for whether the angle is on the right side
    bool right = angle < 0;
    // The magnitude of the angle
    _Float32 magnitude = abs(angle);

    // If the angle is within the range that it's acceptable
    // to not turn, don't
    if (0.0f <= magnitude && magnitude <= zeroThreshold)
    {
        return 0.0f;
    }
    // If it's between no steering and maximum turn,
    // output a value between 0 and the maximum value
    else if (zeroThreshold < magnitude && magnitude <= maxThreshold)
    {
        // The value to output
        _Float32 val;
        // Get the percentage between no steering and max turn
        val = (magnitude - zeroThreshold) / (maxThreshold - zeroThreshold);
        // Multiply the percentage with the maximum value
        val *= MAX_ABS_STEERING_VAL;

        // Right turns have a negative angle
        if (right)
        {
            val = -val;
        }
        return val;
    }

    // If it's above the max turn threshold, turn
    // Multiply by -1 if we're turning to the right
    return MAX_ABS_STEERING_VAL * (right ? -1 : 1);
}
