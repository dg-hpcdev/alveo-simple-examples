# 3. Moving Average HLS Example
This example demonstrate a free-running kernel. Free-running kernel is a kernel which always repeatedly execute on its own whearas normal kernel requires user command (such as `q.enqueueTask(krnl)`). One scenario one might use a free-running kernel is processing a data stream from other kernel (both RTL/HLS) without any CPU involvement. 
- The `write_stream` kernel read from a buffer of floats then write them to stream one by one. In real projects, this could be replaced by other data producing kernel such as streaming data from TCP
- The `simple_moving_average` kernel read the data stream from `write_stream` and calculate simple moving average with window size = 20 and write the result to another stream. For the sake of simplicity, the edge cases(the beginning and the end of data stream) are ignored: the sum are always divided by 20.
- The `read_stream` kernel read from `simple_moving_average` kernel's output stream and write them to FPGA's global memory waiting for user command to migrate them to host PC. If you want the kernel to directly write to host memory, see Host Memory.
- The `exponential_moving_average` kernel is similar to `simple_moving_average` kernel but calculates exponential moving average instead and could be interchanged. This is provided for demonstrating how to modify configuration file.

## Configuration File
These lines specify which kernels are instantiated and how many. The basic syntax is `nk=kernel_name:number_of_kernel:name_1,name_2` (name is optional)
```
nk=simple_moving_average:1
nk=write_stream:1
nk=read_stream:1
```

These lines maps kernel port to memory.

```
sp=write_stream_1.buf:DDR[0]
sp=read_stream_1.out:DDR[0]
```

These lines specify stream connections between kernels

```
stream_connect=write_stream_1.data_stream:simple_moving_average_1.in
stream_connect=simple_moving_average_1.out:read_stream_1.data_stream
```

## Swapping Kernel
Suppose you want to change from `simple_moving_average` to `exponential_moving_average`

First, Instantiate `exponential_moving_average` instead of `simple_moving_average`.
```
[connectivity]
nk=exponential_moving_average:1
nk=write_stream:1
nk=read_stream:1
```
Then, correctly change the stream connections.
```
stream_connect=write_stream_1.data_stream:exponential_moving_average_1.in
stream_connect=exponential_moving_average_1.out:read_stream_1.data_stream
```

## Setup
This must be done everytime a new terminal is opened
```sh
source /opt/xilinx/xrt/setup.sh
# Replace <Vitis install path> and <vesion>
source <Vitis install path>/Vitis/<version>/settings64.sh
export PLATFORM_REPO_PATHS=/opt/xilinx/platforms
# Change to appropiate platform
export PLATFORM=xilinx_u250_gen3x16_xdma_4_1_202210_1
```
For software emulation:
```sh
export XCL_EMULATION_MODE=sw_emu
cd sw_emu
```
For hardware emulation:
```sh
export XCL_EMULATION_MODE=hw_emu
cd hw_emu
```
For hardware:
```sh
cd hw
```
## Build Host Software
```sh
g++ -Wall -g -std=c++11 ../src/host.cpp -o moving_average -I${XILINX_XRT}/include/ -L${XILINX_XRT}/lib/ -lOpenCL -pthread -lrt -lstdc++
```
## Build And Link Kenel
```sh
# Replace ${TARGET} with / Set TARGET to:
#  sw_emu if targeting software emulation
#  hw_emu if targeting hardware emulation
#  hw if targeting hardware
v++ -c -t ${TARGET} --platform ${PLATFORM} -k exponential_moving_average -I../src ../src/exponential_moving_average.cpp -o exponential_moving_average.xo 
v++ -c -t ${TARGET} --platform ${PLATFORM} -k simple_moving_average -I../src ../src/simple_moving_average.cpp -o simple_moving_average.xo 
v++ -c -t ${TARGET} --platform ${PLATFORM} -k write_stream -I../src ../src/write_stream.cpp -o write_stream.xo 
v++ -c -t ${TARGET} --platform ${PLATFORM} -k read_stream -I../src ../src/read_stream.cpp -o read_stream.xo 
v++ -l -t ${TARGET} --platform ${PLATFORM} --config ../src/moving_average.cfg ./simple_moving_average.xo ./read_stream.xo ./write_stream.xo -o moving_average.xclbin

```
## Configure Emulator
Only when targeting software/hardware emulation
```sh
emconfigutil --platform ${PLATFORM} --nd 1
```
## Run the host software
```sh
./moving_average moving_average.xclbin
```
