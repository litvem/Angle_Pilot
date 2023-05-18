#! /usr/bin/sh

# This script starts the Cone Detector in normal mode
xhost +
echo "Starting Cone Detector"
docker run --rm -it --init --net=host -e DISPLAY=$DISPLAY -v /tmp:/tmp \
--ipc=host registry.git.chalmers.se/courses/dit638/students/2023-group-13/cone-detector:v1.0.0 \
--cid=253 --name=img --width=640 --height=480
