#! /usr/bin/sh

# This script starts the h.264 Decoder
xhost +
echo "Starting h.264 Decoder"
docker run --rm -it --init --net=host -v /tmp:/tmp --ipc=host \
h264decoder:v0.0.5 --cid=253 --name=img
