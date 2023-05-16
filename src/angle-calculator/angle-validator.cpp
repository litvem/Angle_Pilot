#include "angle-validator.hpp"

// The acceptable margin of error for each frame with a non-zero value
_Float32 marginOfError = 0.3f;
// The acceptable deviation for any frame where the original
// ground steering request was zero
_Float32 zeroValTolerance = 0.05f;

// The total amount of frames registered in the test
int16_t registeredFrames = 0;
// The subset of registered frames that have passed the test
int16_t passedFrames = 0;

// The subset of registered frames with had zero as the original
// ground steering request
int16_t zeroesRegistered = 0;
// The subset of passed frames that were compared to a zero in
// the original ground steering request
int16_t zeroesPassed = 0;

// The subset of passed frames that were compared to a positive
// value in the original ground steering request
int16_t positivePassed = 0;
// The subset of registered frames that were compared to a positive
// value in the original ground steering request but failed due to
// having value that's too low
int16_t positiveUnder = 0;
// The subset of registered frames that were compared to a positive
// value in the original ground steering request but failed due to
// having value that's too high
int16_t positiveAbove = 0;

// The subset of passed frames that were compared to a negative
// value in the original ground steering request
int16_t negativePassed = 0;
// The subset of registered frames that were compared to a positive
// value in the original ground steering request but failed due to
// having value that's too low
int16_t negativeUnder = 0;
// The subset of registered frames that were compared to a positive
// value in the original ground steering request but failed due to
// having value that's too high
int16_t negativeAbove = 0;

void ang_vld::setMarginOfError(_Float32 margin)
{
    marginOfError = margin;
}

void ang_vld::setZeroValTolerance(_Float32 tolerance)
{
    zeroValTolerance = tolerance;
}

void ang_vld::registerSteering(_Float32 actual, _Float32 ours)
{
    // Register the frame
    registeredFrames++;

    // The case for when the original ground steering request is zero
    if (actual == 0)
    {
        // Register the zero frame
        zeroesRegistered++;

        // Do nothing if the output value is outside the accepted range
        if (!(actual - zeroValTolerance <= ours && ours <= actual + zeroValTolerance))
        {
            return;
        }

        // Otherwise register that the frame passed and
        // that it's a zero frame that passed
        passedFrames++;
        zeroesPassed++;
        return;
    }

    // The case for positive original ground steering requests
    if (actual > 0)
    {
        // If the output value is below the acceptable range,
        // register that it was too low for a positive original
        // ground steering request
        if (ours < actual * (1.0f - marginOfError))
        {
            positiveUnder++;
            return;
        }

        // If the output value is above the acceptable range,
        // register that it was too high for a positive original
        // ground steering request
        if (actual * (1.0f + marginOfError) < ours)
        {
            positiveAbove++;
            return;
        }

        // Otherwise the value must be in the acceptable range,
        // so we register it as passed and that it's a positive
        // frame that passed
        passedFrames++;
        positiveAbove++;
        return;
    }

    // The case for negative original ground steering requests
    if (actual < 0)
    {
        // If the output value is below the acceptable range,
        // register that it was too low for a negative original
        // ground steering request
        if (ours < actual * (1.0f + marginOfError))
        {
            negativeUnder++;
            return;
        }

        // If the output value is above the acceptable range,
        // register that it was too high for a negative original
        // ground steering request
        if (actual * (1.0f - marginOfError) < ours)
        {
            negativeAbove++;
            return;
        }

        // Otherwise the value must be in the acceptable range,
        // so we register it as passed and that it's a negative
        // frame that passed
        passedFrames++;
        negativePassed++;
        return;
    }
}

void ang_vld::printResult()
{
    const std::string SEP = "----------";

    // This is pretty self explanatory...
    std::cout << SEP << std::endl;
    std::cout << "Accuracy report" << std::endl;
    std::cout << SEP << std::endl;
    std::cout << "Total frames: " << registeredFrames << std::endl;
    std::cout << "Passed frames: " << passedFrames << std::endl;
    std::cout << "Overall accuracy: " << (_Float32) (100 * passedFrames) / (_Float32) registeredFrames << "%" << std::endl;
    std::cout << SEP << std::endl;
    std::cout << "Total zeroes: " << zeroesRegistered << std::endl;
    std::cout << "Values within tolerated zero value: " << zeroesPassed << std::endl;
    std::cout << SEP << std::endl;
    std::cout << "Values above tolerated positive values: " << positiveAbove << std::endl;
    std::cout << "Values within the tolerated positive values: " << positivePassed << std::endl;
    std::cout << "Values below tolerated positive values: " << positiveUnder << std::endl;
    std::cout << SEP << std::endl;
    std::cout << "Values above tolerated negative values: " << negativeAbove << std::endl;
    std::cout << "Values within the tolerated negative values: " << negativePassed << std::endl;
    std::cout << "Values below tolerated negative values: " << negativeUnder << std::endl;
    std::cout << SEP << std::endl;
}
