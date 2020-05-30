#include <OpenImageIO/imageio.h>

#include <vector>
#include <sstream>

namespace OpenImageIO = OIIO;

void print_usage() {
  std::cout << "Usage: \n"
	    << "./exrconvert -i <input_prefix> -o <output_prefix> "
	    << "[-f <index_factor>] [-s <start_index>]" << std::endl;
}

int main(int argc, const char ** argv) {
  std::vector<std::string> args(argv, argv + argc);

  int factor = 1;
  int start = 0;
  std::string informat = "";
  std::string outformat = "";
  
  
  for(uint i = 0; i < argc; i++) {
    if(args[i] == "-f" || args[i] == "--factor") {
      factor = std::stoi(args[++i]);
    } else if(args[i] == "-i" || args[i] == "--input") {
      informat = args[++i];
    } else if(args[i] == "-o" || args[i] == "--output") {
      outformat = args[++i];
    } else if(args[i] == "-s" || args[i] == "--start") {
      start = std::stoi(args[++i]);
    } else if(args[i] == "-h" || args[i] == "--help") {
      print_usage();
      exit(0);
    }
  }

  if(informat.length() == 0 ||
     outformat.length() == 0) {
    std::cout << "Output and input formats must be specified" << std::endl;
    print_usage();
    exit(-1);
  }

  int curr_ind = start;
  
  while(true) {
    
    std::ostringstream oss;
    oss << informat << curr_ind << ".exr";
    std::string in_filename = oss.str();
      
    std::unique_ptr<OpenImageIO::ImageInput> in = OpenImageIO::ImageInput::open(in_filename);

    if(!in) {
      std::cout << "Could not find input file " << in_filename
		<< ", considering job done" << std::endl;
      break;
    }
    const OpenImageIO::ImageSpec &spec = in->spec();
    int xres = spec.width;
    int yres = spec.height;
    int channels = spec.nchannels;
    std::vector<float> pixels(xres * yres * channels);
    in->read_image(OpenImageIO::TypeDesc::FLOAT, pixels.data());
    in->close();

    oss = std::ostringstream();
    oss << outformat << (factor * curr_ind) << ".png";

    std::string out_filename = oss.str();
    std::unique_ptr<OpenImageIO::ImageOutput> out = OpenImageIO::ImageOutput::create(out_filename);
    if(!out) {
      std::cerr << "Cannot make output file " << out_filename << ", exiting" << std::endl;
      exit(-1);
    }

    const OpenImageIO::ImageSpec spec2(xres, yres, channels,
				       OpenImageIO::TypeDesc::UINT8);
    out->open(out_filename, spec2);
    out->write_image(OpenImageIO::TypeDesc::FLOAT,
		     pixels.data());
    out->close();

    std::cout << "Successfully converted " << in_filename << " to " << out_filename << std::endl;
    
    curr_ind++;
  }

  return 0;
}
