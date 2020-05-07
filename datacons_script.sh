#!/bin/bash

# source datacons_header.sh
# source datacons_new_sponza_header.sh
source datacons_sponza_header.sh

pushd $RELATIVE_DIR/Vulkan-glTF-PBR/build
./bin/Vulkan-glTF-PBR -p ../../$PATH_FILE -s $SCENE -o ../../$OUTPUT_DIR/shading_normal,../../$OUTPUT_DIR/albedo,../../$OUTPUT_DIR/world_position -f normal,albedo,position --start 0
# ./bin/Vulkan-glTF-PBR -p ../../$PATH_FILE -s $SCENE -o ../../$OUTPUT_DIR/albedo -f albedo --start 0
# ./bin/Vulkan-glTF-PBR -p ../../$PATH_FILE -s $SCENE -o ../../$OUTPUT_DIR/world_position -f position --start 0

popd

pushd $RELATIVE_DIR/ChameleonRT/build

./chameleonrt -optix $SCENE_RAY -path ../../$PATH_FILE -output ../../$OUTPUT_DIR/color -start 0

popd

# pushd $RELATIVE_DIR/../nanort/build

# ./bin/path_tracer $SCENE_RAY -p ../../dataconstruction/$PATH_FILE --output ../../dataconstruction/$OUTPUT_DIR/color 1.0 $SCENE_BASE

# popd

pushd $RELATIVE_DIR/path_writer

./path_write ../$PATH_FILE -o ../$OUTPUT_DIR/camera_matrices.h

popd
