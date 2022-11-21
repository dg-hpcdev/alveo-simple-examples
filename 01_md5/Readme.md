# 1. MD5 HLS Example
This is a simple example demonstrating how to use FPGA hardware accerleration by writing C++ codes. This example is only intended to show how to write simple algorithms in C++ and them in FPGA without much consideration for performance. Thus, the md5 implementation in this example will be very straight forward and similar to how one might normally implement md5 algorithm in software. This example consists of two C++ source files: `md5.cpp` and `host.cpp`.
- `md5.cpp` is the kernel code for computing md5 hash. This is a very straightforward implementation without optimization for hardware.
- `host.cpp` is the code running on the host PC and is responsible for controlling and communicating with the FPGA.
- `md5.cfg` is a configuration file that describe, among other things, what kernels will be instantiated and connectivity among them.

## Setup
This must be done everytime a new terminal is opened
```sh
source /opt/xilinx/xrt/setup.sh
source <Vitis install path>/Vitis/<version>/settings64.sh
export PLATFORM_REPO_PATHS=/opt/xilinx/platforms
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
g++ -Wall -g -std=c++11 ../src/host.cpp -o md5 -I${XILINX_XRT}/include/ -L${XILINX_XRT}/lib/ -lOpenCL -pthread -lrt -lstdc++
```
## Build And Link Kenel
```sh
# Replace ${TARGET} with / Set TARGET to:
#  sw_emu if targeting software emulation
#  hw_emu if targeting hardware emulation
#  hw if targeting hardware
v++ -c -t ${TARGET} --platform ${PLATFORM} --config ../src/md5.cfg -k md5 -I../src ../src/md5.cpp -o md5.xo
v++ -l -t ${TARGET} --platform ${PLATFORM} --config ../src/md5.cfg ./md5.xo -o md5.xclbin
```
## Configure Emulator
Only when targeting software/hardware emulation
```sh
emconfigutil --platform ${PLATFORM} --nd 1
```
## Run the host software
```sh
# Replace <file_to_hash> with path to file to be hashed
./md5 md5.xclbin <file_to_hash>
```
