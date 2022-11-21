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

#define K 2.0/21

const int N = 2000000;
float result[N];

char* read_bin_file(std::string path, size_t& file_size);
char* read_and_pad_data(std::string path, size_t& padded_size);
cl::Platform select_platform();
cl::Device select_device(cl::Platform platform);

void software_run(float *data, float *result, const int N)
{
    int i;
    float s=0;
    for(i=0; i<N && i<20; ++i){
        s += data[i];
        result[i] = s/20.0f;
    }
    for(;i<N;++i){
        s += data[i] - data[i-20];
        result[i] = s/20.0f;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " path_to_xclbin" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    size_t file_buf_size;
    char *buf = read_bin_file(argv[1], file_buf_size);
    cl::Program::Binaries bins{{buf, file_buf_size}};

    cl_int err;
    cl::Platform platform = select_platform();
    cl::Device device = select_device(platform);
    
    cl::Context context(device, NULL, NULL, NULL, &err);
    std::vector<cl::Device> devices{device};
    cl::Program program(context, devices, bins, NULL, &err);
    // Allow out of order execution to allow both read_stream and write_stream to run at the same time
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
    
    cl::Kernel krnl_read_stream(program,"read_stream", &err);
    cl::Kernel krnl_write_stream(program,"write_stream", &err);

    delete[] buf;

    cl::Buffer in_buf(context, CL_MEM_READ_ONLY, sizeof(float) * N, NULL, &err);
    cl::Buffer out_buf(context, CL_MEM_READ_WRITE, sizeof(float) * N, NULL, &err);

    float *in = (float *)q.enqueueMapBuffer(in_buf, CL_TRUE, CL_MAP_READ, 0, sizeof(float) * N);
    float *out = (float *)q.enqueueMapBuffer(out_buf, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(float) * N);

    q.finish();

    for(int i=0; i<N; ++i) {in[i] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);}

    std::cout<<"Running software version...."<<std::endl;
    std::chrono::time_point<std::chrono::system_clock> start,end;
    
    start = std::chrono::system_clock::now();
    software_run(&in[0], &result[0], N);
    end = std::chrono::system_clock::now();

    std::cout << "Software run: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()/1000.0f << "ms \n";

    // No need to setArg for stream

    krnl_write_stream.setArg(1, in_buf);
    krnl_write_stream.setArg(2, N);
    
    krnl_read_stream.setArg(1, out_buf);
    krnl_read_stream.setArg(2, N);

   
    
    start = std::chrono::system_clock::now();
    
    q.enqueueMigrateMemObjects({in_buf}, 0);
    // In practice, you do not need to individually wait for each events to finish 
    // Using one q.finish() after enqueing all tasks is enough
    // This is for measuring performance individually
    q.finish();
    
    end = std::chrono::system_clock::now();
    std::cout << "Transfer Host to FPGA: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()/1000.0f << "ms \n";

    start = std::chrono::system_clock::now();

    q.enqueueTask(krnl_write_stream);
    q.enqueueTask(krnl_read_stream);
    q.finish();

    end = std::chrono::system_clock::now();
    std::cout << "Stream Processing: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms \n";

    start = std::chrono::system_clock::now();

    q.enqueueMigrateMemObjects({out_buf}, CL_MIGRATE_MEM_OBJECT_HOST);
    q.finish();

    end = std::chrono::system_clock::now();
    std::cout << "Transfer FPGA to Host: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()/1000.0f << "ms \n";

    float max_err = 0;
    for(int i=0; i<N; ++i) {
        float err = abs(out[i] - result[i]) / result[i];
        printf("%f\n",err);
        if (err > max_err) max_err = err;
    }
    std::cout<<"Max error = "<<max_err * 100<<"%\n";

    // delete[] data;

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