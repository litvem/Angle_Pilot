#include "PrimeChecker.hpp"
//ossian was here
bool PrimeChecker::isPrime(uint16_t n)
{
    if (n < 2 || n % 2 == 0)
    {
        return false;
    }
    else
    {
        for (uint16_t i{3}; i * i <= n; i += 2)
        {
            if (n % i == 0)
            {
                return false;
            }
        }
    }
    return true;
}

