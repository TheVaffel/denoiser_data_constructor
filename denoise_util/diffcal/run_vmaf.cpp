#include "run_vmaf.h"

#include <thread>
#include <iostream>
#include <cassert>

extern "C" {
#include <libvmaf/libvmaf.h>
}

#include "util.h"

void vmaf_update_buffers(VmafUserInfo& vmaf_info, float *im1, float *im2) {

  {
    std::lock_guard<std::mutex> lk(vmaf_info.vmaf_mutex);
  
    vmaf_info.image1 = im1;
    vmaf_info.image2 = im2;
    vmaf_info.can_reuse_buffers = false;
    vmaf_info.has_next = true;
  }

  vmaf_info.vmaf_cv.notify_all();
  
}

void vmaf_signal_done(VmafUserInfo& vmaf_info) {
  {
    std::lock_guard<std::mutex> lk(vmaf_info.vmaf_mutex);
  
    vmaf_info.done = true;
  }
  
  vmaf_info.vmaf_cv.notify_all();
}

void vmaf_wait_until_read(VmafUserInfo& vmaf_info) {

  std::unique_lock<std::mutex> lk(vmaf_info.vmaf_mutex);

  while(!vmaf_info.can_reuse_buffers) {
    vmaf_info.vmaf_cv.wait(lk);
  }
}

// Be aware that read_image is expected to take reference as first argument, which is
// opposite of the convention used in the rest of this code
int read_image(float *reference, float *test_image, float *tmp, int stride_bytes, void *user_data) {
  
  VmafUserInfo* usr =  reinterpret_cast<VmafUserInfo*>(user_data);
  assert((unsigned int)stride_bytes == usr->width * sizeof(float));
  
  {
    std::unique_lock<std::mutex> lk(usr->vmaf_mutex);
  
  
    // Assume input to VMAF is black-white
    while(!usr->has_next) {
      
      if(usr->done) {
	// Means we're done
	return 2;
      }

      usr->vmaf_cv.wait(lk);
    }
  
    for(int i = 0; i < usr->height; i++) {
      // The second image corresponds to reference, the first corresponds to the query image
      float *begin1 = test_image + (stride_bytes / sizeof(float)) * i;
      float *begin2 = reference + (stride_bytes / sizeof(float)) * i; 
      for(int j = 0; j < usr->width; j++) {
	*(begin1++) = rgb2y(usr->image1 + 3 * (usr->width * i + j));
	*(begin2++) = rgb2y(usr->image2 + 3 * (usr->width * i + j));
      }
    }

    /* 
    imwrite(usr->image1, "test_image" + std::to_string(usr->number) + ".png", usr->width, usr->height);
    imwrite(usr->image2, "reference" + std::to_string(usr->number) + ".png", usr->width, usr->height);
    
    imwrite(reference, "reference_grey" + std::to_string(usr->number) + ".png", usr->width, usr->height, 1);
    imwrite(test_image, "test_grey" + std::to_string(usr->number) + ".png", usr->width, usr->height, 1); */

    usr->has_next = false;
    usr->can_reuse_buffers = true;
  }

  usr->number++;
  usr->vmaf_cv.notify_all();
  
  
  return 0;
}

void run_compute(VmafUserInfo& vmaf_info, int width, int height) {
  double score;

  compute_vmaf(
    &score, // vmaf score
    (char*)"yuv420p", // format not necessarily that important (hopefully)
    width, // width
    height, // height
    read_image, // frame reading method
    (void*)&vmaf_info, // User data
    (char*)"/home/haakon/SDK/vmaf/model/vmaf_v0.6.1.pkl", // Model file
    (char*)VMAF_LOG_FILE, // log file name
    (char*)VMAF_LOG_FMT, // log format
    0, // disable clip
    0, // disable avx
    0, // enable_transform
    0, // phone_model
    1, 1, 1, // psnr, ssim, ms_ssim
    (char*)"mean", // pool method
    0, // Num threads (0 = all)
    1, // subsample
    0); // Enable confidence interval (no idea what that would mean)
  
  vmaf_info.score = score;
}

void run_vmaf(VmafUserInfo& vmaf_info, int width, int height) {
  vmaf_info.width = width;
  vmaf_info.height = height;
  vmaf_info.thread = new std::thread(run_compute,
				     std::ref(vmaf_info),
				     width,
				     height);
  
  
}

double vmaf_get_result(VmafUserInfo& vmaf_info) {
  vmaf_info.thread->join();
  delete vmaf_info.thread;
  return vmaf_info.score;
}



