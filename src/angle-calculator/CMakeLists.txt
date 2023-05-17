# Copyright (C) 2023  Robert Einer, Emma Litvin, Ossian Ålund, Bao Quan Lindgren, Khaled Adel Saleh Mohammed Al-Baadani
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.2)

project(angle-calculator)

# Using C++14
set(CMAKE_CXX_STANDARD 14)

# Enable pthreads and link librt and make it statically linked
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -pthread -lrt")

# Add the executable
add_executable(
    ${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/angle-calculator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../api/position.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/angle-validator.cpp
)