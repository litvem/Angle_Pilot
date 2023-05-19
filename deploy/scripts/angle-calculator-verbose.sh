#! /usr/bin/sh

# This script starts the Angle Calculator in verbose test mode
echo "Starting Angle Calculator"
docker run --rm -ti --init -v /tmp:/tmp --ipc=host \
registry.git.chalmers.se/courses/dit638/students/2023-group-13/angle-calculator:v1.0.0 \
--height=110 --width=640 --z=55 --m=75 --y=-0.5 --l=3 --b=0 --verbose
