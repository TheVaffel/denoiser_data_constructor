#include "pathreadc.hpp"

#include <iostream>
#include <fstream>

// self-chosen
const float fov = 45.0f;
// Assume constant aspect
const float screen_aspect = 16.0f / 9.0f;
const float zNear = 0.001f;
const float zFar = 256.0f;

void print_info() {
  std::cout << "Utility for writing a path JSON file as a C-header with matrices" << std::endl;
}

void print_usage() {
  std::cout << "Usage: ./path_write <path_file> [-o <output_file>]" << std::endl;
}

int main(int argc, const char** argv) {
  if(argc < 1) {
    print_info();
    print_usage();
    exit(-1);
  }

  std::string outfile_name = "camera_matrices.h";
  std::string pathfile_name = "";


  for(int i = 1; i < argc; i++) {
    if(std::string(argv[i]) == "-o") {
      outfile_name = argv[++i];
    } else {
      pathfile_name = argv[i];
    }
  }

  if(pathfile_name.length() == 0) {
    std::cout << "Name of path file not found" << std::endl;
    print_usage();
    exit(-1);
  }

  std::ofstream ofs;
  ofs.open(outfile_name.c_str(),
	   std::ofstream::out);

  // These should vary per scene, but oh well
  float position_limit_squared = 0.001;
  float normal_limit_squared = 1.0;

  glm::mat4 perspective_matrix = glm::perspective(glm::radians(fov), screen_aspect, zNear, zFar);

  ofs << "// Camera matrix input file produced by path_write, Håkon Flatval\n";
  ofs << "const float position_limit_squared = " << position_limit_squared << ";\n";
  ofs << "const float normal_limit_squared = " << normal_limit_squared << ";\n\n";

  std::vector<glm::mat4> vecs = getPath(pathfile_name);

  std::cout << "Outputting " << vecs.size() << " matrices" << std::endl;

  // Oof, check if this is actually right
  ofs << "// Matrices are given in row-major order\n"
      << "const float camera_matrices[" << vecs.size() << "][4][4] = {\n";
  
  for (uint i = 0; i < vecs.size(); i++) {
    glm::mat4 current_matrix = perspective_matrix * vecs[i];
    
    ofs << "\t{\n";
    for (int j = 0; j < 4; j++) {
      ofs << "\t\t{ " <<
	current_matrix[0][j] << ", " <<
	current_matrix[1][j] << ", " <<
	current_matrix[2][j] << ", " <<
	current_matrix[3][j] << " }";
      if(j != 3) {
	ofs << ",";
      }
      ofs << "\n";
    }
    ofs << "\t}";
    if (i != vecs.size() - 1) {
      ofs << ",";
    }
    ofs << "\n";
  }
  ofs << "};\n\n";

  ofs << "// Pixel offsets (I think everything is 0.5, but really not sure)\n";
  ofs << "const float pixel_offsets[" << vecs.size() << "][2] = {\n";
  for(uint i = 0; i < vecs.size(); i++) {
    ofs << "\t{ 0.5f, 0.5f }";
    if(i != vecs.size() - 1) {
      ofs << ",";
    }
    ofs << "\n";
  }
  ofs << "};\n";
  ofs << std::endl;

  ofs.close();
  
  std::cout << "Content written to " << outfile_name << std::endl;
}
