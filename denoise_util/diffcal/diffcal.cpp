#include <vector>
#include <iostream>
#include <fstream>

#include <OpenImageIO/imageio.h>
#include <nlohmann/json.hpp>

#include "run_vmaf.h"
#include "util.h"

namespace OpenImageIO = OIIO;
namespace json = nlohmann;

const int NUM_METRICS = 3;

const int RMSE_INDEX = 0;
const int SSIM_INDEX = 1;
const int TEMP_INDEX = 2;

const char* metric_names[NUM_METRICS] = {
  "RMSE",
  "SSIM",
  "Temporal error",
};

struct ResultState {
  // One vector for each metric, containing score for each image
  std::vector<std::vector<float> > results;

  ResultState() : results(NUM_METRICS) { }
};

bool is_format(const char* arg) {
  // Very simple, we check that the number of %'s is 1
  int num_per = 0;
  while(*arg) {
    if(*arg == '%') {
      num_per++;
    }
    arg++;
  }

  return num_per == 1;
}

float run_rmse(float *im1, float *im2, int num_pixels) {
  float sum = 0;
  for(int i = 0; i < num_pixels; i++) {
    float diff = (luminance3(im1 + 3 * i) - luminance3(im2 + 3 * i));
    sum += diff * diff;
  }

  float mean = sum / num_pixels;
  float rmse = sqrt(mean);
  return rmse;
}

float run_ssim(float *im1, float *im2, int width, int height) {
  float sum1 = 0, sum2 = 0;
  for(int i = 0; i < width * height; i++) {
    sum1 += luminance3(im1 + 3 * i);
    sum2 += luminance3(im2 + 3 * i);
  }

  float mean1 = sum1 / (width * height);
  float mean2 = sum2 / (width * height);

  float stdsum1 = 0, stdsum2 = 0;
  float covsum = 0;
  
  for(int i = 0; i < width * height; i++) {
    float diff1 = luminance3(im1 + 3 * i) - mean1;
    float diff2 = luminance3(im2 + 3 * i) - mean2;

    stdsum1 += diff1 * diff1;
    stdsum2 += diff2 * diff2;
    covsum += diff1 * diff2;
  }

  float std1 = sqrt(stdsum1 / (width * height - 1));
  float std2 = sqrt(stdsum2 / (width * height - 1));
  float cov = covsum / (width * height - 1);

  float K1 = 0.01f;
  float K2 = 0.03f;
  float L = 1.0f; // Dynamic range (1, right?)
  float C1 = (K1 * L) * (K1 * L);
  float C2 = (K2 * L) * (K2 * L);

  float SSIM = ((2 * mean1 * mean2 + C1) * (2 * cov + C2)) /
    ((mean1 * mean1 + mean2 * mean2 + C1) * (std1 * std1 + std2 * std2 + C2));

  return SSIM;
}

void run_temporal_error(ResultState& res, float *im1, float *im2, int width, int height) {

  float lsum = 0;
  for(int i = 0; i < width * height; i++) {
    float l1 = luminance3(im1 + 3 * i);
    float l2 = luminance3(im2 + 3 * i);

    lsum += (l1 - l2) * (l1 - l2);
  }

  float av = lsum / (width * height);

  res.results[TEMP_INDEX].push_back(av);
}

void run_diffs(ResultState& res, float* im1, float* im2, int width, int height) {
  // Found out scale is between 0 and 1.
  /* float mmax = -1e8, mmin = 1e8;
  
  for(int i = 0; i < width * height * 3; i++) {
    mmax = std::max(im1[i], mmax);
    mmin = std::min(im1[i], mmin);
  }
  std::cout << "Global max, global min: " << mmax << ", " << mmin << std::endl; */
  
  float rmse = run_rmse(im1, im2, width *  height);
  float ssim = run_ssim(im1, im2, width, height);
  
  res.results[RMSE_INDEX].push_back(rmse);
  res.results[SSIM_INDEX].push_back(ssim);
}

int main(int argc, const char **argv) 
{
  
  if (argc != 3 || (!is_format(argv[1]) || !is_format(argv[2]))) {
    std::cerr << "Must call diffcal with two arguments: ./diffcal <format1> <format2>, where each format takes an integer, e.g. image%00d.png" << std::endl;
    exit(-1);
  }

  const int start_value = 0;
  
  int curr = start_value;

  float *image_buffer1 = nullptr, *image_buffer2 = nullptr,
    *last_buffer = nullptr;

  ResultState result_state;

  VmafUserInfo vmaf_info;

  int width, height;

  while(true) {
    const int buff_size = 200;
    char buff1[buff_size], buff2[buff_size];
    int num1 = snprintf(buff1, buff_size, argv[1], curr);
    int num2 = snprintf(buff2, buff_size, argv[2], curr);

    if(num1 >= buff_size - 2 || num2 >= buff_size - 2) {
      std::cerr << "Buffer size was " << buff_size << ", and snprint used " << num1 << " and " << num2 << " bytes respectively.. Check that buff_size is big enough" << std::endl;
      exit(-1);
    }
    
    std::unique_ptr<OpenImageIO::ImageInput> in1 = OpenImageIO::ImageInput::open(std::string(buff1));
    std::unique_ptr<OpenImageIO::ImageInput> in2 = OpenImageIO::ImageInput::open(std::string(buff2));

    if(!in1) {
      std::cout << "Could not find " << buff1 << ", ending difftaking" << std::endl;
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
      last_buffer = new float[width * height * 3];

      
      // start this asynchronously 
      run_vmaf(vmaf_info, width, height);

    }

    if(in1_width != width || in1_height != height) {
      std::cerr << "Input dimensions of image 1 did not match already established dimensions!\n" <<
	"Established dimensions: " << width << ", " << height <<
	", dimensions of image 1: " << in1_width << ", " << in1_height << std::endl;
      
      delete[] image_buffer1;
      delete[] image_buffer2;
      delete[] last_buffer;
      exit(-1);
    }

    if(in2_width != width || in2_height != height) {
      std::cerr << "Input dimensions of image 2 did not match already established dimensions!\n" <<
	"Established dimensions: " << width << ", " << height <<
	", dimensions of image 2: " << in2_width << ", " << in2_height << std::endl;

      delete[] image_buffer1;
      delete[] image_buffer2;
      delete[] last_buffer;
      exit(-1);
    }

    in1->read_image(OpenImageIO::TypeDesc::FLOAT, image_buffer1);
    in1->close();
    in2->read_image(OpenImageIO::TypeDesc::FLOAT, image_buffer2);
    in2->close();

    vmaf_update_buffers(vmaf_info, image_buffer1, image_buffer2);    

    run_diffs(result_state, image_buffer1, image_buffer2, width, height);

    if(curr != start_value) {
      run_temporal_error(result_state, image_buffer1, last_buffer, width, height);
    }

    vmaf_wait_until_read(vmaf_info);
    std::swap(image_buffer1, last_buffer);
    

    std::cout << "Ran diff between " << buff1 << " and " << buff2 << std::endl;
    
    curr++;
  }

  vmaf_signal_done(vmaf_info);
  float vmaf_score = vmaf_get_result(vmaf_info);

  if(image_buffer1 == nullptr) {
    std::cout << "Could not find the first file pair, check if the format is correct " << std::endl;
    exit(-1);
  }

  json::json obj = json::json::object();

  for(int i = 0; i < NUM_METRICS; i++) {
    std::vector<float>& vv = result_state.results[i];

    float sum = 0.0f;
    float mmin = 1e8, mmax = -1e8;
    for(unsigned int j = 0; j < vv.size(); j++) {
      sum += vv[j];
      mmin = std::min(mmin, vv[j]);
      mmax = std::max(mmax, vv[j]);
    }
    
    float mean = sum / vv.size();

    float sqsum = 0;
    for(unsigned int j = 0; j < vv.size(); j++) {
      float diff = vv[j] - mean;
      sqsum += diff * diff;
    }

    float variance = sqsum / (vv.size() - 1);
    float standard_deviation = sqrt(variance);

    std::cout << "--------------------\n"
	      << "Results using " << metric_names[i] << "\n"
	      << "Mean: " << mean << "\n"
	      << "Variance: " << variance << "\n"
	      << "Standard deviation: " << standard_deviation << "\n"
	      << "Minimum value: " << mmin << "\n"
	      << "Maximum value: " << mmax << "\n"
	      << "--------------------\n"
	      << std::endl;

    obj[metric_names[i]] = json::json(vv);
  }

  const std::string json_output_filename = "diff_results.json";
  
  std::ofstream ofs(json_output_filename);
  ofs << std::setw(4) << obj << std::endl;
  ofs.close();

  std::cout << "The above reported results are detailed in " << json_output_filename << std::endl;

  std::cout << "VMAF score was " << vmaf_score << std::endl;
  std::cout << "Additional VMAF info can be found in " << VMAF_LOG_FILE << std::endl;
}
