#include <vector>
#include <iostream>
#include <fstream>

#include <OpenImageIO/imageio.h>
#include <nlohmann/json.hpp>

#include "diffcal.hpp"
#include "run_vmaf.h"
#include "util.h"

namespace OpenImageIO = OIIO;
namespace json = nlohmann;


const char *diff_metric_names[DIFF_NUM_METRICS] = {
  "RMSE",
  "SSIM",
  "Temporal error",
  "VMAF",
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

void run_temporal_error(DiffResultState& res, float *im1, float *im2, int width, int height) {

  float lsum = 0;
  for(int i = 0; i < width * height; i++) {
    float l1 = luminance3(im1 + 3 * i);
    float l2 = luminance3(im2 + 3 * i);

    lsum += (l1 - l2) * (l1 - l2);
  }

  float av = lsum / (width * height);

  res.results[DIFF_TEMP_INDEX].push_back(av);
}

void run_diffs(DiffResultState& res, float* im1, float* im2, int width, int height) {
  
  float rmse = run_rmse(im1, im2, width *  height);
  float ssim = run_ssim(im1, im2, width, height);
  
  res.results[DIFF_RMSE_INDEX].push_back(rmse);
  res.results[DIFF_SSIM_INDEX].push_back(ssim);
}

void computeStatistics(DiffResultState& state) {
  for(int i = 0; i < DIFF_NUM_METRICS; i++) {
    std::vector<float>& vv = state.results[i];

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

    state.means[i] = mean;
    state.mins[i] = mmin;
    state.maxs[i] = mmax;
    state.variances[i] = variance;
  }
}

DiffResultState computeDiff(ImageIterator& imit) {
  
  VmafUserInfo vmaf_info;
  run_vmaf(vmaf_info, imit.getWidth(), imit.getHeight());

  DiffResultState result_state;

  while(true) {
    vmaf_update_buffers(vmaf_info, imit.getImage1(), imit.getImage2());
    
    run_diffs(result_state, imit.getImage1(), imit.getImage2(), imit.getHeight(), imit.getWidth());

    if(imit.hasLast()) {
      run_temporal_error(result_state, imit.getImage1(), imit.getLast(), imit.getWidth(), imit.getHeight());
    }

    vmaf_wait_until_read(vmaf_info); 
    
    if(!imit.forward()) {
      break;
    }
  }
  
  vmaf_signal_done(vmaf_info);
  
  // Do this to ensure vmaf thread finishes
  vmaf_get_result(vmaf_info);
  
  // Read VMAF results and put them together with the rest of the scores
  json::json obj;
  std::ifstream ifs(VMAF_LOG_FILE);
  
  if(!ifs) {
    std::cerr << "Could not open VMAF log file " << VMAF_LOG_FILE << std::endl;
    exit(-1);
  }
  ifs >> obj;
  ifs.close();

  json::json frame_list = obj["frames"];
  
  for(json::json& elem : frame_list) {
    float score = elem["metrics"]["vmaf"];
    result_state.results[DIFF_VMAF_INDEX].push_back(score);
  }

  return result_state;
}

void outputResult(DiffResultState& result_state) {
  
  json::json obj = json::json::object();

  computeStatistics(result_state);
  
  for(int i = 0; i < DIFF_NUM_METRICS; i++) {
    
    std::cout << "--------------------\n"
	      << "Results using " << diff_metric_names[i] << "\n"
	      << "Mean: " << result_state.means[i] << "\n"
	      << "Variance: " << result_state.variances[i] << "\n"
	      << "Standard deviation: " << sqrt(result_state.variances[i]) << "\n"
	      << "Minimum value: " << result_state.mins[i] << "\n"
	      << "Maximum value: " << result_state.maxs[i] << "\n"
	      << "--------------------\n"
	      << std::endl;

    obj[diff_metric_names[i]] = json::json(result_state.results[i]);
  }

  const std::string json_output_filename = "diff_results.json";
  
  std::ofstream ofs(json_output_filename);
  ofs << std::setw(4) << obj << std::endl;
  ofs.close();

  std::cout << "The above reported results are detailed in " << json_output_filename << std::endl;

  // std::cout << "VMAF score was " << vmaf_score << std::endl;
  std::cout << "Additional VMAF info can be found in " << VMAF_LOG_FILE << std::endl;
}
