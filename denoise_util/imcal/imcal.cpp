#include <vector>
#include <iostream>
#include <random>

#include <OpenImageIO/imageio.h>

namespace OpenImageIO = OIIO;

struct RNG {
  
  std::default_random_engine generator;
  std::uniform_real_distribution<float> distribution;

  RNG(int a, int b) : distribution(a, b) { }
};

void cal(float* im1, float* im2, float* res, int width, int height, RNG& rng) {

  float mmin[3] = { 1e8, 1e8, 1e8 };
  float mmax[3] = { -1e8, -1e8, -1e8 };
  
  for(int i = 0; i < height * width; i++) {
    for(int j = 0; j < 3; j++) {
      res[3 * i + j] = im1[3 * i + j] - im2[3 * i + j];
      mmin[j] = std::min(mmin[j], res[3 * i + j]);
      mmax[j] = std::max(mmax[j], res[3 * i + j]);
    }
  }

  float diff[3];

  for(int j = 0; j < 3; j++) {
    diff[j] = mmin[j] == mmax[j] ? 1.0 : mmax[j] - mmin[j];
  }
  
  for(int i = 0; i < height * width; i++) {
    for(int j = 0; j < 3; j++) {
      // res[3 * i + j] = (res[3 * i + j] - mmin[j]) / diff[j];
      // res[3 * i + j] += 0.5f;
      
      res[3 * i + j] = rng.distribution(rng.generator);
    }
  }
  
}

void imwrite(float *im, const std::string& name, int width, int height) {
  
  std::unique_ptr<OpenImageIO::ImageOutput> out = OpenImageIO::ImageOutput::create(name);
    if(!out) {
      std::cerr << "Cannot open output file " << name << ", exiting" << std::endl;
      exit(-1);
    }

    OpenImageIO::ImageSpec spec(width, height, 3, OpenImageIO::TypeDesc::FLOAT);
    out->open(name, spec);
    out->write_image(OpenImageIO::TypeDesc::FLOAT, im);
    out->close();

    std::cout << "Wrote output image " << name << std::endl;
    
}

int main(int argc, const char **argv) 
{
  
  if (argc != 4) {
    std::cerr << "Must call imcal with three arguments: ./imcal <format1> <format2> <result>, where each format takes an integer, e.g. image%00d.png" << std::endl;
    exit(-1);
  }
  
  int curr = 0;

  float *image_buffer1 = nullptr,
    *image_buffer2 = nullptr,
    *output_buffer = nullptr;

  int width, height;
  
  RNG rng(0, 1);
    
  while(true) {
    const int buff_size = 200;
    char buff1[buff_size], buff2[buff_size], buff3[buff_size];
    int num1 = snprintf(buff1, buff_size, argv[1], curr);
    int num2 = snprintf(buff2, buff_size, argv[2], curr);
    int num3 = snprintf(buff3, buff_size, argv[3], curr);

    if(num1 >= buff_size - 2 || num2 >= buff_size - 2 || num3 >= buff_size - 2) {
      std::cerr << "Buffer size was " << buff_size << ", and snprint used " << num1 << ", " << num2 << " and " << num3 << " bytes respectively.. Check that buff_size is big enough" << std::endl;
      exit(-1);
    }
    std::unique_ptr<OpenImageIO::ImageInput> in1 = OpenImageIO::ImageInput::open(std::string(buff1));
    std::unique_ptr<OpenImageIO::ImageInput> in2 = OpenImageIO::ImageInput::open(std::string(buff2));

    if(!in1) {
      std::cout << "Could not find " << buff1 << ", ending imcal" << std::endl;
      break;
    }

    if(!in2) {
      curr++;
      continue;
    }

    int in1_width = in1->spec().width;
    int in1_height = in1->spec().height;
    int in2_width = in2->spec().width;
    int in2_height = in2->spec().height;

    // Use image buffer1 empty as indicator that this is first iteration
    if(image_buffer1 == nullptr) {
      width = in1_width;
      height = in1_height;

      image_buffer1 = new float[width * height * 3];
      image_buffer2 = new float[width * height * 3];
      output_buffer = new float[width * height * 3];
    }

    if(in1_width != width || in1_height != height) {
      std::cerr << "Input dimensions of image 1 did not match already established dimensions!\n" <<
	"Established dimensions: " << width << ", " << height <<
	", dimensions of image 1: " << in1_width << ", " << in1_height << std::endl;
      
      delete[] image_buffer1;
      delete[] image_buffer2;
      exit(-1);
    }

    if(in2_width != width || in2_height != height) {
      std::cerr << "Input dimensions of image 2 did not match already established dimensions!\n" <<
	"Established dimensions: " << width << ", " << height <<
	", dimensions of image 2: " << in2_width << ", " << in2_height << std::endl;

      delete[] image_buffer1;
      delete[] image_buffer2;
      exit(-1);
    }

    in1->read_image(OpenImageIO::TypeDesc::FLOAT, image_buffer1);
    in1->close();
    in2->read_image(OpenImageIO::TypeDesc::FLOAT, image_buffer2);
    in2->close();

    
    cal(image_buffer1, image_buffer2, output_buffer, width, height, rng);

    imwrite(output_buffer, buff3, width, height);

    curr++;
  }

  if(image_buffer1 == nullptr) {
    std::cout << "Could not find the first file pair, check if the format is correct " << std::endl;
    exit(-1);
  }

}
