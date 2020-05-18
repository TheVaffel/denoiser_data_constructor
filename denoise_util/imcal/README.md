# Imcal

Simple program that takes a number of input pictures and combine them to output pictures. Made to be heavily modified by user.

### Usage

Modify the code to perform the desired image operation. Then do

```
./imcal <format1> <format2> <result_format>
```

where each format is of C `printf` format taking one integer number, e.g. `image%d.png`. The program will start at zero and continue until it cannot find the first of the two input images. It will skip image pairs where the first input image is found but the second is not.