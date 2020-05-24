#include "run_vmaf.h"

#include <thread>
#include <iostream>
#include <cassert>

#include <libvmaf/libvmaf.h>

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

int read_image(float *im1, float *im2, float *tmp, int stride_bytes, void *user_data) {
  
  VmafUserInfo* usr =  reinterpret_cast<VmafUserInfo*>(user_data);
  assert((unsigned int)stride_bytes == usr->width * sizeof(float));
  
  {
    std::unique_lock<std::mutex> lk(usr->vmaf_mutex);
  
  
    // Assume input to VMAF is black-white
    while(!usr->has_next) {
      
      if(usr->done) {
	// Means we're done
	return 1;
      }

      usr->vmaf_cv.wait(lk);
    }
  
    for(int i = 0; i < usr->height; i++) {
      float *begin1 = im1 + (stride_bytes / sizeof(float)) * i;
      float *begin2 = im2 + (stride_bytes / sizeof(float)) * i;
      for(int j = 0; j < usr->width; j++) {
	*(begin1++) = luminance3(usr->image1 + 3 * (usr->width * i + j));
	*(begin2++) = luminance3(usr->image2 + 3 * (usr->width * i + j));
      }
    }

    usr->has_next = false;
    usr->can_reuse_buffers = true;
  }
  
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



