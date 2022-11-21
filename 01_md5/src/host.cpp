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

#include <chrono>
#include <vector>
#include <iostream>
#include <fstream>
#include <CL/cl2.hpp>

char* read_bin_file(std::string path, size_t& file_size);
char* read_and_pad_data(std::string path, size_t& padded_size);
cl::Platform select_platform();
cl::Device select_device(cl::Platform platform);

int main(int argc, char **argv)
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " path_to_xclbin path_to_file_to_hash" << std::endl;
        exit(EXIT_FAILURE);
    }
    
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
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
    cl::Kernel krnl_md5(program,"md5",&err);

    delete[] buf;

    size_t data_size;    
    char* data = read_and_pad_data(argv[2], data_size);
    unsigned char out[16];

    // In this demo, simple buffer allocation is used for simplicity.
    // However, aligned allocation/allocation with openCL is highly recommended for better performance.
    // See official documentation for more details. 
    cl::Buffer in1_buf(context, static_cast<cl_mem_flags>(CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), data_size, data, &err);
    cl::Buffer out_buf(context, static_cast<cl_mem_flags>(CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR), 16, out, &err);

    krnl_md5.setArg(0, in1_buf);
    krnl_md5.setArg(1, out_buf);
    krnl_md5.setArg(2, data_size);
    
    q.enqueueMigrateMemObjects({in1_buf}, 0 /* 0 means from host*/);
    q.enqueueTask(krnl_md5);
    q.enqueueMigrateMemObjects({out_buf}, CL_MIGRATE_MEM_OBJECT_HOST);
    q.finish();
    
    end = std::chrono::system_clock::now();
    
    printf("Result: ");
    for(unsigned char* p=out; p<out+16; ++p) printf("%02x",*p);
    printf("\n");

    std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms \n";

    delete[] data;

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

char* read_and_pad_data(std::string path, size_t& padded_size)
{
    std::ifstream bin_file{path, std::ifstream::binary | std::ifstream::ate};
    if(!bin_file.good()){
        std::cerr << "ERROR: Cannot Open File at: "<<path<<std::endl;
        exit(EXIT_FAILURE);
    }
    size_t len = bin_file.tellg();
    std::cout<<"LEN"<<len<<std::endl;

    // Do padding in software for simplicity
    size_t mod = (len+1)%64;
    size_t pad_size = (64+56-mod)%64;
    padded_size = len+1+pad_size+8;
    char* padded = new char[padded_size];
    
    bin_file.seekg(0);
    bin_file.read(padded, len);
    bin_file.close();

    padded[len] = (char)0x80;
    std::memset(padded+len+1, 0x0, pad_size);

    // Convert bytes to bits and append length
    len <<= 3;
    for(char* p = padded+(padded_size-8); p < padded+padded_size; ++p){
        *p = len & 0xff;
        len >>= 8;
    }

    return padded;
}