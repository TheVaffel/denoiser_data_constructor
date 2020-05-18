#include "util.h"

float luminance3(float* rgb) {
  return (2.0f * rgb[0] + 3.0f * rgb[1] + rgb[2]) / 6.0f;
}
