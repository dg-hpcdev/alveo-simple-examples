set -e
export PLATFORM_REPO_PATHS=/opt/xilinx/platforms
export XCL_EMULATION_MODE=hw_emu
export PLATFORM=xilinx_u250_gen3x16_xdma_4_1_202210_1

xclbin_name=moving_average.xclbin
host_bin_name=moving_average_hw_emu

g++ -Wall -g -std=c++11 ../src/host.cpp -o ${host_bin_name} -I${XILINX_XRT}/include/ -L${XILINX_XRT}/lib/ -lOpenCL -pthread -lrt -lstdc++
emconfigutil --platform ${PLATFORM} --nd 1
v++ -c -t ${XCL_EMULATION_MODE} --platform ${PLATFORM} -k exponential_moving_average -I../src ../src/exponential_moving_average.cpp -o exponential_moving_average.xo 
v++ -c -t ${XCL_EMULATION_MODE} --platform ${PLATFORM} -k simple_moving_average -I../src ../src/simple_moving_average.cpp -o simple_moving_average.xo 
v++ -c -t ${XCL_EMULATION_MODE} --platform ${PLATFORM} -k write_stream -I../src ../src/write_stream.cpp -o write_stream.xo 
v++ -c -t ${XCL_EMULATION_MODE} --platform ${PLATFORM} -k read_stream -I../src ../src/read_stream.cpp -o read_stream.xo 
v++ -l -t ${XCL_EMULATION_MODE} --platform ${PLATFORM} --config ../src/moving_average.cfg ./simple_moving_average.xo ./read_stream.xo ./write_stream.xo -o ${xclbin_name}

echo "Usage: XCL_EMULATION_MODE=${XCL_EMULATION_MODE} ./${host_bin_name} ${xclbin_name}"
echo "IMPORTANT: source XRT and Vitis setup files"