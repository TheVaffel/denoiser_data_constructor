#ifndef INCLUDE_RUN_VMAF
#define INCLUDE_RUN_VMAF

#include <thread>
#include <mutex>
#include <condition_variable>

#define VMAF_LOG_FILE "vmaf_log.json"
#define VMAF_LOG_FMT "json"

struct VmafUserInfo {
  bool has_next = false;
  bool can_reuse_buffers = true;
  bool done = false;

  std::thread* thread;

  std::condition_variable vmaf_cv;
  std::mutex vmaf_mutex;

  double score;
  int width, height;
  
  float *image1,
    *image2;
};

double vmaf_get_result(VmafUserInfo& vmaf_info);

void vmaf_wait_until_read(VmafUserInfo& vmaf_info);

void vmaf_signal_done(VmafUserInfo& vmaf_info);
  
void vmaf_update_buffers(VmafUserInfo& vmaf_info, float *im1, float *im2);
  
void run_vmaf(VmafUserInfo& vmaf,
	      int width, int height);


#endif // INCLUDE_RUN_VMAF
