set -e
export PLATFORM_REPO_PATHS=/opt/xilinx/platforms
export XCL_EMULATION_MODE=hw_emu
export PLATFORM=xilinx_u250_gen3x16_xdma_4_1_202210_1

xclbin_name=perlin.xclbin
host_bin_name=perlin_hw_emu

g++ -Wall -g -std=c++11 ../src/host.cpp -o ${host_bin_name} -I${XILINX_XRT}/include/ -L${XILINX_XRT}/lib/ -lOpenCL -pthread -lrt -lstdc++
emconfigutil --platform ${PLATFORM} --nd 1
v++ -c -t ${XCL_EMULATION_MODE} --platform ${PLATFORM} --config ../src/perlin.cfg -k perlin -I../src ../src/perlin.cpp -o perlin.xo 
v++ -l -t ${XCL_EMULATION_MODE} --platform ${PLATFORM} --config ../src/perlin.cfg ./perlin.xo -o ${xclbin_name}

echo "Usage: XCL_EMULATION_MODE=${XCL_EMULATION_MODE} ./${host_bin_name} ${xclbin_name} W H freq"
echo "IMPORTANT: source XRT and Vitis setup files"