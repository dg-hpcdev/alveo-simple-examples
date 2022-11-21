/**
*
* Copyright 2022 Design Gateway Co., Ltd.
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*     http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
* 
*/


#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <vector>
#include <chrono>
#include <iostream>
#include <fstream>
#include <CL/cl2.hpp>

char* read_bin_file(std::string path, size_t& file_size);
cl::Platform select_platform();
cl::Device select_device(cl::Platform platform);

int main(int argc, char **argv)
{
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " path_to_xclbin W H freq" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    const int W = atoi(argv[2]);
    const int H = atoi(argv[3]);
    const int DATA_SIZE = W*H;
    const float freq = atof(argv[4]);

    size_t file_buf_size;
    char *buf = read_bin_file(argv[1], file_buf_size);
    cl::Program::Binaries bins{{buf, file_buf_size}};

    cl_int err;
    cl::Platform platform = select_platform();
    cl::Device device = select_device(platform);

    std::chrono::time_point<std::chrono::system_clock> start,end;     
    start = std::chrono::system_clock::now();

    cl::Context context(device, NULL, NULL, NULL, &err);
    std::vector<cl::Device> devices{device};
    cl::Program program(context, devices, bins, NULL, &err);
    delete[] buf;
    
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
    cl::Kernel krnl_perlin(program,"perlin",&err);

    cl::Buffer out_buf(context, CL_MEM_READ_WRITE, sizeof(float) * DATA_SIZE, NULL, &err);

    krnl_perlin.setArg(0, W);
    krnl_perlin.setArg(1, H);
    krnl_perlin.setArg(2, out_buf);
    krnl_perlin.setArg(3, freq);

    float *out = (float *)q.enqueueMapBuffer(out_buf, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(float) * DATA_SIZE);

    krnl_perlin.setArg(0, W);
    krnl_perlin.setArg(1, H);
    krnl_perlin.setArg(2, out_buf);
    krnl_perlin.setArg(3, freq);

    q.enqueueTask(krnl_perlin);
    q.enqueueMigrateMemObjects({out_buf}, CL_MIGRATE_MEM_OBJECT_HOST);
    q.finish();

    end = std::chrono::system_clock::now();
    std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms \n";

    // Save to file as python lists
    FILE* outfile = fopen("perlin_hls.txt", "w");
    fprintf(outfile, "[\n");
    for(int y=0; y<H; ++y){
        fprintf(outfile, "[");
        for(int x=0; x<W; ++x){
            fprintf(outfile, "%.5f,", out[y*W+x]);
        }
        fprintf(outfile, "],\n");
    }
    fprintf(outfile, "]");
    fclose(outfile);

    return EXIT_SUCCESS;
}

cl::Platform select_platform()
{
    cl_int stat;
    
    std::vector<cl::Platform> platforms;
    stat = cl::Platform::get(&platforms);

    if(stat != CL_SUCCESS){
        std::cerr << "ERROR: Cannot get list of platforms" << std::endl;
        exit(EXIT_FAILURE);
    }

    size_t choice;
    std::cout << "----- List of platforms -----" << std::endl;
    for(size_t i=0; i<platforms.size(); ++i)
    {
        std::string name = platforms[i].getInfo<CL_PLATFORM_NAME>(&stat);
        std::cout << '[' << i <<"] " << name << std::endl;
    }
    std::cout << "> Select platform: ";
    std::cin >> choice;
    
    return platforms[choice];
}


cl::Device select_device(cl::Platform platform)
{
    cl_int stat;
    
    std::vector<cl::Device> devices;
    stat = platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);

    if(stat != CL_SUCCESS){
        std::cerr << "ERROR: Cannot get list of devices" << std::endl;
        exit(EXIT_FAILURE);
    }

    size_t choice;
    std::cout << "----- List of devices -----" << std::endl;
    for(size_t i=0; i<devices.size(); ++i)
    {
        std::string name = devices[i].getInfo<CL_DEVICE_NAME>(&stat);
        std::cout << '[' << i <<"] " << name << std::endl;
    }
    std::cout << "> Select device: ";
    std::cin >> choice;
    
    return devices[choice];
}

char* read_bin_file(std::string path, size_t& file_size)
{
    std::ifstream bin_file{path, std::ifstream::binary | std::ifstream::ate};

    if(!bin_file.good()){
        std::cerr << "ERROR: Cannot Open File at: "<<path<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    file_size = bin_file.tellg();
    char* buf = new char[file_size];
    
    bin_file.seekg(0);
    bin_file.read(buf, file_size);
    bin_file.close();
    
    return buf;
}
