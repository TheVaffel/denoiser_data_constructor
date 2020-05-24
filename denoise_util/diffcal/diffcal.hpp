#ifndef INCLUDE_DIFFCAL
#define INCLUDE_DIFFCAL

#include <vector>
#include <iostream>

#include <OpenImageIO/imageio.h>

namespace OpenImageIO = OIIO;

const int DIFF_NUM_METRICS = 4;

const int DIFF_RMSE_INDEX = 0;
const int DIFF_SSIM_INDEX = 1;
const int DIFF_TEMP_INDEX = 2;
const int DIFF_VMAF_INDEX = 3;

extern const char *diff_metric_names[DIFF_NUM_METRICS];
bool is_format(const char* arg);

struct DiffResultState {
  // One vector for each metric, containing score for each image
  std::vector<std::vector<float> > results;

  std::vector<float> means;
  std::vector<float> mins;
  std::vector<float> maxs;
  std::vector<float> variances;

  DiffResultState() : results(DIFF_NUM_METRICS),
		      means(DIFF_NUM_METRICS),
		      mins(DIFF_NUM_METRICS),
		      maxs(DIFF_NUM_METRICS),
		      variances(DIFF_NUM_METRICS) { }
};

class ImageIterator {
protected:
  int width, height;
public:
  int getWidth() {
    return width;
  }

  int getHeight() {
    return height;
  }
  
  virtual float *getImage1() = 0;
  virtual float *getImage2() = 0;
  virtual float *getLast() = 0;
  virtual bool forward() = 0;
  virtual bool hasLast() = 0;
};


struct ImageFileIterator : public ImageIterator {
  float *image1 = nullptr, *image2 = nullptr,
    *last_image = nullptr;
  int start_value, curr;
  static const int buff_size = 200;

  std::string path1, path2;

  char buff1[buff_size], buff2[buff_size];

public:
  
  ImageFileIterator(int start_value,
		    const std::string& path1,
		    const std::string& path2) {
    this->start_value = start_value;
    this->curr = start_value - 1;

    this->path1 = path1;
    this->path2 = path2;
    
    if(!this->forward()) {
      std::cerr << "Could not initialize image file iterator" << std::endl;
      exit(-1);
    }
  }

  virtual float *getImage1() {
    return image1;
  }

  virtual float *getImage2() {
    return image2;
  }

  virtual float *getLast() {
    return last_image;
  }

  virtual bool hasLast() {
    return curr != start_value;
  }

  virtual bool forward() {
    this->curr++;

    std::swap(this->image1, last_image);
    
    int num1 = snprintf(buff1, buff_size, path1.c_str(), curr);
    int num2 = snprintf(buff2, buff_size, path2.c_str(), curr);
    
    if(num1 >= buff_size - 2 || num2 >= buff_size - 2) {
      std::cerr << "Buffer size was " << buff_size << ", and snprint used " << num1 << " and " << num2 << " bytes respectively.. Check that buff_size is big enough" << std::endl;
      exit(-1);
    }
    
    std::unique_ptr<OpenImageIO::ImageInput> in1 = OpenImageIO::ImageInput::open(std::string(buff1));
    std::unique_ptr<OpenImageIO::ImageInput> in2 = OpenImageIO::ImageInput::open(std::string(buff2));

    if(!in1) {
      std::cout << "Could not find " << buff1 << ", iterator is done" << std::endl;
      return false;
    }

    if(!in2) {
      curr++;
      std::cout << "Could not find " << buff2 << ", assuming done" << std::endl;
      in1->close();
      return false;
    }

    int in1_width = in1->spec().width;
    int in1_height = in1->spec().height;
    int in2_width = in2->spec().width;
    int in2_height = in2->spec().height;

    // Use image buffer1 empty as indicator that this is first iteration
    if(image1 == nullptr) {
      this->width = in1_width;
      this->height = in1_height;

      image1 = new float[width * height * 3];
      image2 = new float[width * height * 3];
      last_image = new float[width * height * 3];

      
      // start this asynchronously 

    }

    if(in1_width != width || in1_height != height) {
      std::cerr << "Input dimensions of image 1 did not match already established dimensions!\n" <<
	"Established dimensions: " << width << ", " << height <<
	", dimensions of image 1: " << in1_width << ", " << in1_height << std::endl;
      
      delete[] image1;
      delete[] image2;
      delete[] last_image;
      exit(-1);
    }

    if(in2_width != width || in2_height != height) {
      std::cerr << "Input dimensions of image 2 did not match already established dimensions!\n" <<
	"Established dimensions: " << width << ", " << height <<
	", dimensions of image 2: " << in2_width << ", " << in2_height << std::endl;

      delete[] image1;
      delete[] image2;
      delete[] last_image;
      exit(-1);
    }

    in1->read_image(OpenImageIO::TypeDesc::FLOAT, image1);
    in1->close();
    in2->read_image(OpenImageIO::TypeDesc::FLOAT, image2);
    in2->close();
    
    std::cout << "Successfully iterated to images " << buff1 << " and " << buff2 << std::endl;
    
    return true;
  }
  
};

DiffResultState computeDiff(ImageIterator& imit);
void outputResult(DiffResultState& result_state, const std::string& filename);

#endif // INCLUDE_DIFFCAL
