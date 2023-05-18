#! /usr/bin/sh

# This script starts the OpenDLV Vehicle View
cd ../rec-files
echo "Starting OpenDLV Vehicle View"
docker run --rm -it --init --net=host -v $PWD:/opt/vehicle-view/recordings \
-v /var/run/docker.sock:/var/run/docker.sock -p 8081:8081 chrberger/opendlv-vehicle-view:v0.0.64
