# 2. Perlin Noise HLS Example
This is simple example that generate 2D perlin noise in FPGA and write the values to a file as python style lists. The main aim of this example is to introduce the DATAFLOW pragma which enable task level parallelism. However, the main purpose of using DATAFLOW pragma is to show how writing HLS C++ code for hardware generally requires a diffrent style of programming.

```c++
void perlin(int nx, int ny, float* result, const float freq)
{
#pragma HLS INTERFACE m_axi port=result
#pragma HLS DATAFLOW
    
    hls::stream<Coord> coord_stream;
    hls::stream<float> noise_stream;

    const int N = nx*ny;

    gen_coord(coord_stream, nx, ny, freq);
    perlin_calc(coord_stream, noise_stream, N);
    write_mem(noise_stream, result, N);
}
```

In the code snippet above, it might seem like `gen_coord` is executed and then `perlin_calc` and `write_mem` respectively. In real hardware, those three functions are actually executed concurrently with data streams (`coord_stream` and `noise_stream`) connecting them. The data 'flows' between those 3 tasks, thus the name DATAFLOW.
- `gen_coord` generate x and y coordiate and write them to `coord_stream` one by one.
- `perlin_calc` reads coordinates from `coord_stream` and writes generated noise values to `noise_stream`.
- `write_mem` reads noise values from `noise_stream` and writes them to memory.

For more complex dataflows, the flow must be carefully designed to prevent any potential deadlocks.

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
g++ -Wall -g -std=c++11 ../src/host.cpp -o perlin -I${XILINX_XRT}/include/ -L${XILINX_XRT}/lib/ -lOpenCL -pthread -lrt -lstdc++
```
## Build And Link Kenel
```sh
# Replace ${TARGET} with / Set TARGET to:
#  sw_emu if targeting software emulation
#  hw_emu if targeting hardware emulation
#  hw if targeting hardware
v++ -c -t ${TARGET} --platform ${PLATFORM} --config ../src/perlin.cfg -k perlin -I../src ../src/perlin.cpp -o perlin.xo
v++ -l -t ${TARGET} --platform ${PLATFORM} --config ../src/perlin.cfg ./perlin.xo -o perlin.xclbin
```
## Configure Emulator
Only when targeting software/hardware emulation
```sh
emconfigutil --platform ${PLATFORM} --nd 1
```
## Run the host software
```sh
# Replace <W> with width, <H> with height, and <freq> with frequency
./perlin perlin.xclbin <W> <H> <freq>
```
After running, a text file `perlin_hls.txt` will be generated and contains lists of generated noise value.
