# EXR Convert

Simple utility for converting `.exr` image files to `.png` files.

### Usage

```
./exrconvert -i <input_prefix> -o <output_prefix> [-f <index_factor>] [-s <start_index>]
```

The program will start with index `<start_index>` (0 by default) and convert images of the form `<input_prefix>N.exr` into images on the form `<output_prefix>M.png` until `<input_prefix>N.exr` cannot be found for the given `N`. A factor can be specified (default is 1), with with the start index will be multiplied to construct the output file name, i.e. `M` in `<output_prefix>M.png` is defined by `M = <index_factor> * N`.