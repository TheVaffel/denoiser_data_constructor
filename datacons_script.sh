#!/bin/bash
SCENE=~/data/glTF-Sample-Models-master/2.0/Sponza/glTF/Sponza.gltf

# This is the path from where you execute script to the folder containing the ChameleonRT and the Vulkan-glTF-PBR folders.
RELATIVE_DIR=.
OUTPUT_DIR=output

pushd $RELATIVE_DIR/Vulkan-glTF-PBR/build
./bin/Vulkan-glTF-PBR -p ../path.json -s $SCENE -o ../../$OUTPUT_DIR/shading_normal -f normal
./bin/Vulkan-glTF-PBR -p ../path.json -s $SCENE -o ../../$OUTPUT_DIR/albedo -f albedo
./bin/Vulkan-glTF-PBR -p ../path.json -s $SCENE -o ../../$OUTPUT_DIR/world_position -f position

popd
pushd $RELATIVE_DIR/ChameleonRT/build

./chameleonrt -optix $SCENE -path ../path.json -output ../../$OUTPUT_DIR/noisy

popd
