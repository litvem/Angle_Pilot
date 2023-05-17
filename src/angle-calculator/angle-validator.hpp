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

#ifndef DIT639_2023_GROUP_13_ANGLE_VALIDATOR_HPP
#define DIT639_2023_GROUP_13_ANGLE_VALIDATOR_HPP

#include <iostream>
#include <math.h>

/**
 * A namespace containing functions for testing the accuracy
 * of the angle calculator by comparing its output to the
 * original ground steering request.
 * 
 * Author: Bao Quan Lindgren (2023)
 */
namespace ang_vld {
    /**
     * Sets the acceptable margin of error for each frame
     * with a non-zero value.
     * 
     * @param margin the fraction of the original non-zero
     * ground steering request that the calculated value
     * can at most deviate with
     */
    void setMarginOfError(_Float32 margin);

    /**
     * Sets the acceptable deviation for any frame where
     * the original ground steering request was zero.
     * 
     * @param tolerance the value that the calculated
     * value can deviate with, if the original ground
     * steering request is zero
     */
    void setZeroValTolerance(_Float32 tolerance);

    /**
     * Register a frame to the test and compare the output
     * value of the angle calculator with the actual ground
     * steering request.
     * 
     * @param actual the original ground steering request
     * @param ours the output from the angle calculator
     */
    void registerSteering(_Float32 actual, _Float32 ours);

    /**
     * Prints the current statistics of the accuracy test.
     * This includes the total amount of registered frames,
     * the total amount of frames that passed the test,
     * along with more data.
     */
    void printResult();
} // !namespace ang_vld

#endif // !DIT639_2023_GROUP_13_ANGLE_VALIDATOR_HPP