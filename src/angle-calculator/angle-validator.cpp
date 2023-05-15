#include "angle-validator.hpp"

_Float32 marginOfError = 0.3f;
_Float32 zeroValTolerance = 0.05f;

int16_t registeredFrames = 0;
int16_t passedFrames = 0;
int16_t zeroesRegistered = 0;
int16_t zeroesPassed = 0;
int16_t positivePassed = 0;
int16_t positiveUnder = 0;
int16_t positiveAbove = 0;
int16_t negativePassed = 0;
int16_t negativeUnder = 0;
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
    registeredFrames++;

    if (actual == 0)
    {
        zeroesRegistered++;
        if (!(actual - zeroValTolerance <= ours && ours <= actual + zeroValTolerance))
        {
            return;
        }

        zeroesPassed++;
        passedFrames++;
        return;
    }

    if (actual > 0)
    {
        if (ours < actual * (1.0f - marginOfError))
        {
            positiveUnder++;
            return;
        }
        if (actual * (1.0f + marginOfError) < ours)
        {
            positiveAbove++;
            return;
        }

        positiveAbove++;
        passedFrames++;
        return;
    }

    if (actual < 0)
    {
        if (ours < actual * (1.0f + marginOfError))
        {
            negativeUnder++;
            return;
        }
        if (actual * (1.0f - marginOfError) < ours)
        {
            negativeAbove++;
            return;
        }

        negativePassed++;
        passedFrames++;
        return;
    }
}

void ang_vld::printResult()
{
    const std::string SEP = "----------";

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
