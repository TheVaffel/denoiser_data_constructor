#!/bin/bash
SCENE=~/data/glTF-Sample-Models-master/2.0/Sponza/glTF/Sponza.gltf

# This is the path from where you execute script to the folder containing the ChameleonRT and the Vulkan-glTF-PBR folders.
RELATIVE_DIR=.
OUTPUT_DIR=output
PATH_FILE=path.json

pushd $RELATIVE_DIR/Vulkan-glTF-PBR/build
./bin/Vulkan-glTF-PBR -p ../../$PATH_FILE -s $SCENE -o ../../$OUTPUT_DIR/shading_normal -f normal --start 0
./bin/Vulkan-glTF-PBR -p ../../$PATH_FILE -s $SCENE -o ../../$OUTPUT_DIR/albedo -f albedo --start 0
./bin/Vulkan-glTF-PBR -p ../../$PATH_FILE -s $SCENE -o ../../$OUTPUT_DIR/world_position -f position --start 0

popd
pushd $RELATIVE_DIR/ChameleonRT/build

# ./chameleonrt -optix $SCENE -path ../../path.json -output ../../$OUTPUT_DIR/noisy -start 0
./chameleonrt -optix $SCENE -path ../../path.json -output ../../$OUTPUT_DIR/color -start 0

popd

pushd $RELATIVE_DIR/demodulator

# ./demodulator ../$OUTPUT_DIR/noisy%d.png ../$OUTPUT_DIR/albedo%d.exr -o ../$OUTPUT_DIR/color%d.exr --start-index 0

popd

pushd $RELATIVE_DIR/path_writer

./path_write ../$PATH_FILE -o ../$OUTPUT_DIR/camera_matrices.h

popd
