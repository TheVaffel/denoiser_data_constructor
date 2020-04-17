# Denoiser Data Constructor

A script using a rasterizer, a ray tracer, and some utilities to make datasets for ray tracing denoisers

## Cloning

Since this project contains submodules, you should clone with the `--recursive` argument:

```
$ git clone https://github.com/TheVaffel/denoiser_data_constructor.git --recursive
```

## Building

Vulkan-glTF-PBR and ChameleonRT must be compiled separately. You can create a `build` folder in each of them and run CMake. ChameleonRT has some nifty dependencies, and has its own `cmake_script.sh` to give a default build.

`path_writer` and `demodulator` have their own Makefiles

## Running

The `datacons_script.sh` should construct all the data necessary. By default, it conforms to the standard of the Blockwise Multi-Order Feature Regression algorithm (noisy buffer, feature buffers containing surface normals, world position and albedo). The noisy buffer is "divided by" the albedo buffer to give a noisy illumination buffer by the `demodulator` code. `path_writer` outputs a header files with the camera matrix (including perspective matrix) for each image in the output.

## Path description

An example path is given in `path.json`. The path is described by a number of key frames with given position and looking direction (always assumes the positive y-axis is up), and these are linearly interpolated between keyframes. The difference in timestamps give the number of frames from one keyframe to another. __NB__: Users should take care not to produce singularities when providing the directions.