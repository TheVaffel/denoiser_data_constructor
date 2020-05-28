#include <string>

float luminance3(float* rgb);
float rgb2y(float* rgb);

void imwrite(float *im, const std::string& name, int width, int height, int num_channels = 3);
