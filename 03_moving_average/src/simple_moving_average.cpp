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

#include "hls_stream.h"

#define PERIOD 20

extern "C" {

    
    void simple_moving_average(hls::stream<float>& in, hls::stream<float>& out)
    {
        static float data[PERIOD + 1]; // Initialized to 0
        static float sum;
        #pragma HLS interface ap_ctrl_none port=return
        float new_data = in.read();

        // Should infer shift register
        for(int i=0; i<PERIOD; ++i) {
            data[i] = data[i+1];
        }
        data[PERIOD] = new_data;

        sum += (new_data-data[0]); // add new data and remove the oldest data
        out.write(sum/PERIOD);
    }
}