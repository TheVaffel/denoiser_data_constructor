#include "util.h"


#include <OpenImageIO/imageio.h>

namespace OpenImageIO = OIIO;

float luminance3(float* rgb) {
  return (0.2126f * rgb[0] + 0.7152f * rgb[1] + 0.0722f * rgb[2]);
}

float rgb2y(float* rgb) {
  return (0.299f * rgb[0] + 0.587f * rgb[1] + 0.114f * rgb[2]) * 255;
}


void imwrite(float *im, const std::string& name, int width, int height, int num_channels) {
  
  std::unique_ptr<OpenImageIO::ImageOutput> out = OpenImageIO::ImageOutput::create(name);
    if(!out) {
      std::cerr << "Cannot open output file " << name << ", exiting" << std::endl;
      exit(-1);
    }

    OpenImageIO::ImageSpec spec(width, height, num_channels, OpenImageIO::TypeDesc::FLOAT);
    out->open(name, spec);
    out->write_image(OpenImageIO::TypeDesc::FLOAT, im);
    out->close();

    std::cout << "Wrote output image " << name << std::endl;
    
}
