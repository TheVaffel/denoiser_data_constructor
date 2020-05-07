#!/bin/bash

export SCENE=~/data/glTF-Sample-Models-master/2.0/Sponza/glTF/Sponza.gltf
# export SCENE=~/data/san_miguel/san-miguel-low-poly.gltf
# export SCENE_BASE=~/data/living_room/glTF/
# export SCENE="$SCENE_BASE/living_room.gltf"
export SCENE_RAY="$SCENE"
# export SCENE_RAY="$SCENE_BASE/living_room.obj"

# This is the path from where you execute script to the folder containing the ChameleonRT and the Vulkan-glTF-PBR folders.
export RELATIVE_DIR=.
export OUTPUT_DIR=output
export PATH_FILE=path.json
# export PATH_FILE=living_path.json
