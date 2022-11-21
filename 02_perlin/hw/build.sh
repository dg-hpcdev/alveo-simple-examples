export PLATFORM_REPO_PATHS=/opt/xilinx/platforms
g++ -Wall -g -std=c++11 ../src/host.cpp -o perlin_hw_emu -I${XILINX_XRT}/include/ -L${XILINX_XRT}/lib/ -lOpenCL -pthread -lrt -lstdc++
v++ -c -t hw --platform xilinx_u250_gen3x16_xdma_4_1_202210_1 --config ../src/u200.cfg -k perlin -I../src ../src/perlin.cpp -o perlin.xo 
v++ -l -t hw --platform xilinx_u250_gen3x16_xdma_4_1_202210_1 --config ../src/u200.cfg ./perlin.xo -o perlin.xclbin