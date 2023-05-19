#! /usr/bin/sh

# This script pulls or builds all required images
echo "Pulling OpenDLV Vehicle View..."
docker pull chrberger/opendlv-vehicle-view:v0.0.64
echo "Done"
echo "Building h.264 Decoder..."
docker build https://github.com/chalmers-revere/opendlv-video-h264-decoder.git#v0.0.5 \
-f Dockerfile -t h264decoder:v0.0.5
echo "Done"
echo "Pulling Cone Detector..."
docker pull registry.git.chalmers.se/courses/dit638/students/2023-group-13/cone-detector:v1.0.0
echo "Done"
echo "Pulling Angle Calculator..."
docker pull registry.git.chalmers.se/courses/dit638/students/2023-group-13/angle-calculator:v1.0.0
echo "Done"
