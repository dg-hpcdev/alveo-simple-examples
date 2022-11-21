set -e
export PLATFORM_REPO_PATHS=/opt/xilinx/platforms
export XCL_EMULATION_MODE=hw_emu
export PLATFORM=xilinx_u250_gen3x16_xdma_4_1_202210_1

xclbin_name=md5.xclbin
host_bin_name=md5_hw_emu

g++ -Wall -g -std=c++11 ../src/host.cpp -o ${host_bin_name} -I${XILINX_XRT}/include/ -L${XILINX_XRT}/lib/ -lOpenCL -pthread -lrt -lstdc++
emconfigutil --platform ${PLATFORM} --nd 1
v++ -c -t ${XCL_EMULATION_MODE} --platform ${PLATFORM} --config ../src/md5.cfg -k md5 -I../src ../src/md5.cpp -o md5.xo 
v++ -l -t ${XCL_EMULATION_MODE} --platform ${PLATFORM} --config ../src/md5.cfg ./md5.xo -o ${xclbin_name}

echo "Usage: XCL_EMULATION_MODE=${XCL_EMULATION_MODE} ./${host_bin_name} ${xclbin_name} path_to_file_to_hash"
echo "IMPORTANT: source XRT and Vitis setup files"