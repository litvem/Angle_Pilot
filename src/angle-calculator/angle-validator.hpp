#ifndef DIT639_2023_GROUP_13_ANGLE_VALIDATOR_HPP
#define DIT639_2023_GROUP_13_ANGLE_VALIDATOR_HPP

#include <iostream>
#include <math.h>

namespace ang_vld {
    void setMarginOfError(_Float32 margin);
    void setZeroValTolerance(_Float32 tolerance);
    void registerSteering(_Float32 actual, _Float32 ours);
    void printResult();
}

#endif // !DIT639_2023_GROUP_13_ANGLE_VALIDATOR_HPP