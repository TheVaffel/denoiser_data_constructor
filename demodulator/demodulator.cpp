#include <OpenImageIO/imageio.h>

#include <iostream>
#include <vector>

bool isFormat(const std::string& str) {
  // Check if contains percent
  for(int i = 0; i < str.length(); i++) {
    if (str[i] == '%') {
      return true;
    }
  }
  return false;
}

void printUsage() {
  std::cout << "./demodulator <format_noisy> <format_albedo> [-o output_format]" << std::endl;
}

int main(int argc, const char** argv) {
  std::vector<std::string> args(argv, argv + argc);

  if (argc < 3) {
    std::cerr << "Must have at least two arguments" << std::endl;
    printUsage();
    exit(-1);
  }
  
  std::string in1_format;
  std::string in2_format;
  std::string out_format = "color%03d.exr";
  
  int start_value = 0;

  for(int i = 1; i < args.size(); i++)  {
    if(args[i] == "-o") {
      out_format = args[++i];
    } else if(args[i] == "-s" || args[i] == "--start-index") {
      start_value = std::stoi(args[++i]);
    } else if(in1_format.length()) {
      in2_format = args[i];
    } else {
      in1_format = args[i];
    }
  }

  
  if (!isFormat(in1_format) ||
     !isFormat(in2_format) ||
     !isFormat(out_format)) {
    std::cerr << "All inputs must be formats" << std::endl;
    printUsage();
    exit(-1);
  }
  
  const int csize = 256;

  char buf_in1[csize],
    buf_in2[csize],
    buf_out[csize];

  int current = start_value; 

  int width = - 1, height;
  int nchans1, nchans2;

  float *output, *noisy, *albedo;
  
  while(true) {
    int n = snprintf(buf_in1, csize - 1, in1_format.c_str(), current);
    n = std::max(n, snprintf(buf_in2, csize - 1, in2_format.c_str(), current));
    n = std::max(n, snprintf(buf_out, csize - 1, out_format.c_str(), current));

    if(n >= csize - 1) {
      std::cerr << "One of the formats yielded a string too long, aborting" << std::endl;
      exit(-1);
    }
    
    OpenImageIO::ImageInput *in1 =
      OpenImageIO::ImageInput::open(buf_in1);
    if (!in1) {
      std::cerr << "Could not open " << buf_in1 << ", stopping here" << std::endl;
      break;
    }
    OpenImageIO::ImageInput *in2 =
      OpenImageIO::ImageInput::open(buf_in2);
    if (!in2) {
      std::cerr << "Could not open " << buf_in2 << ", stopping here" << std::endl;
      break;
    }

    if (width == -1) { // This means this is the first iteration
      width = in1->spec().width;
      height = in1->spec().height;
      if (width != in2->spec().width ||
	 height != in2->spec().height) {
	std::cerr << "Mismatching dimensions! Noisy dimensions: " << in1->spec().width << ", " << in1->spec().height << ", albedo dimensions: " << in2->spec().width << ", " << in2->spec().height << std::endl;
        exit(-1);
      }

      nchans1 = in1->spec().nchannels;
      nchans2 = in2->spec().nchannels;
      noisy = new float[nchans1 * width * height];
      albedo = new float[nchans2 * width * height];
      output = new float[3 * width * height];
    }

    in1->read_image(OpenImageIO::TypeDesc::FLOAT, noisy);
    in2->read_image(OpenImageIO::TypeDesc::FLOAT, albedo);

    in1->close();
    in2->close();
    
    float sum_maxdiff = 0.0;
    float maxalb = 0.0f;
    float maxnoise = 0.0f;
    
    for(int i = 0; i < height; i++) {
      for(int j = 0; j < width; j++) {
	int pix = i * width + j;
	float fs[3];
	for(int k = 0; k < 3; k++) {
	  maxalb = std::max(albedo[nchans2 * pix + k], maxalb);
	  maxnoise = std::max(noisy[nchans1 * pix + k], maxnoise);
	  // albedo[nchans2 * pix + k] /= 255.0f;
	  fs[k] = albedo[nchans2 * pix + k] > 1e-7 ?
	    noisy[nchans1 * pix + k] / albedo[nchans2 * pix + k] : 0.0;
	}

	// Compute a maximal difference to measure viability of result
	float maxdiff = std::max(std::max(std::abs(fs[0] - fs[1]),
					  std::abs(fs[1] - fs[2])),
				 std::abs(fs[2] - fs[0]));
	sum_maxdiff += maxdiff;

	// We compute average of the three illuminations, even though we
	// technically could store them separately, but it doesn't seem
	// like that's what the BMFR people want

	/* float av = (fs[0] + fs[1] + fs[2]) / 3.0f;

	for(int k = 0; k < 3; k++) {
	  output[3 * (i * width + j) + k] = av;
	  } */
	
	// Screw it, we try separating anyway
	for(int k = 0; k < 3 ; k++) {
	  output[3 * pix + k] =  fs[k];
	}
      }
    }
    
    std::cout << "Maxalb: " << maxalb << std::endl;
    std::cout << "Maxnoise: " << maxnoise << std::endl;

    OpenImageIO::ImageSpec spec(width, height, 3,
				OpenImageIO::TypeDesc::FLOAT);
    OpenImageIO::ImageOutput *out_im = OpenImageIO::ImageOutput::create(buf_out);
    if(!out_im || !out_im->open(buf_out, spec)) {
      std::cerr << "Could not open output image " << buf_out << ", aborting " << std::endl;
      exit(-1);
    }
    
    out_im->write_image(OpenImageIO::TypeDesc::FLOAT, output);
    out_im->close();
    std::cout << "Wrote output image " << buf_out << std::endl;
    
    float av_maxdiff = sum_maxdiff / (height * width);
    std::cout << "Average maxdiff of each pixel: " << av_maxdiff << std::endl;
    current++;
  }
    
    std::cout << "Done writing output" << std::endl;
}
